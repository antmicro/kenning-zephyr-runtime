/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include <kenning_inference_lib/core/callbacks.h>
#include <kenning_inference_lib/core/inference_server.h>
#include <kenning_inference_lib/core/kenning_protocol.h>
#include <kenning_inference_lib/core/model.h>

#include "utils.h"

static protocol_event_t gp_event_to_recv;
static outgoing_message_t gp_resp_message_to_send;

const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};
const char *const FLOW_CONTROL_STR[] = {FLOW_CONTROL_VALUES(GENERATE_STR)};
callback_ptr_t g_msg_callback[NUM_MESSAGE_TYPES] = {
#define ENTRY(msg_type, callback_func) callback_func,
    CALLBACKS_TABLE(ENTRY)
#undef ENTRY
};

struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_LOADER_TYPES];
// ========================================================
// mocks
// ========================================================

typedef struct msg_loader *(*loader_callback_t)(message_type_t);

DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                                                \
    MOCK(const char *, get_status_str, status_t)                                   \
    MOCK(status_t, protocol_init)                                                  \
    MOCK(status_t, model_init)                                                     \
    MOCK(status_t, unsupported_callback, protocol_event_t *, protocol_payload_t *) \
    MOCK(status_t, ping_callback, protocol_event_t *, protocol_payload_t *)        \
    MOCK(status_t, ok_callback, protocol_event_t *, protocol_payload_t *)          \
    MOCK(status_t, error_callback, protocol_event_t *, protocol_payload_t *)       \
    MOCK(status_t, data_callback, protocol_event_t *, protocol_payload_t *)        \
    MOCK(status_t, model_callback, protocol_event_t *, protocol_payload_t *)       \
    MOCK(status_t, process_callback, protocol_event_t *, protocol_payload_t *)     \
    MOCK(status_t, output_callback, protocol_event_t *, protocol_payload_t *)      \
    MOCK(status_t, stats_callback, protocol_event_t *, protocol_payload_t *)       \
    MOCK(status_t, iospec_callback, protocol_event_t *, protocol_payload_t *)      \
    MOCK(status_t, runtime_callback, protocol_event_t *, protocol_payload_t *)     \
    MOCK(status_t, protocol_transmit, const protocol_event_t *)                    \
    MOCK(status_t, protocol_listen, protocol_event_t *, loader_callback_t)

MOCKS(DECLARE_MOCK);

const char *get_status_str_mock(status_t);

status_t protocol_listen_mock(protocol_event_t *event, loader_callback_t callback);

status_t protocol_transmit_mock(const protocol_event_t *msg);

/**
 * Mock of runtime callback without response
 *
 * @param request incoming message
 */
status_t callback_without_response_mock(protocol_event_t *request, protocol_payload_t *resp);

/**
 * Mock of runtime callback with success response
 *
 * @param request incoming message
 */
status_t callback_with_ok_response_mock(protocol_event_t *request, protocol_payload_t *resp);

/**
 * Mock of runtime callback with success response with payload
 *
 * @param request incoming message
 */
status_t callback_with_ok_response_with_payload_mock(protocol_event_t *request, protocol_payload_t *resp);

/**
 * Mock of runtime callback that returns error
 *
 * @param request incoming message
 */
status_t callback_error_mock(protocol_event_t *request, protocol_payload_t *resp);

// ========================================================
// helper functions declarations
// ========================================================
/**
 * Prepares message of given type and payload
 *
 * @param msg_type type of the message
 * @param payload_size size of the payload
 */
protocol_event_t prepare_event_to_recv(message_type_t msg_type, size_t payload_size);

// ========================================================
// setup
// ========================================================

static void inference_server_tests_setup_f() { MOCKS(RESET_MOCK); }

static void inference_server_tests_teardown_f() {}

ZTEST_SUITE(kenning_inference_lib_test_inference_server, NULL, NULL, inference_server_tests_setup_f,
            inference_server_tests_teardown_f, NULL);

// ========================================================
// init_server
// ========================================================

/**
 * Tests if init server initializes protocol
 */
ZTEST(kenning_inference_lib_test_inference_server, test_init_server)
{
    status_t status = STATUS_OK;

    protocol_init_fake.return_val = STATUS_OK;
    model_init_fake.return_val = STATUS_OK;

    status = init_server();

    zassert_equal(STATUS_OK, status);
    zassert_equal(protocol_init_fake.call_count, 1);
    zassert_equal(model_init_fake.call_count, 1);
}

/**
 * Tests if init server fails when protocol init fails
 */
ZTEST(kenning_inference_lib_test_inference_server, test_init_server_protocol_error)
{
    status_t status = STATUS_OK;

    protocol_init_fake.return_val = PROTOCOL_STATUS_ERROR;

    status = init_server();

    zassert_equal(PROTOCOL_STATUS_ERROR, status);
    zassert_equal(protocol_init_fake.call_count, 1);
}

/**
 * Tests if init server fails when model init fails
 */
ZTEST(kenning_inference_lib_test_inference_server, test_init_server_model_error)
{
    status_t status = STATUS_OK;

    model_init_fake.return_val = MODEL_STATUS_ERROR;

    status = init_server();

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_init_fake.call_count, 1);
}

// ========================================================
// wait_for_protocol_event
// ========================================================

/**
 * Tests if wait for message properly receives message from protocol
 */
ZTEST(kenning_inference_lib_test_inference_server, test_wait_for_message)
{
    status_t status = STATUS_OK;
    static protocol_event_t request;

    protocol_listen_fake.custom_fake = protocol_listen_mock;
    gp_event_to_recv = prepare_event_to_recv(MESSAGE_TYPE_MODEL, 0);

    status = wait_for_protocol_event(&request);

    zassert_equal(STATUS_OK, status);
    zassert_equal(gp_event_to_recv.payload.size, request.payload.size);
    zassert_equal(gp_event_to_recv.message_type, request.message_type);
    zassert_equal(protocol_listen_fake.call_count, 1);
}

/**
 * Tests if wait for message fails if protocol receive message fails
 */
ZTEST(kenning_inference_lib_test_inference_server, test_wait_for_message_protocol_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request;
    memset(&request, 0, sizeof(protocol_event_t));

    protocol_listen_fake.return_val = KENNING_PROTOCOL_STATUS_ERROR;

    status = wait_for_protocol_event(&request);

    zassert_equal(INFERENCE_SERVER_STATUS_ERROR, status);
    zassert_equal(protocol_listen_fake.call_count, 1);
}

// ========================================================
// handle_protocol_event
// ========================================================

/**
 * Tests if handle message calls proper callback for messages without response
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_without_response)
{
    status_t status = STATUS_OK;
    protocol_event_t transmission;

    protocol_transmit_fake.custom_fake = protocol_transmit_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                          \
    protocol_transmit_fake.call_count = 0;                          \
    transmission = prepare_event_to_recv(_message_type, 0);         \
    transmission.is_request = 0;                                    \
    g_msg_callback[_message_type] = callback_without_response_mock; \
                                                                    \
    status = handle_protocol_event(&transmission);                  \
                                                                    \
    zassert_equal(STATUS_OK, status);                               \
    zassert_equal(protocol_transmit_fake.call_count, 0);

    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_MODEL);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_STATUS);

#undef TEST_HANDLE_MESSAGE
}

/**
 * Tests if handle message calls proper callback for messages with success response without payload
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_with_success_response_without_payload)
{
    status_t status = STATUS_OK;
    protocol_event_t request;

    protocol_transmit_fake.custom_fake = protocol_transmit_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                                  \
    protocol_transmit_fake.call_count = 0;                                  \
    request = prepare_event_to_recv(_message_type, 0);                      \
    g_msg_callback[_message_type] = callback_with_ok_response_mock;         \
                                                                            \
    status = handle_protocol_event(&request);                               \
                                                                            \
    zassert_equal(STATUS_OK, status);                                       \
    zassert_equal(0, gp_resp_message_to_send.hdr.payload_size);             \
    zassert_equal(_message_type, gp_resp_message_to_send.hdr.message_type); \
    zassert_equal(protocol_transmit_fake.call_count, 1);                    \
    zassert_equal(gp_resp_message_to_send.hdr.flags.general_purpose_flags.success, 1);

    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_DATA);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_MODEL);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_PROCESS);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_IOSPEC);

#undef TEST_HANDLE_MESSAGE
}

/**
 * Tests if handle message calls proper callback for messages with failure response without payload
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_with_failure_response_without_payload)
{
    status_t status = STATUS_OK;
    protocol_event_t request;

    protocol_transmit_fake.custom_fake = protocol_transmit_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                                  \
    protocol_transmit_fake.call_count = 0;                                  \
    request = prepare_event_to_recv(_message_type, 0);                      \
    g_msg_callback[_message_type] = callback_error_mock;                    \
                                                                            \
    status = handle_protocol_event(&request);                               \
                                                                            \
    zassert_equal(STATUS_OK, status);                                       \
    zassert_equal(0, gp_resp_message_to_send.hdr.payload_size);             \
    zassert_equal(_message_type, gp_resp_message_to_send.hdr.message_type); \
    zassert_equal(protocol_transmit_fake.call_count, 1);                    \
    zassert_equal(gp_resp_message_to_send.hdr.flags.general_purpose_flags.fail, 1);

    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_DATA);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_MODEL);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_PROCESS);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_OUTPUT);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_STATS);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_IOSPEC);

#undef TEST_HANDLE_MESSAGE
}

/**
 * Tests if handle message calls proper callback for messages with success response with payload
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_with_success_response_with_payload)
{
    status_t status = STATUS_OK;
    protocol_event_t request;

    protocol_transmit_fake.custom_fake = protocol_transmit_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                                       \
    protocol_transmit_fake.call_count = 0;                                       \
    request = prepare_event_to_recv(_message_type, 0);                           \
    g_msg_callback[_message_type] = callback_with_ok_response_with_payload_mock; \
    status = handle_protocol_event(&request);                                    \
    zassert_equal(STATUS_OK, status);                                            \
    zassert_equal(128, gp_resp_message_to_send.hdr.payload_size);                \
    zassert_equal(_message_type, gp_resp_message_to_send.hdr.message_type);      \
    zassert_equal(protocol_transmit_fake.call_count, 1);                         \
    zassert_equal(gp_resp_message_to_send.hdr.flags.general_purpose_flags.success, 1);

    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_OUTPUT);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_STATS);

#undef TEST_HANDLE_MESSAGE
}

/**
 * Tests if handle message properly sends error message when callback fails
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_callback_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request;

    protocol_transmit_fake.custom_fake = protocol_transmit_mock;

#define TEST_HANDLE_MESSAGE(_message_type)               \
    protocol_transmit_fake.call_count = 0;               \
    request = prepare_event_to_recv(_message_type, 0);   \
    g_msg_callback[_message_type] = callback_error_mock; \
    status = handle_protocol_event(&request);            \
    zassert_equal(STATUS_OK, status);                    \
    zassert_equal(protocol_transmit_fake.call_count, 1); \
    zassert_equal(gp_resp_message_to_send.hdr.flags.general_purpose_flags.fail, 1);

    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_PING);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_STATUS);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_DATA);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_MODEL);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_PROCESS);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_OUTPUT);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_STATS);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_IOSPEC);

#undef TEST_HANDLE_MESSAGE
}

/**
 * Tests if handle message does nothing when message pointer is invalid
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_inv_ptr)
{
    status_t status = STATUS_OK;

    status = handle_protocol_event(NULL);

    zassert_equal(INFERENCE_SERVER_STATUS_INV_PTR, status);
    zassert_equal(protocol_transmit_fake.call_count, 0);
}

// ========================================================
// mocks
// ========================================================

const char *get_status_str_mock(status_t status) { return "STATUS_STR"; }

status_t protocol_listen_mock(protocol_event_t *event, loader_callback_t callback)
{
    *event = gp_event_to_recv;
    return STATUS_OK;
}

status_t protocol_transmit_mock(const protocol_event_t *event)
{
    gp_resp_message_to_send.payload = event->payload.raw_bytes;
    gp_resp_message_to_send.hdr.message_type = event->message_type;
    gp_resp_message_to_send.hdr.flags = event->flags;
    gp_resp_message_to_send.hdr.payload_size = event->payload.size;

    return STATUS_OK;
}

status_t callback_without_response_mock(protocol_event_t *request, protocol_payload_t *resp) { return STATUS_OK; }

status_t callback_with_ok_response_mock(protocol_event_t *request, protocol_payload_t *resp)
{
    resp->size = 0;
    memset(resp->raw_bytes, 0, CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE);
    return STATUS_OK;
}

status_t callback_with_ok_response_with_payload_mock(protocol_event_t *request, protocol_payload_t *resp)
{
    const int payload_size = 128;
    resp->size = payload_size;
    memset(resp->raw_bytes, 0, CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE);
    memset(resp->raw_bytes, 'x', payload_size);
    return STATUS_OK;
}

status_t callback_error_mock(protocol_event_t *request, protocol_payload_t *resp)
{
    resp->size = 0;
    memset(resp->raw_bytes, 0, CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE);
    return CALLBACKS_STATUS_ERROR;
}

// ========================================================
// helper functions
// ========================================================

protocol_event_t prepare_event_to_recv(message_type_t msg_type, size_t payload_size)
{
    protocol_event_t event;

    event.is_request = 1;
    event.payload.size = payload_size;
    event.message_type = msg_type;
    return event;
}
