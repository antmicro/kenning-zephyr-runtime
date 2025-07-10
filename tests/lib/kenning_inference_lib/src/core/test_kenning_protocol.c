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

#include "kenning_inference_lib/core/protocol.h"
#include "kenning_inference_lib/core/utils.h"
#include "utils.h"

#define MOCK_BUFFER_SIZE 8192
static uint8_t mock_write_buffer[MOCK_BUFFER_SIZE];
static uint8_t mock_read_buffer[MOCK_BUFFER_SIZE];
static uint8_t mock_loader_buffer[MOCK_BUFFER_SIZE];
static int mock_write_buffer_idx;
static int mock_read_buffer_idx;
static int mock_loader_buffer_idx;

struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_LOADER_TYPES];
resp_message_t *resp_message;

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                        \
    MOCK(const char *, get_status_str, status_t);          \
    MOCK(status_t, protocol_read_data, uint8_t *, size_t); \
    MOCK(status_t, protocol_write_data, const uint8_t *, size_t);

MOCKS(DECLARE_MOCK);

const char *get_status_str_mock(status_t);
status_t protocol_read_data_mock(uint8_t *data, size_t data_length);
status_t protocol_write_data_mock(const uint8_t *data, size_t data_length);

// ========================================================
// helper functions declarations
// ========================================================

/**
 * Prepares message of given type and payload size and adds it to read buffer
 *
 * @param message_type type of the message
 * @param payload_size size of the payload
 */
void prepare_message_in_buffer(message_type_t message_type, flags_t flags, size_t payload_size);

/**
 * Prepares message of given type and payload and stores it in resp_message
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 */
void prepare_send_message(message_type_t msg_type, flags_t flags, uint8_t *payload, size_t payload_size);

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

status_t protocol_read_data_mock(uint8_t *data, size_t data_length)
{
    if (mock_read_buffer_idx + data_length > sizeof(mock_read_buffer) / sizeof(int))
    {
        return KENNING_PROTOCOL_STATUS_MSG_TOO_BIG;
    }

    if (data != NULL)
    {
        memcpy(data, mock_read_buffer + mock_read_buffer_idx, data_length);
    }
    mock_read_buffer_idx += data_length;
    return STATUS_OK;
}

status_t protocol_write_data_mock(const uint8_t *data, size_t data_length)
{
    RETURN_ERROR_IF_POINTER_INVALID(data, PROTOCOL_STATUS_INV_PTR);

    if (mock_read_buffer_idx + data_length > sizeof(mock_read_buffer) / sizeof(int))
    {
        return KENNING_PROTOCOL_STATUS_MSG_TOO_BIG;
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

int loader_save_mock(struct msg_loader *ldr, const uint8_t *src, size_t n)
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
    flags_t test_flags;
    test_flags.raw_bytes = 0b0001000000011001;
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, 0x123);

    status = protocol_recv_msg_hdr(&hdr);

    zassert_equal(status, STATUS_OK);
    zassert_equal(hdr.payload_size, 0x123);
    zassert_equal(hdr.message_type, MESSAGE_TYPE_IOSPEC);
    zassert_equal(hdr.flags.flags_iospec.serialized, 1);
    zassert_equal(hdr.flags.general_purpose_flags.success, 1);
    zassert_equal(hdr.flags.general_purpose_flags.fail, 0);
    zassert_equal(hdr.flags.general_purpose_flags.is_host_message, 0);
    zassert_equal(hdr.flags.general_purpose_flags.has_payload, 1);
    zassert_equal(hdr.flags.general_purpose_flags.first, 1);
    zassert_equal(hdr.flags.general_purpose_flags.last, 0);
    zassert_equal(mock_read_buffer_idx, sizeof(message_hdr_t));
    zassert_equal(mock_write_buffer_idx, 0);
}

/**
 * Tests if protocol receive message reads message content properly
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_recv_msg_content)
{
    status_t status = STATUS_OK;

    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    test_flags.raw_bytes = 0b0000000000000000;
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, 0x123);
    protocol_read_data_mock(NULL, sizeof(message_hdr_t));

    struct msg_loader *ldr = prepare_loader();

    status = protocol_recv_msg_content(ldr, 0x123);
    int expected_size = mock_read_buffer_idx;

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
    message_hdr_t hdr = {0};

    protocol_read_data_fake.return_val = PROTOCOL_STATUS_TIMEOUT;

    status = protocol_recv_msg(&hdr);

    zassert_equal(KENNING_PROTOCOL_STATUS_TIMEOUT, status);
}

// ========================================================
// send_message
// ========================================================

/**
 * Tests if protocol properly divides large payload into messages.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_transmit)
{
    status_t status;

    // Mock 'protocol_write_data' function will write all calls to a buffer ('mock_write_buffer').
    // Each message sent calls that function twice (for header and for payload).
    // We need to extract the headers, so this variable is used to store position of the mext message's header.
    unsigned int mock_write_buffer_hdr_idx;

#define TEST_PROTOCOL_TRANSMIT(_transmission, _message_count, _payload_sizes, _payload_offsets)        \
    kenning_protocol_tests_setup_f();                                                                  \
    protocol_write_data_fake.custom_fake = protocol_write_data_mock;                                   \
    status = protocol_transmit(&_transmission);                                                        \
    zassert_equal(status, STATUS_OK);                                                                  \
    zassert_equal(protocol_write_data_fake.call_count, 2 * _message_count);                            \
    mock_write_buffer_hdr_idx = 0;                                                                     \
    for (unsigned int i = 0; i < _message_count; i++)                                                  \
    {                                                                                                  \
        const message_hdr_t *hdr = (message_hdr_t *)(mock_write_buffer + mock_write_buffer_hdr_idx);   \
        const uint8_t *payload = protocol_write_data_fake.arg0_history[2 * i + 1];                     \
        zassert_equal(hdr->flags.general_purpose_flags.success, 1);                                    \
        zassert_equal(hdr->flags.general_purpose_flags.fail, 0);                                       \
        zassert_equal(hdr->flags.flags_iospec.serialized, 1);                                          \
        if (i == 0)                                                                                    \
        {                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.first, 1);                                  \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.first, 0);                                  \
        }                                                                                              \
        if (i == (_message_count - 1))                                                                 \
        {                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.last, 1);                                   \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.last, 0);                                   \
        }                                                                                              \
        zassert_equal(hdr->flags.general_purpose_flags.has_payload, 1);                                \
        zassert_equal(hdr->flags.general_purpose_flags.is_host_message, 0);                            \
        zassert_equal(hdr->payload_size, _payload_sizes[i]);                                           \
        zassert_equal(hdr->flow_control_flags, FLOW_CONTROL_TRANSMISSION);                             \
        zassert_equal(hdr->message_type, _transmission.hdr.message_type);                              \
        zassert_equal((unsigned int)payload, (unsigned int)test_payload_buffer + _payload_offsets[i]); \
        mock_write_buffer_hdr_idx += sizeof(message_hdr_t) + _payload_sizes[i];                        \
    }

    uint8_t test_payload_buffer[100];

    flags_t test_flags;
    test_flags.general_purpose_flags.success = 1;
    test_flags.general_purpose_flags.fail = 0;
    test_flags.flags_iospec.serialized = 1;

    resp_message_t transmission;
    transmission.hdr.message_type = MESSAGE_TYPE_IOSPEC;
    transmission.hdr.flags = test_flags;
    transmission.payload = test_payload_buffer;

    // In prj.conf file for the tests we override the meximum message size (setting it to 8).
    // Therefore 'protocol_transmit' function will splice the payload into 8-byte slices.
    unsigned int payload_sizes_2_messages[] = {8, 6};
    unsigned int payload_offsets_2_messages[] = {0, 8};
    transmission.hdr.payload_size = 14;
    TEST_PROTOCOL_TRANSMIT(transmission, 2, payload_sizes_2_messages, payload_offsets_2_messages);
    unsigned int payload_sizes_1_message[] = {4};
    unsigned int payload_offsets_1_message[] = {0};
    transmission.hdr.payload_size = 4;
    TEST_PROTOCOL_TRANSMIT(transmission, 1, payload_sizes_1_message, payload_offsets_1_message);
    unsigned int payload_sizes_7_messages[] = {8, 8, 8, 8, 8, 8, 3};
    unsigned int payload_offsets_7_messages[] = {0, 8, 16, 24, 32, 40, 48};
    transmission.hdr.payload_size = 51;
    TEST_PROTOCOL_TRANSMIT(transmission, 7, payload_sizes_7_messages, payload_offsets_7_messages);
    unsigned int payload_sizes_2_messages_full[] = {8, 8};
    unsigned int payload_offsets_2_messages_full[] = {0, 8};
    transmission.hdr.payload_size = 16;
    TEST_PROTOCOL_TRANSMIT(transmission, 2, payload_sizes_2_messages_full, payload_offsets_2_messages_full);
    unsigned int payload_sizes_1_message_full[] = {8};
    unsigned int payload_offsets_1_message_full[] = {0};
    transmission.hdr.payload_size = 8;
    TEST_PROTOCOL_TRANSMIT(transmission, 1, payload_sizes_1_message_full, payload_offsets_1_message_full);
    unsigned int payload_sizes_7_messages_full[] = {8, 8, 8, 8, 8, 8, 8};
    unsigned int payload_offsets_7_messages_full[] = {0, 8, 16, 24, 32, 40, 48};
    transmission.hdr.payload_size = 56;
    TEST_PROTOCOL_TRANSMIT(transmission, 7, payload_sizes_7_messages_full, payload_offsets_7_messages_full);

#undef TEST_PROTOCOL_TRANSMIT
}

/**
 * Tests if protocol send message writes properly message without payload
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_send_message_without_payload)
{
    status_t status = STATUS_OK;

    protocol_write_data_fake.custom_fake = protocol_write_data_mock;
    flags_t test_flags;
    test_flags.raw_bytes = 0b1110000000111111;

#define TEST_PROTOCOL_SEND_MSG(_message_type)                                        \
    prepare_send_message(_message_type, test_flags, NULL, 0);                        \
                                                                                     \
    status = protocol_send_msg(resp_message);                                        \
                                                                                     \
    zassert_equal(STATUS_OK, status);                                                \
    zassert_equal(resp_message->hdr.payload_size, 0);                                \
    zassert_equal(resp_message->hdr.message_type, _message_type);                    \
    zassert_equal(resp_message->hdr.flags.flags_iospec.serialized, 0);               \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.success, 1);         \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.fail, 1);            \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.is_host_message, 1); \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.has_payload, 1);     \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.first, 1);           \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.last, 1);

    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_OK);
    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_ERROR);
    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_PROCESS);
    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_OUTPUT);

#undef TEST_PROTOCOL_SEND_MSG
}

/**
 * Tests if protocol send message writes properly message with payload
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_send_message_with_payload)
{
    status_t status = STATUS_OK;
    uint8_t msg_payload[128] = {0};

    protocol_write_data_fake.custom_fake = protocol_write_data_mock;
    flags_t test_flags;
    test_flags.raw_bytes = 0b0001111111000000;

#define TEST_PROTOCOL_SEND_MSG(_message_type)                                          \
    prepare_send_message(_message_type, test_flags, msg_payload, sizeof(msg_payload)); \
                                                                                       \
    status = protocol_send_msg(resp_message);                                          \
                                                                                       \
    zassert_equal(STATUS_OK, status);                                                  \
    zassert_equal(resp_message->hdr.payload_size, sizeof(msg_payload));                \
    zassert_equal(resp_message->hdr.message_type, _message_type);                      \
    zassert_equal(resp_message->hdr.flags.flags_iospec.serialized, 1);                 \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.success, 0);           \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.fail, 0);              \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.is_host_message, 0);   \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.has_payload, 0);       \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.first, 0);             \
    zassert_equal(resp_message->hdr.flags.general_purpose_flags.last, 0);

    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_OK);
    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_ERROR);
    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_PROCESS);
    TEST_PROTOCOL_SEND_MSG(MESSAGE_TYPE_OUTPUT);

#undef TEST_PROTOCOL_SEND_MSG
}

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
    message_hdr_t hdr = {0};
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
    message_hdr_t hdr = {0};
    resp_message_t response;
    response.hdr = hdr;

    status = protocol_prepare_success_resp(&response);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MESSAGE_TYPE_OK, response.hdr.message_type);
    zassert_equal(0, response.hdr.payload_size);
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
    message_hdr_t hdr = {0};
    resp_message_t response;
    response.hdr = hdr;

    status = protocol_prepare_fail_resp(&response);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MESSAGE_TYPE_ERROR, response.hdr.message_type);
    zassert_equal(0, response.hdr.payload_size);
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
void prepare_message_in_buffer(message_type_t message_type, flags_t flags, size_t payload_size)
{
    message_hdr_t hdr_to_read = {
        .message_type = message_type,
        .flags = flags,
        .payload_size = payload_size,
    };
    memcpy(mock_read_buffer, &hdr_to_read, sizeof(message_hdr_t));
    memset(mock_read_buffer + sizeof(message_hdr_t), 'x', payload_size);
}

void prepare_send_message(message_type_t msg_type, flags_t flags, uint8_t *payload, size_t payload_size)
{
    if (IS_VALID_POINTER(resp_message))
    {
        free(resp_message);
        resp_message = NULL;
    }
    message_hdr_t hdr = {
        .message_type = msg_type,
        .payload_size = payload_size,
        .flags = flags,
    };
    resp_message = malloc(sizeof(resp_message_t));
    resp_message->payload = malloc(payload_size);
    resp_message->hdr = hdr;
    if (payload_size > 0)
    {
        memcpy(resp_message->payload, payload, payload_size);
    }
}
