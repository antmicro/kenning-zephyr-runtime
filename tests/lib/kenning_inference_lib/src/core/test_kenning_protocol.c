/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include <kenning_inference_lib/core/kenning_protocol.h>

#include "utils.h"

extern uint8_t g_message_buffer[];
message_t *gp_message = NULL;
uint8_t *gp_protocol_buffer = NULL;
size_t g_protocol_data_read = 0;

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

/**
 * Prepares message of given type and payload and store result in gp_message
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 */
void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size);

// ========================================================
// setup
// ========================================================

static void kenning_protocol_tests_setup_f()
{
    MOCKS(RESET_MOCK);

    g_protocol_data_read = 0;
}

static void kenning_protocol_tests_teardown_f()
{
    if (IS_VALID_POINTER(gp_message))
    {
        free(gp_message);
    }
    if (IS_VALID_POINTER(gp_protocol_buffer))
    {
        free(gp_protocol_buffer);
    }
}

ZTEST_SUITE(kenning_inference_lib_test_kenning_protocol, NULL, NULL, kenning_protocol_tests_setup_f, NULL,
            kenning_protocol_tests_teardown_f);

// ========================================================
// receive_message
// ========================================================

/**
 * Tests if protocol receive message reads message without payload properly
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_without_payload)
{
    status_t status = STATUS_OK;
    message_t *msg;

    protocol_read_data_fake.custom_fake = protocol_read_data_mock;

#define TEST_PROTOCOL_RECV_MSG(_message_type)                   \
    prepare_message(_message_type, NULL, 0);                    \
                                                                \
    status = protocol_recv_msg(&msg);                           \
                                                                \
    zassert_equal(KENNING_PROTOCOL_STATUS_DATA_READY, status);  \
    zassert_equal(gp_message->message_size, msg->message_size); \
    zassert_equal(gp_message->message_type, msg->message_type);

    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_OK);
    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_ERROR);
    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_PROCESS);
    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_OUTPUT);

#undef TEST_PROTOCOL_RECV_MSG
}

/**
 * Tests if protocol receive message reads message with payload properly
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_with_payload)
{
    status_t status = STATUS_OK;
    uint8_t msg_payload[] = "some data";
    message_t *msg;

    protocol_read_data_fake.custom_fake = protocol_read_data_mock;

#define TEST_PROTOCOL_RECV_MSG(_message_type)                         \
    prepare_message(_message_type, msg_payload, sizeof(msg_payload)); \
                                                                      \
    status = protocol_recv_msg(&msg);                                 \
                                                                      \
    zassert_equal(KENNING_PROTOCOL_STATUS_DATA_READY, status);        \
    zassert_equal(gp_message->message_size, msg->message_size);       \
    zassert_equal(gp_message->message_type, msg->message_type);       \
    zassert_mem_equal(gp_message->payload, msg->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size));

    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_DATA);
    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_MODEL);
    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_STATS);
    TEST_PROTOCOL_RECV_MSG(MESSAGE_TYPE_IOSPEC);

#undef TEST_PROTOCOL_RECV_MSG
}

/**
 * Tests if protocol receive message fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_invalid_pointer)
{
    status_t status = STATUS_OK;
    uint8_t message_data[] = "some data";

    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    prepare_message(MESSAGE_TYPE_DATA, message_data, sizeof(message_data));

    status = protocol_recv_msg(NULL);

    zassert_equal(KENNING_PROTOCOL_STATUS_INV_PTR, status);
}

/**
 * Tests if protocol receive message fails if message size is too big
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_too_big)
{
    status_t status = STATUS_OK;
    uint8_t message_data[MAX_MESSAGE_SIZE_BYTES + 1] = {0};
    message_t *msg;

    protocol_read_data_fake.custom_fake = protocol_read_data_mock;
    prepare_message(MESSAGE_TYPE_DATA, message_data, sizeof(message_data));

    status = protocol_recv_msg(&msg);

    zassert_equal(KENNING_PROTOCOL_STATUS_MSG_TOO_BIG, status);
}

/**
 * Tests if protocol receive message fails if protocol read fails
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_read_error)
{
    status_t status = STATUS_OK;
    uint8_t message_payload[] = "some data";
    message_t *msg;

    protocol_read_data_fake.return_val = PROTOCOL_STATUS_RECV_ERROR_BUSY;
    prepare_message(MESSAGE_TYPE_DATA, message_payload, sizeof(message_payload));

    status = protocol_recv_msg(&msg);

    zassert_equal(KENNING_PROTOCOL_STATUS_CLIENT_DISCONNECTED, status);
}

/**
 * Tests if protocol receive message hits timeout if protocol read does so
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_receive_message_timeout)
{
    status_t status = STATUS_OK;
    uint8_t message_payload[] = "some data";
    message_t *msg;

    protocol_read_data_fake.return_val = PROTOCOL_STATUS_TIMEOUT;
    prepare_message(MESSAGE_TYPE_DATA, message_payload, sizeof(message_payload));

    status = protocol_recv_msg(&msg);

    zassert_equal(KENNING_PROTOCOL_STATUS_TIMEOUT, status);
}

// ========================================================
// send_message
// ========================================================

/**
 * Tests if protocol send message writes properly message without payload
 */
ZTEST(kenning_inference_lib_test_kenning_protocol, test_protocol_send_message_without_payload)
{
    status_t status = STATUS_OK;
    message_t *msg_from_buffer;

    protocol_write_data_fake.custom_fake = protocol_write_data_mock;

#define TEST_PROTOCOL_SEND_MSG(_message_type)                               \
    prepare_message(_message_type, NULL, 0);                                \
                                                                            \
    status = protocol_send_msg(gp_message);                                 \
                                                                            \
    msg_from_buffer = (message_t *)gp_protocol_buffer;                      \
                                                                            \
    zassert_equal(STATUS_OK, status);                                       \
    zassert_equal(gp_message->message_size, msg_from_buffer->message_size); \
    zassert_equal(gp_message->message_type, msg_from_buffer->message_type);

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
    message_t *msg_from_buffer;

    protocol_write_data_fake.custom_fake = protocol_write_data_mock;

#define TEST_PROTOCOL_SEND_MSG(_message_type)                               \
    prepare_message(_message_type, msg_payload, sizeof(msg_payload));       \
                                                                            \
    status = protocol_send_msg(gp_message);                                 \
                                                                            \
    msg_from_buffer = (message_t *)gp_protocol_buffer;                      \
                                                                            \
    zassert_equal(STATUS_OK, status);                                       \
    zassert_equal(gp_message->message_size, msg_from_buffer->message_size); \
    zassert_equal(gp_message->message_type, msg_from_buffer->message_type); \
    zassert_mem_equal(gp_message->payload, msg_from_buffer->payload, MESSAGE_SIZE_PAYLOAD(gp_message->message_size));

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

    protocol_write_data_fake.return_val = PROTOCOL_STATUS_RECV_ERROR;
    prepare_message(MESSAGE_TYPE_OK, NULL, 0);

    status = protocol_send_msg(gp_message);

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
    message_t *msg;

    status = protocol_prepare_success_resp(&msg);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MESSAGE_TYPE_OK, msg->message_type);
    zassert_equal(sizeof(message_type_t), msg->message_size);
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
    message_t *msg;

    status = protocol_prepare_fail_resp(&msg);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MESSAGE_TYPE_ERROR, msg->message_type);
    zassert_equal(sizeof(message_type_t), msg->message_size);
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
// mocks definitions
// ========================================================

const char *get_status_str_mock(status_t status) { return "STATUS_STR"; }

status_t protocol_read_data_mock(uint8_t *data, size_t data_length)
{
    switch (g_protocol_data_read)
    {
    case 0:
        if (data != NULL)
            memcpy(data, &gp_message->message_size, sizeof(message_size_t));
        g_protocol_data_read += data_length;
        break;
    case sizeof(message_size_t):
        if (data != NULL)
            memcpy(data, &gp_message->message_type, sizeof(message_type_t));
        g_protocol_data_read += data_length;
        break;
    case sizeof(message_t):
        if (data != NULL)
            memcpy(data, &gp_message->payload, gp_message->message_size);
        g_protocol_data_read = 0;
        break;
    default:
        return PROTOCOL_STATUS_RECV_ERROR;
    }

    return STATUS_OK;
}

status_t protocol_write_data_mock(const uint8_t *data, size_t data_length)
{
    if (IS_VALID_POINTER(gp_protocol_buffer))
    {
        free(gp_protocol_buffer);
        gp_protocol_buffer = NULL;
    }

    gp_protocol_buffer = malloc(data_length);
    memcpy(gp_protocol_buffer, data, data_length);

    return STATUS_OK;
}

// ========================================================
// helper functions
// ========================================================

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size)
{
    if (IS_VALID_POINTER(gp_message))
    {
        free(gp_message);
        gp_message = NULL;
    }
    gp_message = malloc(sizeof(message_t) + payload_size);
    gp_message->message_size = sizeof(message_type_t) + payload_size;
    gp_message->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy(gp_message->payload, payload, payload_size);
    }
}
