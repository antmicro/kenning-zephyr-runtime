/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include <kenning_inference_lib/core/callbacks.h>
#include <kenning_inference_lib/core/inference_server.h>
#include <kenning_inference_lib/core/kenning_protocol.h>

#include "utils.h"

message_t *gp_message = NULL;
message_t *gp_message_sent = NULL;
message_t *gp_message_to_receive = NULL;

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

#define MOCKS(MOCK)                                      \
    MOCK(const char *, get_status_str, status_t)         \
    MOCK(status_t, protocol_init)                        \
    MOCK(status_t, protocol_recv_msg, message_t **)      \
    MOCK(status_t, protocol_send_msg, const message_t *) \
    MOCK(status_t, ok_callback, message_t **)            \
    MOCK(status_t, error_callback, message_t **)         \
    MOCK(status_t, data_callback, message_t **)          \
    MOCK(status_t, model_callback, message_t **)         \
    MOCK(status_t, process_callback, message_t **)       \
    MOCK(status_t, output_callback, message_t **)        \
    MOCK(status_t, stats_callback, message_t **)         \
    MOCK(status_t, iospec_callback, message_t **)

MOCKS(DECLARE_MOCK);

const char *get_status_str_mock(status_t);

status_t protocol_recv_msg_mock(message_t **msg);

status_t protocol_send_msg_mock(const message_t *msg);

/**
 * Mock of runtime callback without response
 *
 * @param request incoming message
 */
status_t callback_without_response_mock(message_t **request);

/**
 * Mock of runtime callback with success response
 *
 * @param request incoming message
 */
status_t callback_with_ok_response_mock(message_t **request);

/**
 * Mock of runtime callback with error response
 *
 * @param request incoming message
 */
status_t callback_with_error_response_mock(message_t **request);

/**
 * Mock of runtime callback with success response with payload
 *
 * @param request incoming message
 */
status_t callback_with_ok_response_with_payload_mock(message_t **request);

/**
 * Mock of runtime callback that returns error
 *
 * @param request incoming message
 */
status_t callback_error_mock(message_t **request);

// ========================================================
// helper functions declarations
// ========================================================
/**
 * Prepares message of given type and payload
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 * @param msg prepared message
 */
void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message_t **msg);

// ========================================================
// setup
// ========================================================

static void inference_server_tests_setup_f()
{
    MOCKS(RESET_MOCK);

    if (IS_VALID_POINTER(gp_message))
    {
        free(gp_message);
        gp_message = NULL;
    }
    if (IS_VALID_POINTER(gp_message_sent))
    {
        free(gp_message_sent);
        gp_message_sent = NULL;
    }
    if (IS_VALID_POINTER(gp_message_to_receive))
    {
        free(gp_message_to_receive);
        gp_message_to_receive = NULL;
    }
}

ZTEST_SUITE(kenning_inference_lib_test_inference_server, NULL, NULL, inference_server_tests_setup_f, NULL, NULL);

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

    status = init_server();

    zassert_equal(STATUS_OK, status);
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
    message_t *msg = NULL;

    protocol_recv_msg_fake.custom_fake = protocol_recv_msg_mock;
    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message_to_receive);

    status = wait_for_message(&msg);

    zassert_equal(STATUS_OK, status);
    zassert_equal(gp_message_to_receive, msg);
}

/**
 * Tests if wait for message fails if protocol receive message fails
 */
ZTEST(kenning_inference_lib_test_inference_server, test_wait_for_message_protocol_error)
{
    status_t status = STATUS_OK;
    message_t *msg = NULL;

    protocol_recv_msg_fake.return_val = KENNING_PROTOCOL_STATUS_ERROR;

    status = wait_for_message(&msg);

    zassert_equal(INFERENCE_SERVER_STATUS_ERROR, status);
}

// ========================================================
// handle_message
// ========================================================

/**
 * Tests if handle message calls proper callback for messages without response
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_without_response)
{
    status_t status = STATUS_OK;

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                          \
    prepare_message(_message_type, NULL, 0, &gp_message);           \
    g_msg_callback[_message_type] = callback_without_response_mock; \
                                                                    \
    status = handle_message(gp_message);                            \
                                                                    \
    zassert_equal(STATUS_OK, status);                               \
    zassert_is_null(gp_message_sent);

    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_OK);
    TEST_HANDLE_MESSAGE(MESSAGE_TYPE_ERROR);

#undef TEST_HANDLE_MESSAGE
}

/**
 * Tests if handle message calls proper callback for messages with success response without payload
 */
ZTEST(kenning_inference_lib_test_inference_server, test_handle_message_with_success_response_without_payload)
{
    status_t status = STATUS_OK;

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                          \
    prepare_message(_message_type, NULL, 0, &gp_message);           \
    g_msg_callback[_message_type] = callback_with_ok_response_mock; \
                                                                    \
    status = handle_message(gp_message);                            \
                                                                    \
    zassert_equal(STATUS_OK, status);                               \
    zassert_not_null(gp_message_sent);                              \
    zassert_equal(MESSAGE_TYPE_OK, gp_message_sent->message_type);  \
    zassert_equal(sizeof(message_type_t), gp_message_sent->message_size);

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

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                             \
    prepare_message(_message_type, NULL, 0, &gp_message);              \
    g_msg_callback[_message_type] = callback_with_error_response_mock; \
                                                                       \
    status = handle_message(gp_message);                               \
                                                                       \
    zassert_equal(STATUS_OK, status);                                  \
    zassert_not_null(gp_message_sent);                                 \
    zassert_equal(MESSAGE_TYPE_ERROR, gp_message_sent->message_type);  \
    zassert_equal(sizeof(message_type_t), gp_message_sent->message_size);

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

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                                       \
    prepare_message(_message_type, NULL, 0, &gp_message);                        \
    g_msg_callback[_message_type] = callback_with_ok_response_with_payload_mock; \
                                                                                 \
    status = handle_message(gp_message);                                         \
                                                                                 \
    zassert_equal(STATUS_OK, status);                                            \
    zassert_not_null(gp_message_sent);                                           \
    zassert_equal(MESSAGE_TYPE_OK, gp_message_sent->message_type);               \
    zassert_between_inclusive(gp_message_sent->message_size, sizeof(message_t) + 1, MAX_MESSAGE_SIZE_BYTES);

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

    protocol_send_msg_fake.custom_fake = protocol_send_msg_mock;

#define TEST_HANDLE_MESSAGE(_message_type)                            \
    prepare_message(_message_type, NULL, 0, &gp_message);             \
    g_msg_callback[_message_type] = callback_error_mock;              \
                                                                      \
    status = handle_message(gp_message);                              \
                                                                      \
    zassert_equal(STATUS_OK, status);                                 \
    zassert_not_null(gp_message_sent);                                \
    zassert_equal(MESSAGE_TYPE_ERROR, gp_message_sent->message_type); \
    zassert_equal(gp_message_sent->message_size, sizeof(message_type_t));

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
}

// ========================================================
// mocks
// ========================================================

const char *get_status_str_mock(status_t status) { return "STATUS_STR"; }

status_t protocol_recv_msg_mock(message_t **msg)
{
    *msg = gp_message_to_receive;

    return KENNING_PROTOCOL_STATUS_DATA_READY;
}

status_t protocol_send_msg_mock(const message_t *msg)
{
    gp_message_sent = malloc(sizeof(message_size_t) + msg->message_size);
    memcpy(gp_message_sent, msg, sizeof(message_size_t) + msg->message_size);

    return STATUS_OK;
}

status_t callback_without_response_mock(message_t **request)
{
    *request = NULL;

    return STATUS_OK;
}

status_t callback_with_ok_response_mock(message_t **request)
{
    *request = NULL;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0, request);

    return STATUS_OK;
}

status_t callback_with_error_response_mock(message_t **request)
{
    *request = NULL;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, request);

    return STATUS_OK;
}

status_t callback_with_ok_response_with_payload_mock(message_t **request)
{
    uint8_t payload[512];
    *request = NULL;

    prepare_message(MESSAGE_TYPE_OK, payload, sizeof(payload), request);

    return STATUS_OK;
}

status_t callback_error_mock(message_t **request)
{
    *request = NULL;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, request);

    return CALLBACKS_STATUS_ERROR;
}

// ========================================================
// helper functions
// ========================================================

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message_t **msg)
{
    if (IS_VALID_POINTER(*msg))
    {
        free(*msg);
        *msg = NULL;
    }
    *msg = malloc(sizeof(message_size_t) + payload_size);
    (*msg)->message_size = sizeof(message_type_t) + payload_size;
    (*msg)->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy((*msg)->payload, payload, payload_size);
    }
}
