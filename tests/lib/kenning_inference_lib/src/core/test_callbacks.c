/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include <kenning_inference_lib/core/callbacks.h>
#include <kenning_inference_lib/core/kenning_protocol.h>
#include <kenning_inference_lib/core/model.h>
#include <kenning_inference_lib/core/utils.h>

#include "utils.h"

static uint8_t g_message_buffer[MAX_MESSAGE_SIZE_BYTES];
message_t *gp_message = NULL;

GENERATE_MODULE_STATUSES_STR(MODEL);
GENERATE_MODULE_STATUSES_STR(KENNING_PROTOCOL);
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                                     \
    MOCK(const char *, get_status_str, status_t)                        \
    MOCK(status_t, protocol_prepare_success_resp, message_t **)         \
    MOCK(status_t, protocol_prepare_fail_resp, message_t **)            \
    MOCK(status_t, model_load_struct, const uint8_t *, size_t)          \
    MOCK(status_t, model_load_weights, const uint8_t *, size_t)         \
    MOCK(status_t, model_load_input, const uint8_t *, size_t)           \
    MOCK(status_t, model_run)                                           \
    MOCK(status_t, model_get_output, const size_t, uint8_t *, size_t *) \
    MOCK(status_t, model_get_statistics, const size_t, uint8_t *, size_t *)

MOCKS(DECLARE_MOCK);

const char *get_status_str_mock(status_t);

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

static void callbacks_tests_setup_f()
{
    MOCKS(RESET_MOCK);

    get_status_str_fake.custom_fake = get_status_str_mock;

    protocol_prepare_success_resp_fake.return_val = STATUS_OK;
    protocol_prepare_fail_resp_fake.return_val = STATUS_OK;
}

ZTEST_SUITE(kenning_inference_lib_test_callbacks, NULL, NULL, callbacks_tests_setup_f, NULL, NULL);

// ========================================================
// ok_callback
// ========================================================

/**
 * Tests if ok callback has no response
 */
ZTEST(kenning_inference_lib_test_callbacks, test_ok_callback)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_OK, NULL, 0, &gp_message);

    status = ok_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_is_null(gp_message);
}

/**
 * Tests if ok callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_ok_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = ok_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
}

/**
 * Tests if ok callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_ok_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_OK_CALLBACK(_message_type)                   \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = ok_callback(&gp_message);                    \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status);

    TEST_OK_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_OK_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_OK_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_OK_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_OK_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_OK_CALLBACK(MESSAGE_TYPE_STATS);
    TEST_OK_CALLBACK(MESSAGE_TYPE_IOSPEC);

#undef TEST_OK_CALLBACK
}

// ========================================================
// error_callback
// ========================================================

/**
 * Tests if error callback has no response
 */
ZTEST(kenning_inference_lib_test_callbacks, test_error_callback)
{
    status_t status = STATUS_OK;

    prepare_message(MESSAGE_TYPE_ERROR, NULL, 0, &gp_message);

    status = error_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_is_null(gp_message);
}

/**
 * Tests if error callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_error_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = error_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
}

/**
 * Tests if error callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_error_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_ERROR_CALLBACK(_message_type)                \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = error_callback(&gp_message);                 \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status);

    TEST_ERROR_CALLBACK(MESSAGE_TYPE_OK);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_STATS);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_IOSPEC);

#undef TEST_ERROR_CALLBACK
}

// ========================================================
// data_callback
// ========================================================

/**
 * Tests if data callback properly loads model input
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    model_load_input_fake.return_val = STATUS_OK;
    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data), &gp_message);

    status = data_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_equal(gp_message->payload, model_load_input_fake.arg0_val);
    zassert_equal(MESSAGE_SIZE_PAYLOAD(gp_message->message_size), model_load_input_fake.arg1_val);
    zassert_equal(1, model_load_input_fake.call_count);
}
/**
 * Tests if data callback fails if model input loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_model_error)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    model_load_input_fake.return_val = MODEL_STATUS_ERROR;
    prepare_message(MESSAGE_TYPE_DATA, data, sizeof(data), &gp_message);

    status = data_callback(&gp_message);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(1, model_load_input_fake.call_count);
}

/**
 * Tests if data callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = data_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(0, model_load_input_fake.call_count);
}

/**
 * Tests if data callback fails for ivalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_DATA_CALLBACK(_message_type)                 \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = data_callback(&gp_message);                  \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(0, model_load_input_fake.call_count);

    TEST_DATA_CALLBACK(MESSAGE_TYPE_OK);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_STATS);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_IOSPEC);

#undef TEST_DATA_CALLBACK
}

// ========================================================
// model_callback
// ========================================================

/**
 * Tests if model callback properly loads model weights
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    model_load_weights_fake.return_val = STATUS_OK;
    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data), &gp_message);

    status = model_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_equal(gp_message->payload, model_load_weights_fake.arg0_val);
    zassert_equal(MESSAGE_SIZE_PAYLOAD(gp_message->message_size), model_load_weights_fake.arg1_val);
    zassert_equal(1, model_load_weights_fake.call_count);
}

/**
 * Tests if model callback fails if model weights loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_model_error)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    model_load_weights_fake.return_val = MODEL_STATUS_ERROR;
    prepare_message(MESSAGE_TYPE_MODEL, data, sizeof(data), &gp_message);

    status = model_callback(&gp_message);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(1, model_load_weights_fake.call_count);
}

/**
 * Tests if model callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = model_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(0, model_load_weights_fake.call_count);
}

/**
 * Tests if model callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_MODEL_CALLBACK(_message_type)                \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = model_callback(&gp_message);                 \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(0, model_load_weights_fake.call_count);

    TEST_MODEL_CALLBACK(MESSAGE_TYPE_OK);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_STATS);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_IOSPEC);

#undef TEST_MODEL_CALLBACK
}

// ========================================================
// process_callback
// ========================================================

/**
 * Tests if process callback properly runs model inference
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback)
{
    status_t status = STATUS_OK;

    model_run_fake.return_val = STATUS_OK;
    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0, &gp_message);

    status = process_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_equal(1, model_run_fake.call_count);
}

/**
 * Tests if process callback fails if model inference fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_model_error)
{
    status_t status = STATUS_OK;

    model_run_fake.return_val = MODEL_STATUS_ERROR;
    prepare_message(MESSAGE_TYPE_PROCESS, NULL, 0, &gp_message);

    status = process_callback(&gp_message);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(1, model_run_fake.call_count);
}

/**
 * Tests if process callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = process_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(0, model_run_fake.call_count);
}

/**
 * Tests if process callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_PROCESS_CALLBACK(_message_type)              \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = process_callback(&gp_message);               \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(0, model_run_fake.call_count);

    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_OK);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_STATS);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_IOSPEC);

#undef TEST_PROCESS_CALLBACK
}

// ========================================================
// output_callback
// ========================================================

/**
 * Tests if output callback properly loads model output
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback)
{
    status_t status = STATUS_OK;

    model_get_output_fake.return_val = STATUS_OK;
    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0, &gp_message);

    status = output_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_equal(1, model_get_output_fake.call_count);
}

/**
 * Tests if output callback fails when model output loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_model_error)
{
    status_t status = STATUS_OK;

    model_get_output_fake.return_val = MODEL_STATUS_ERROR;
    prepare_message(MESSAGE_TYPE_OUTPUT, NULL, 0, &gp_message);

    status = output_callback(&gp_message);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(1, model_get_output_fake.call_count);
}

/**
 * Tests if output callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = output_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(0, model_get_output_fake.call_count);
}

/**
 * Tests if output callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_OUTPUT_CALLBACK(_message_type)               \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = output_callback(&gp_message);                \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(0, model_get_output_fake.call_count);

    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_OK);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_STATS);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_IOSPEC);

#undef TEST_OUTPUT_CALLBACK
}

// ========================================================
// stats_callback
// ========================================================

/**
 * Tests if stats callback properly loads execution statistics
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback)
{
    status_t status = STATUS_OK;

    model_get_statistics_fake.return_val = STATUS_OK;
    prepare_message(MESSAGE_TYPE_STATS, NULL, 0, &gp_message);

    status = stats_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_equal(1, model_get_statistics_fake.call_count);
}

/**
 * Tests if stats callback fails if get statistics fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_model_error)
{
    status_t status = STATUS_OK;

    model_get_statistics_fake.return_val = MODEL_STATUS_ERROR;
    prepare_message(MESSAGE_TYPE_STATS, NULL, 0, &gp_message);

    status = stats_callback(&gp_message);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(1, model_get_statistics_fake.call_count);
}

/**
 * Tests if stats callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = stats_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(0, model_get_statistics_fake.call_count);
}

/**
 * Tests if stats callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_STATS_CALLBACK(_message_type)                \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = stats_callback(&gp_message);                 \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(0, model_get_statistics_fake.call_count);

    TEST_STATS_CALLBACK(MESSAGE_TYPE_OK);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_IOSPEC);

#undef TEST_STATS_CALLBACK
}

// ========================================================
// iospec_callback
// ========================================================

/**
 * Tests if IO spec callback load model struct
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    model_load_struct_fake.return_val = STATUS_OK;
    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data), &gp_message);

    status = iospec_callback(&gp_message);

    zassert_equal(STATUS_OK, status);
    zassert_equal(gp_message->payload, model_load_struct_fake.arg0_val);
    zassert_equal(MESSAGE_SIZE_PAYLOAD(gp_message->message_size), model_load_struct_fake.arg1_val);
    zassert_equal(1, model_load_struct_fake.call_count);
}

/**
 * Tests if IO spec callback fails when struct loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_model_error)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    model_load_struct_fake.return_val = MODEL_STATUS_ERROR;
    prepare_message(MESSAGE_TYPE_IOSPEC, data, sizeof(data), &gp_message);

    status = iospec_callback(&gp_message);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(1, model_load_struct_fake.call_count);
}

/**
 * Tests if IO spec callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = iospec_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(0, model_load_struct_fake.call_count);
}

/**
 * Tests if IO spec callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_invalid_message_type)
{
    status_t status = STATUS_OK;

#define TEST_IOSPEC_CALLBACK(_message_type)               \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = iospec_callback(&gp_message);                \
                                                          \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(0, model_load_struct_fake.call_count);

    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_OK);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_STATS);

#undef TEST_IOSPEC_CALLBACK
}

// ========================================================
// mocks
// ========================================================

const char *get_status_str_mock(status_t status) { return "STATUS_STR"; }

// ========================================================
// helper functions
// ========================================================

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message_t **msg)
{
    *msg = &g_message_buffer;
    (*msg)->message_size = sizeof(message_type_t) + payload_size;
    (*msg)->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy((*msg)->payload, payload, payload_size);
    }
}
