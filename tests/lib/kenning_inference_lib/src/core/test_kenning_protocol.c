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
