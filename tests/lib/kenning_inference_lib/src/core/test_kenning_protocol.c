/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include <kenning_inference_lib/core/kenning_protocol.h>
#include <kenning_inference_lib/core/loaders.h>

#include "utils.h"

#define MOCK_BUFFER_SIZE 8192
static uint8_t mock_write_buffer[MOCK_BUFFER_SIZE];
static uint8_t mock_read_buffer[MOCK_BUFFER_SIZE];
static uint8_t mock_loader_buffer[MOCK_BUFFER_SIZE];
static int mock_write_buffer_idx;
static int mock_read_buffer_idx;
static int mock_loader_buffer_idx;

struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_LOADER_TYPES];

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                        \
    MOCK(const char *, get_status_str, status_t)           \
    MOCK(status_t, protocol_read_data, uint8_t *, size_t); \
    MOCK(status_t, protocol_write_data, const uint8_t *, size_t);

MOCKS(DECLARE_MOCK);

const char *get_status_str_mock(status_t);
status_t protocol_read_data_mock(uint8_t *data, size_t data_length);
status_t protocol_write_data_mock(const uint8_t *data, size_t data_length);

// ========================================================
// helper functions declarations
// ========================================================

void prepare_message_in_buffer(message_type_t message_type, size_t payload_size);

// ========================================================
// setup
// ========================================================

static void kenning_protocol_tests_setup_f()
{
    MOCKS(RESET_MOCK);
    memset(mock_write_buffer, 0, MOCK_BUFFER_SIZE);
    memset(mock_read_buffer, 0, MOCK_BUFFER_SIZE);
    memset(mock_loader_buffer, 0, MOCK_BUFFER_SIZE);
    mock_write_buffer_idx = 0;
    mock_read_buffer_idx = 0;
    mock_loader_buffer_idx = 0;
}

static void kenning_protocol_tests_teardown_f() {}

ZTEST_SUITE(kenning_inference_lib_test_kenning_protocol, NULL, NULL, kenning_protocol_tests_setup_f, NULL,
            kenning_protocol_tests_teardown_f);

// ========================================================
// mocks definitions
// ========================================================

// TODO: out of bounds checks

status_t protocol_read_data_mock(uint8_t *data, size_t data_length)
{
    if (data != NULL)
    {
        memcpy(data, mock_read_buffer + mock_read_buffer_idx, data_length);
    }
    mock_read_buffer_idx += data_length;
    return STATUS_OK;
}

status_t protocol_write_data_mock(const uint8_t *data, size_t data_length)
{
    if (data == NULL)
    {
        return PROTOCOL_STATUS_INV_PTR;
    }
    memcpy(mock_write_buffer + mock_write_buffer_idx, data, data_length);
    mock_write_buffer_idx += data_length;
    return STATUS_OK;
}

int loader_reset_mock(struct msg_loader *ldr, size_t n)
{
    mock_loader_buffer_idx = 0;
    return 0;
}

int loader_save_mock(struct msg_loader *ldr, uint8_t *src, size_t n)
{
    memcpy(mock_loader_buffer + mock_loader_buffer_idx, src, n);
    mock_loader_buffer_idx += n;
    return 0;
}

struct msg_loader *prepare_loader()
{
    static struct msg_loader ldr = {
        .save = loader_save_mock,
        .reset = loader_reset_mock,
    };
    return &ldr;
}

// ========================================================
// receive_message
// ========================================================

/**
 * Tests if protocol receive message reads message header properly
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_recv_msg_hdr)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = {0};

    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, 0x123);

    status = protocol_recv_msg_hdr(&hdr);

    zassert_equal(status, STATUS_OK);
    zassert_equal(hdr.message_size, MESSAGE_SIZE_FULL(0x123));
    zassert_equal(hdr.message_type, MESSAGE_TYPE_IOSPEC);
    zassert_equal(mock_read_buffer_idx, sizeof(message_hdr_t));
    zassert_equal(mock_write_buffer_idx, 0);
}

/**
 * Tests if protocol receive message reads message content properly
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_recv_msg_content)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = {0};

    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, 0x123);
    protocol_read_data_mock(NULL, sizeof(message_hdr_t));

    struct msg_loader *ldr = prepare_loader();

    status = protocol_recv_msg_content(ldr, 0x123);
    int expected_size = mock_read_buffer_idx - sizeof(message_hdr_t) + sizeof(MESSAGE_TYPE_IOSPEC);

    zassert_equal(status, STATUS_OK);
    zassert_equal(expected_size, MESSAGE_SIZE_FULL(0x123));
    zassert_equal(mock_write_buffer_idx, 0);
}

/**
 * Tests if protocol receive message header fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_recv_msg_hdr_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = protocol_recv_msg_hdr(NULL);

    zassert_equal(KENNING_PROTOCOL_STATUS_INV_PTR, status);
}

/**
 * Tests if protocol receive message content fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_recv_msg_content_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = protocol_recv_msg_content(NULL, 0);

    zassert_equal(KENNING_PROTOCOL_STATUS_INV_PTR, status);
}

/**
 * Tests if protocol receive message fails if protocol read fails
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_read_error)
{
    status_t status = STATUS_OK;
    uint8_t message_payload[] = "some data";
    message_hdr_t hdr = {0};

    protocol_read_data_fake.return_val = PROTOCOL_STATUS_RECV_ERROR_BUSY;

    status = protocol_recv_msg(&hdr);

    zassert_equal(KENNING_PROTOCOL_STATUS_CLIENT_DISCONNECTED, status);
}

/**
 * Tests if protocol receive message hits timeout if protocol read does so
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_timeout)
{
    status_t status = STATUS_OK;
    uint8_t message_payload[] = "some data";
    message_hdr_t hdr = {0};

    protocol_read_data_fake.return_val = PROTOCOL_STATUS_TIMEOUT;

    status = protocol_recv_msg(&hdr);

    zassert_equal(KENNING_PROTOCOL_STATUS_TIMEOUT, status);
}

// ========================================================
// send_message
// ========================================================

/**
 * Tests if protocol send message fails if message pointer is invalid
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_send_message_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = protocol_send_msg(NULL);

    zassert_equal(KENNING_PROTOCOL_STATUS_INV_PTR, status);
}

/**
 * Tests if protocol send message fails if UART write fails
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_send_message_protocol_fail)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t msg = {
        .hdr = hdr,
        .payload = 0,
    };

    protocol_write_data_fake.return_val = PROTOCOL_STATUS_RECV_ERROR;

    status = protocol_send_msg(&msg);

    zassert_equal(KENNING_PROTOCOL_STATUS_CLIENT_DISCONNECTED, status);
}

// ========================================================
// prepare_success_response
// ========================================================

/**
 * Tests if protocol prepare success message properly creates empty OK message
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_prepare_success_response)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t response;
    response.hdr = hdr;

    status = protocol_prepare_success_resp(&response);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MESSAGE_TYPE_OK, response.hdr.message_type);
    zassert_equal(sizeof(message_type_t), response.hdr.message_size);
}

/**
 * Tests if protocol prepare success message fails if message pointer is invalid
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_prepare_success_response_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = protocol_prepare_success_resp(NULL);

    zassert_equal(KENNING_PROTOCOL_STATUS_INV_PTR, status);
}

// ========================================================
// prepare_failure_response
// ========================================================

/**
 * Tests if protocol prepare failure message properly creates empty ERROR message
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_prepare_fail_response)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t response;
    response.hdr = hdr;

    status = protocol_prepare_fail_resp(&response);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MESSAGE_TYPE_ERROR, response.hdr.message_type);
    zassert_equal(sizeof(message_type_t), response.hdr.message_size);
}

/**
 * Tests if protocol prepare failure message fails if message pointer is invalid
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_prepare_fail_response_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = protocol_prepare_fail_resp(NULL);

    zassert_equal(KENNING_PROTOCOL_STATUS_INV_PTR, status);
}

// ========================================================
// helper functions
// ========================================================
void prepare_message_in_buffer(message_type_t message_type, size_t payload_size)
{
    message_hdr_t hdr_to_read = {
        .message_type = message_type,
        .message_size = MESSAGE_SIZE_FULL(payload_size),
    };
    memcpy(mock_read_buffer, &hdr_to_read, sizeof(message_hdr_t));
    memset(mock_read_buffer + sizeof(message_hdr_t), 'x', payload_size);
}
