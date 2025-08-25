/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
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
#define MOCK_LOADER_RESET_ERROR 37
static uint8_t mock_write_buffer[MOCK_BUFFER_SIZE];
static uint8_t mock_read_buffer[MOCK_BUFFER_SIZE];
static uint8_t mock_loader_buffer[MOCK_BUFFER_SIZE];
static int mock_write_buffer_idx;
static int mock_read_buffer_idx;
static int mock_loader_buffer_idx;

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
void prepare_message_in_buffer(message_type_t message_type, flags_t flags, flow_control_flags_t flow_control_flags,
                               size_t payload_size);

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

int loader_reset_mock(struct msg_loader *ldr)
{
    mock_loader_buffer_idx = 0;
    ldr->written = 0;
    return 0;
}

int loader_save_mock(struct msg_loader *ldr, const uint8_t *src, size_t n)
{
    memcpy(mock_loader_buffer + mock_loader_buffer_idx, src, n);
    mock_loader_buffer_idx += n;
    ldr->written += n;
    return 0;
}

int loader_reset_failure_mock(struct msg_loader *ldr) { return MOCK_LOADER_RESET_ERROR; }

int loader_save_failure_mock(struct msg_loader *ldr, const uint8_t *src, size_t n) { return 1; }

struct msg_loader *get_loader(message_type_t message_type)
{
    static struct msg_loader ldr = {
        .save = loader_save_mock,
        .reset = loader_reset_mock,
    };
    return &ldr;
}

struct msg_loader *get_loader_reset_fail(message_type_t message_type)
{
    static struct msg_loader ldr = {
        .save = loader_save_mock,
        .reset = loader_reset_failure_mock,
    };
    return &ldr;
}

struct msg_loader *get_loader_save_fail(message_type_t message_type)
{
    static struct msg_loader ldr = {
        .save = loader_save_failure_mock,
        .reset = loader_reset_mock,
    };
    return &ldr;
}

// ========================================================
// listen
// ========================================================

/**
 * Tests if a transmission or request is received properly.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_multiple_messages)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    int expected_size;
#define TEST_PROTOCOL_LISTEN(flow_control_flags, should_be_request)                      \
    mock_read_buffer_idx = 0;                                                            \
    test_flags.raw_bytes = 0b0001000000011100;                                           \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 100); \
    test_flags.raw_bytes = 0b0001000000001100;                                           \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 100); \
    test_flags.raw_bytes = 0b0001000000001100;                                           \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 200); \
    test_flags.raw_bytes = 0b0001000000101100;                                           \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 700); \
    expected_size = mock_read_buffer_idx;                                                \
    mock_read_buffer_idx = 0;                                                            \
    status = protocol_listen(&event, get_loader);                                        \
    zassert_equal(status, STATUS_OK);                                                    \
    zassert_equal(event.payload.size, 100 + 100 + 200 + 700);                            \
    zassert_equal(event.is_request, should_be_request);                                  \
    zassert_equal(event.message_type, MESSAGE_TYPE_IOSPEC);                              \
    zassert_equal(mock_read_buffer_idx, expected_size);

    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_REQUEST, 1);
    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_TRANSMISSION, 0);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if a transmission or request with a single message is received properly.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_single_message)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    int expected_size;

#define TEST_PROTOCOL_LISTEN(flow_control_flags, should_be_request)                      \
    mock_read_buffer_idx = 0;                                                            \
    test_flags.raw_bytes = 0b0001000000111100;                                           \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 100); \
    expected_size = mock_read_buffer_idx;                                                \
    mock_read_buffer_idx = 0;                                                            \
    status = protocol_listen(&event, get_loader);                                        \
    zassert_equal(status, STATUS_OK);                                                    \
    zassert_equal(event.payload.size, 100);                                              \
    zassert_equal(event.is_request, should_be_request);                                  \
    zassert_equal(event.message_type, MESSAGE_TYPE_IOSPEC);                              \
    zassert_equal(mock_read_buffer_idx, expected_size);

    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_REQUEST, 1);
    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_TRANSMISSION, 0);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if the correct error is returned on a loader failure.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_loader_failure)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    test_flags.raw_bytes = 0b0001000000111100;
    protocol_event_t event;
    int expected_size;
#define TEST_PROTOCOL_LISTEN(loader_picker, status_)                                       \
    mock_read_buffer_idx = 0;                                                              \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, FLOW_CONTROL_REQUEST, 100); \
    expected_size = mock_read_buffer_idx;                                                  \
    mock_read_buffer_idx = 0;                                                              \
    status = protocol_listen(&event, loader_picker);                                       \
    zassert_equal(mock_read_buffer_idx, expected_size);                                    \
    zassert_equal(status, status_);

    TEST_PROTOCOL_LISTEN(get_loader_reset_fail, MOCK_LOADER_RESET_ERROR);
    TEST_PROTOCOL_LISTEN(get_loader_save_fail, KENNING_PROTOCOL_STATUS_MSG_TOO_BIG);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if a transmission or request with a single empty message is received properly.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_empty_message)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    int expected_size;

#define TEST_PROTOCOL_LISTEN(flow_control_flags, should_be_request)                    \
    mock_read_buffer_idx = 0;                                                          \
    test_flags.raw_bytes = 0b0001000000110100;                                         \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 0); \
    expected_size = mock_read_buffer_idx;                                              \
    mock_read_buffer_idx = 0;                                                          \
    status = protocol_listen(&event, get_loader);                                      \
    zassert_equal(status, STATUS_OK);                                                  \
    zassert_equal(event.payload.size, 0);                                              \
    zassert_equal(event.is_request, should_be_request);                                \
    zassert_equal(event.message_type, MESSAGE_TYPE_IOSPEC);                            \
    zassert_equal(mock_read_buffer_idx, expected_size);

    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_REQUEST, 1);
    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_TRANSMISSION, 0);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if proper error is returned if an the first message has not 'first' flag set.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_no_first_flag)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    test_flags.raw_bytes = 0b0001000000001100;
    prepare_message_in_buffer(MESSAGE_TYPE_MODEL, test_flags, FLOW_CONTROL_REQUEST, 100);
    int expected_size = mock_read_buffer_idx;
    mock_read_buffer_idx = 0;
    status = protocol_listen(&event, get_loader);
    zassert_equal(status, KENNING_PROTOCOL_STATUS_FLOW_CONTROL_ERROR);
    // We still want to read the whole message in case of an error
    zassert_equal(mock_read_buffer_idx, expected_size);

#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if proper error is returned if an unexpected message (like acknowledge) is received.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_invalid_flow_control)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    int expected_size;
#define TEST_PROTOCOL_LISTEN(flow_control_flags)                                           \
    mock_read_buffer_idx = 0;                                                              \
    test_flags.raw_bytes = 0b0001000000011100;                                             \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, FLOW_CONTROL_REQUEST, 100); \
    test_flags.raw_bytes = 0b0001000000001100;                                             \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, FLOW_CONTROL_REQUEST, 100); \
    test_flags.raw_bytes = 0b0001000000001100;                                             \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 200);   \
    expected_size = mock_read_buffer_idx;                                                  \
    test_flags.raw_bytes = 0b0001000000101100;                                             \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, FLOW_CONTROL_REQUEST, 700); \
    mock_read_buffer_idx = 0;                                                              \
    status = protocol_listen(&event, get_loader);                                          \
    zassert_equal(status, KENNING_PROTOCOL_STATUS_FLOW_CONTROL_ERROR);                     \
    zassert_equal(mock_read_buffer_idx, expected_size);

    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_ACKNOWLEDGE);
    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_REQUEST_RETRANSMIT);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if proper error is returned if an unexpected message (like acknowledge) is received as the first message.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_invalid_flow_control_first_message)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    int expected_size;
#define TEST_PROTOCOL_LISTEN(flow_control_flags)                                         \
    mock_read_buffer_idx = 0;                                                            \
    test_flags.raw_bytes = 0b0001000000011100;                                           \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, flow_control_flags, 100); \
    expected_size = mock_read_buffer_idx;                                                \
    mock_read_buffer_idx = 0;                                                            \
    status = protocol_listen(&event, get_loader);                                        \
    zassert_equal(status, KENNING_PROTOCOL_STATUS_FLOW_CONTROL_ERROR);                   \
    zassert_equal(mock_read_buffer_idx, expected_size);

    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_ACKNOWLEDGE);
    TEST_PROTOCOL_LISTEN(FLOW_CONTROL_REQUEST_RETRANSMIT);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if proper error is returned if messages within an event have different types.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_invalid_message_type)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    int expected_size;
#define TEST_PROTOCOL_LISTEN(message_type)                                                 \
    mock_read_buffer_idx = 0;                                                              \
    test_flags.raw_bytes = 0b0001000000011100;                                             \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, FLOW_CONTROL_REQUEST, 100); \
    test_flags.raw_bytes = 0b0001000000001100;                                             \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, FLOW_CONTROL_REQUEST, 100); \
    test_flags.raw_bytes = 0b0001000000001100;                                             \
    prepare_message_in_buffer(message_type, test_flags, FLOW_CONTROL_REQUEST, 200);        \
    expected_size = mock_read_buffer_idx;                                                  \
    test_flags.raw_bytes = 0b0001000000101100;                                             \
    prepare_message_in_buffer(MESSAGE_TYPE_IOSPEC, test_flags, FLOW_CONTROL_REQUEST, 700); \
    mock_read_buffer_idx = 0;                                                              \
    status = protocol_listen(&event, get_loader);                                          \
    zassert_equal(status, KENNING_PROTOCOL_STATUS_INVALID_MESSAGE_TYPE);                   \
    zassert_equal(mock_read_buffer_idx, expected_size);

    TEST_PROTOCOL_LISTEN(MESSAGE_TYPE_DATA);
    TEST_PROTOCOL_LISTEN(MESSAGE_TYPE_MODEL);
    TEST_PROTOCOL_LISTEN(MESSAGE_TYPE_RUNTIME);
    TEST_PROTOCOL_LISTEN(MESSAGE_TYPE_OUTPUT);
    TEST_PROTOCOL_LISTEN(MESSAGE_TYPE_PROCESS);
    TEST_PROTOCOL_LISTEN(34);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if proper error is returned if a non-existant message type is received.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_invalid_message_type_first_message)
{
    status_t status = STATUS_OK;
    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    flags_t test_flags;
    protocol_event_t event;
    int expected_size;
    test_flags.raw_bytes = 0b0001000000011100;
#define TEST_PROTOCOL_LISTEN(message_type)                                          \
    mock_read_buffer_idx = 0;                                                       \
    prepare_message_in_buffer(message_type, test_flags, FLOW_CONTROL_REQUEST, 100); \
    expected_size = mock_read_buffer_idx;                                           \
    mock_read_buffer_idx = 0;                                                       \
    status = protocol_listen(&event, get_loader);                                   \
    zassert_equal(status, KENNING_PROTOCOL_STATUS_INVALID_MESSAGE_TYPE);            \
    zassert_equal(mock_read_buffer_idx, expected_size);

    TEST_PROTOCOL_LISTEN(34);
    TEST_PROTOCOL_LISTEN(17);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if protocol_listen returns proper error message when given an invalid pointer to the callback function.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_lower_layer_protocol_failure)
{
    status_t status;
    protocol_event_t event;
#define TEST_PROTOCOL_LISTEN(lower_level_error, expected_error) \
    protocol_read_data_fake.return_val = lower_level_error;     \
    status = protocol_listen(&event, get_loader);               \
    zassert_equal(status, expected_error);

    TEST_PROTOCOL_LISTEN(PROTOCOL_STATUS_RECV_ERROR, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_LISTEN(PROTOCOL_STATUS_RECV_ERROR_NOSYS, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_LISTEN(PROTOCOL_STATUS_RECV_ERROR_BUSY, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_LISTEN(PROTOCOL_STATUS_NO_DATA, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_LISTEN(PROTOCOL_STATUS_INV_ARG_STOP_BITS, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_LISTEN(PROTOCOL_STATUS_INV_ARG_WORDSIZE, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_LISTEN(PROTOCOL_STATUS_TIMEOUT, KENNING_PROTOCOL_STATUS_TIMEOUT);
#undef TEST_PROTOCOL_LISTEN
}

/**
 * Tests if protocol_listen returns proper error message when given an invalid pointer to the callback function.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_null_callback_pointer)
{
    status_t status;
    protocol_event_t event;
    status = protocol_listen(&event, NULL);
    zassert_equal(status, KENNING_PROTOCOL_STATUS_INV_PTR);
}

/**
 * Tests if protocol_listen returns proper error message when given an invalid pointer to the event.
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_listen_null_event_pointer)
{
    status_t status;

    status = protocol_listen(NULL, get_loader);
    zassert_equal(status, KENNING_PROTOCOL_STATUS_INV_PTR);
}

// ========================================================
// transmit
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

#define TEST_PROTOCOL_TRANSMIT(_transmission, _message_count, _payload_sizes, _payload_offsets, write_data_call_count) \
    kenning_protocol_tests_setup_f();                                                                                  \
    protocol_write_data_fake.custom_fake = protocol_write_data_mock;                                                   \
    status = protocol_transmit(&_transmission);                                                                        \
    zassert_equal(status, STATUS_OK);                                                                                  \
    zassert_equal(protocol_write_data_fake.call_count, write_data_call_count);                                         \
    mock_write_buffer_hdr_idx = 0;                                                                                     \
    for (unsigned int i = 0; i < _message_count; i++)                                                                  \
    {                                                                                                                  \
        const message_hdr_t *hdr = (message_hdr_t *)(mock_write_buffer + mock_write_buffer_hdr_idx);                   \
        const uint8_t *payload = protocol_write_data_fake.arg0_history[2 * i + 1];                                     \
        zassert_equal(hdr->flags.general_purpose_flags.success, 1);                                                    \
        zassert_equal(hdr->flags.general_purpose_flags.fail, 0);                                                       \
        zassert_equal(hdr->flags.flags_iospec.serialized, 1);                                                          \
        if (i == 0)                                                                                                    \
        {                                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.first, 1);                                                  \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.first, 0);                                                  \
        }                                                                                                              \
        if (i == (_message_count - 1))                                                                                 \
        {                                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.last, 1);                                                   \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            zassert_equal(hdr->flags.general_purpose_flags.last, 0);                                                   \
        }                                                                                                              \
        zassert_equal(hdr->flags.general_purpose_flags.is_host_message, 0);                                            \
        zassert_equal(hdr->payload_size, _payload_sizes[i]);                                                           \
        zassert_equal(hdr->flow_control_flags, FLOW_CONTROL_TRANSMISSION);                                             \
        zassert_equal(hdr->message_type, _transmission.message_type);                                                  \
        if (_transmission.payload.size > 0)                                                                            \
        {                                                                                                              \
            zassert_equal((unsigned int)payload, (unsigned int)test_payload_buffer + _payload_offsets[i]);             \
        }                                                                                                              \
        mock_write_buffer_hdr_idx += sizeof(message_hdr_t) + _payload_sizes[i];                                        \
    }

    uint8_t test_payload_buffer[100];

    flags_t test_flags;
    test_flags.general_purpose_flags.success = 1;
    test_flags.general_purpose_flags.fail = 0;
    test_flags.flags_iospec.serialized = 1;

    protocol_event_t transmission;
    transmission.message_type = MESSAGE_TYPE_IOSPEC;
    transmission.flags = test_flags;
    transmission.payload.raw_bytes = test_payload_buffer;

    // In prj.conf file for the tests we override the meximum message size (setting it to 8).
    // Therefore 'protocol_transmit' function will splice the payload into 8-byte slices.
    unsigned int payload_sizes_2_messages[] = {8, 6};
    unsigned int payload_offsets_2_messages[] = {0, 8};
    transmission.payload.size = 14;
    TEST_PROTOCOL_TRANSMIT(transmission, 2, payload_sizes_2_messages, payload_offsets_2_messages, 4);
    unsigned int payload_sizes_1_message[] = {4};
    unsigned int payload_offsets_1_message[] = {0};
    transmission.payload.size = 4;
    TEST_PROTOCOL_TRANSMIT(transmission, 1, payload_sizes_1_message, payload_offsets_1_message, 2);
    unsigned int payload_sizes_7_messages[] = {8, 8, 8, 8, 8, 8, 3};
    unsigned int payload_offsets_7_messages[] = {0, 8, 16, 24, 32, 40, 48};
    transmission.payload.size = 51;
    TEST_PROTOCOL_TRANSMIT(transmission, 7, payload_sizes_7_messages, payload_offsets_7_messages, 14);
    unsigned int payload_sizes_2_messages_full[] = {8, 8};
    unsigned int payload_offsets_2_messages_full[] = {0, 8};
    transmission.payload.size = 16;
    TEST_PROTOCOL_TRANSMIT(transmission, 2, payload_sizes_2_messages_full, payload_offsets_2_messages_full, 4);
    unsigned int payload_sizes_1_message_full[] = {8};
    unsigned int payload_offsets_1_message_full[] = {0};
    transmission.payload.size = 8;
    TEST_PROTOCOL_TRANSMIT(transmission, 1, payload_sizes_1_message_full, payload_offsets_1_message_full, 2);
    unsigned int payload_sizes_7_messages_full[] = {8, 8, 8, 8, 8, 8, 8};
    unsigned int payload_offsets_7_messages_full[] = {0, 8, 16, 24, 32, 40, 48};
    transmission.payload.size = 56;
    TEST_PROTOCOL_TRANSMIT(transmission, 7, payload_sizes_7_messages_full, payload_offsets_7_messages_full, 14);
    unsigned int payload_sizes_1_messages_empty[] = {0};
    unsigned int payload_offsets_1_message_empty[] = {0};
    transmission.payload.size = 0;
    TEST_PROTOCOL_TRANSMIT(transmission, 1, payload_sizes_1_messages_empty, payload_offsets_1_message_empty, 1);

#undef TEST_PROTOCOL_TRANSMIT
}

/**
 * Tests if protocol_transmit returns proper error message when given an invalid pointer;
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_transmit_null_pointer)
{
    status_t status;

    status = protocol_transmit(NULL);
    zassert_equal(status, KENNING_PROTOCOL_STATUS_INV_PTR);
}

/**
 * Tests if protocol_transmit returns proper error message when given an invalid pointer;
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_transmit_lower_layer_error)
{
    status_t status;
    uint8_t test_payload_buffer[100];

    protocol_event_t transmission;
    transmission.message_type = MESSAGE_TYPE_IOSPEC;
    transmission.payload.raw_bytes = test_payload_buffer;
    transmission.payload.size = 14;
#define TEST_PROTOCOL_TRANSMIT(error, expected_error) \
    protocol_write_data_fake.return_val = error;      \
    status = protocol_transmit(&transmission);        \
    zassert_equal(status, expected_error);

    TEST_PROTOCOL_TRANSMIT(PROTOCOL_STATUS_INV_ARG_WORDSIZE, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_TRANSMIT(4545, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_TRANSMIT(PROTOCOL_STATUS_NO_DATA, KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR);
    TEST_PROTOCOL_TRANSMIT(PROTOCOL_STATUS_TIMEOUT, KENNING_PROTOCOL_STATUS_TIMEOUT);
#undef TEST_PROTOCOL_TRANSMIT
}

// ========================================================
// helper functions
// ========================================================
void prepare_message_in_buffer(message_type_t message_type, flags_t flags, flow_control_flags_t flow_control_flags,
                               size_t payload_size)
{
    message_hdr_t hdr_to_read = {
        .message_type = message_type,
        .flags = flags,
        .payload_size = payload_size,
        .flow_control_flags = flow_control_flags,
    };
    memcpy(mock_read_buffer + mock_read_buffer_idx, &hdr_to_read, sizeof(message_hdr_t));
    memset(mock_read_buffer + mock_read_buffer_idx + sizeof(message_hdr_t), 'x', payload_size);
    mock_read_buffer_idx += payload_size + sizeof(message_hdr_t);
}
