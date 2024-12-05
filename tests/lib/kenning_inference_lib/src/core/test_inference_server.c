/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
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

static message_hdr_t gp_message_hdr_to_recv;
static resp_message_t gp_resp_message_to_send;

const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};
callback_ptr_t g_msg_callback[NUM_MESSAGE_TYPES] = {
#define ENTRY(msg_type, callback_func) callback_func,
    CALLBACKS_TABLE(ENTRY)
#undef ENTRY
};

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                                         \
    MOCK(const char *, get_status_str, status_t)                            \
    MOCK(status_t, protocol_init)                                           \
    MOCK(status_t, model_init)                                              \
    MOCK(status_t, protocol_recv_msg, message_hdr_t *)                      \
    MOCK(status_t, protocol_send_msg, const resp_message_t *)               \
    MOCK(status_t, unsupported_callback, message_hdr_t *, resp_message_t *) \
    MOCK(status_t, ok_callback, message_hdr_t *, resp_message_t *)          \
    MOCK(status_t, error_callback, message_hdr_t *, resp_message_t *)       \
    MOCK(status_t, data_callback, message_hdr_t *, resp_message_t *)        \
    MOCK(status_t, model_callback, message_hdr_t *, resp_message_t *)       \
    MOCK(status_t, process_callback, message_hdr_t *, resp_message_t *)     \
    MOCK(status_t, output_callback, message_hdr_t *, resp_message_t *)      \
    MOCK(status_t, stats_callback, message_hdr_t *, resp_message_t *)       \
    MOCK(status_t, iospec_callback, message_hdr_t *, resp_message_t *)      \
    MOCK(status_t, runtime_callback, message_hdr_t *, resp_message_t *)

MOCKS(DECLARE_MOCK);

const char *get_status_str_mock(status_t);

status_t protocol_recv_msg_mock(message_hdr_t *hdr);

status_t protocol_send_msg_mock(const resp_message_t *msg);

/**
 * Mock of runtime callback without response
 *
 * @param request incoming message
 */
status_t callback_without_response_mock(message_hdr_t *hdr, resp_message_t *resp);

/**
 * Mock of runtime callback with success response
 *
 * @param request incoming message
 */
status_t callback_with_ok_response_mock(message_hdr_t *hdr, resp_message_t *resp);

/**
 * Mock of runtime callback with error response
 *
 * @param request incoming message
 */
status_t callback_with_error_response_mock(message_hdr_t *hdr, resp_message_t *resp);

/**
 * Mock of runtime callback with success response with payload
 *
 * @param request incoming message
 */
status_t callback_with_ok_response_with_payload_mock(message_hdr_t *hdr, resp_message_t *resp);

/**
 * Mock of runtime callback that returns error
 *
 * @param request incoming message
 */
status_t callback_error_mock(message_hdr_t *hdr, resp_message_t *resp);

// ========================================================
// helper functions declarations
// ========================================================
/**
 * Prepares message of given type and payload
 *
 * @param msg_type type of the message
 * @param payload_size size of the payload
 */
message_hdr_t prepare_message_header(message_type_t msg_type, size_t payload_size);

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
// wait_for_message
// ========================================================

/**
 * Tests if wait for message properly receives message from protocol
 */
ZTEST(kenning_inference_lib_test_inference_server, test_wait_for_message)
{
    status_t status = STATUS_OK;
    static message_hdr_t hdr;
    memset(&hdr, 0, sizeof(message_hdr_t));

    protocol_recv_msg_fake.custom_fake = protocol_recv_msg_mock;
    gp_message_hdr_to_recv = prepare_message_header(MESSAGE_TYPE_OK, 0);

    status = wait_for_message(&hdr);

    zassert_equal(STATUS_OK, status);
    zassert_equal(gp_message_hdr_to_recv.message_size, hdr.message_size);
    zassert_equal(gp_message_hdr_to_recv.message_type, hdr.message_type);
    zassert_equal(protocol_recv_msg_fake.call_count, 1);
}

/**
 * Tests if wait for message fails if protocol receive message fails
 */
ZTEST(kenning_inference_lib_test_inference_server, test_wait_for_message_protocol_error)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    memset(&hdr, 0, sizeof(message_hdr_t));

    protocol_recv_msg_fake.return_val = KENNING_PROTOCOL_STATUS_ERROR;

    status = wait_for_message(&hdr);

    zassert_equal(INFERENCE_SERVER_STATUS_ERROR, status);
    zassert_equal(protocol_recv_msg_fake.call_count, 1);
}

// ========================================================
// handle_message
// ========================================================

/**
 * Tests if handle message calls proper callback for messages with success response without payload
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_with_success_response_without_payload)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                                         \
    protocol_send_msg_fake.call_count = 0;                                         \
    hdr = prepare_message_header(_message_type, 0);                                \
    g_msg_callback[_message_type] = callback_with_ok_response_mock;                \
                                                                                   \
    status = handle_message(&hdr);                                                 \
                                                                                   \
    zassert_equal(STATUS_OK, status);                                              \
    zassert_equal(MESSAGE_SIZE_FULL(0), gp_resp_message_to_send.hdr.message_size); \
    zassert_equal(MESSAGE_TYPE_OK, gp_resp_message_to_send.hdr.message_type);      \
    zassert_equal(protocol_send_msg_fake.call_count, 1);

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
    message_hdr_t hdr;

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                                         \
    protocol_send_msg_fake.call_count = 0;                                         \
    hdr = prepare_message_header(_message_type, 0);                                \
    g_msg_callback[_message_type] = callback_with_error_response_mock;             \
                                                                                   \
    status = handle_message(&hdr);                                                 \
                                                                                   \
    zassert_equal(STATUS_OK, status);                                              \
    zassert_equal(MESSAGE_SIZE_FULL(0), gp_resp_message_to_send.hdr.message_size); \
    zassert_equal(MESSAGE_TYPE_ERROR, gp_resp_message_to_send.hdr.message_type);   \
    zassert_equal(protocol_send_msg_fake.call_count, 1);

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
    message_hdr_t hdr;

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                                           \
    protocol_send_msg_fake.call_count = 0;                                           \
    hdr = prepare_message_header(_message_type, 0);                                  \
    g_msg_callback[_message_type] = callback_with_ok_response_with_payload_mock;     \
    status = handle_message(&hdr);                                                   \
    zassert_equal(STATUS_OK, status);                                                \
    zassert_equal(MESSAGE_SIZE_FULL(128), gp_resp_message_to_send.hdr.message_size); \
    zassert_equal(MESSAGE_TYPE_OK, gp_resp_message_to_send.hdr.message_type);        \
    zassert_equal(protocol_send_msg_fake.call_count, 1);

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
    message_hdr_t hdr;

    protocol_send_msg_fake.return_val = STATUS_OK;

#define TEST_HANDLE_MESSAGE(_message_type)               \
    protocol_send_msg_fake.call_count = 0;               \
    hdr = prepare_message_header(_message_type, 0);      \
    g_msg_callback[_message_type] = callback_error_mock; \
    status = handle_message(&hdr);                       \
    zassert_equal(CALLBACKS_STATUS_ERROR, status);       \
    zassert_equal(protocol_send_msg_fake.call_count, 0);

    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_OK);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_ERROR);
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

    status = handle_message(NULL);

    zassert_equal(INFERENCE_SERVER_STATUS_INV_PTR, status);
    zassert_equal(protocol_send_msg_fake.call_count, 0);
}

// ========================================================
// mocks
// ========================================================

const char *get_status_str_mock(status_t status) { return "STATUS_STR"; }

status_t protocol_recv_msg_mock(message_hdr_t *hdr)
{
    *hdr = gp_message_hdr_to_recv;

    return STATUS_OK;
}

status_t protocol_send_msg_mock(const resp_message_t *msg)
{
    gp_resp_message_to_send = *msg;

    return STATUS_OK;
}

status_t callback_with_ok_response_mock(message_hdr_t *hdr, resp_message_t *resp)
{
    resp->hdr.message_type = MESSAGE_TYPE_OK;
    resp->hdr.message_size = MESSAGE_SIZE_FULL(0);
    memset(resp->payload, 0, CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE);
    return STATUS_OK;
}

status_t callback_with_error_response_mock(message_hdr_t *hdr, resp_message_t *resp)
{
    resp->hdr.message_type = MESSAGE_TYPE_ERROR;
    resp->hdr.message_size = MESSAGE_SIZE_FULL(0);
    memset(resp->payload, 0, CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE);
    return STATUS_OK;
}

status_t callback_with_ok_response_with_payload_mock(message_hdr_t *hdr, resp_message_t *resp)
{
    const int payload_size = 128;
    resp->hdr.message_type = MESSAGE_TYPE_OK;
    resp->hdr.message_size = MESSAGE_SIZE_FULL(payload_size);
    memset(resp->payload, 0, CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE);
    memset(resp->payload, 'x', payload_size);
    return STATUS_OK;
}

status_t callback_error_mock(message_hdr_t *hdr, resp_message_t *resp)
{
    resp->hdr.message_type = MESSAGE_TYPE_OK;
    resp->hdr.message_size = MESSAGE_SIZE_FULL(0);
    memset(resp->payload, 0, CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE);
    return CALLBACKS_STATUS_ERROR;
}

// ========================================================
// helper functions
// ========================================================

message_hdr_t prepare_message_header(message_type_t msg_type, size_t payload_size)
{
    message_hdr_t hdr;
    hdr.message_size = payload_size + sizeof(message_type_t);
    hdr.message_type = msg_type;
    return hdr;
}
