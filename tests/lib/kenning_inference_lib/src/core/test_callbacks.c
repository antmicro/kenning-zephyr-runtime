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

#include <mocks/llext.h>

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

#define MOCKS(MOCK)                                                                                        \
    MOCK(const char *, get_status_str, status_t)                                                           \
    MOCK(status_t, protocol_prepare_success_resp, message_t **)                                            \
    MOCK(status_t, protocol_prepare_fail_resp, message_t **)                                               \
    MOCK(status_t, model_load_struct, const uint8_t *, size_t)                                             \
    MOCK(status_t, model_load_weights, const uint8_t *, size_t)                                            \
    MOCK(status_t, model_load_input, const uint8_t *, size_t)                                              \
    MOCK(status_t, model_run)                                                                              \
    MOCK(status_t, model_get_output, const size_t, uint8_t *, size_t *)                                    \
    MOCK(status_t, model_get_statistics, const size_t, uint8_t *, size_t *)                                \
    MOCK(status_t, runtime_deinit)                                                                         \
    MOCK(status_t, model_init)                                                                             \
    MOCK(struct llext *, llext_by_name, char *)                                                            \
    MOCK(int, llext_unload, struct llext **)                                                               \
    MOCK(int, llext_load, struct llext_loader *, const char *, struct llext **, struct llext_load_param *) \
    MOCK(int, llext_bringup, struct llext *)                                                               \
    MOCK(int, llext_teardown, struct llext *)
MOCKS(DECLARE_MOCK);

const char *get_status_str_mock(status_t);
int llext_load_success_mock(struct llext_loader *, const char *, struct llext **, struct llext_load_param *);

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

struct llext *prepare_llext();

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
// unsupported_callback
// ========================================================

/**
 * Tests if unsupported callback response is fail
 */
ZTEST(kenning_inference_lib_test_callbacks, test_unsupported_callback)
{
    status_t status = STATUS_OK;

#define TEST_UNSUPPORTED_CALLBACK(_message_type)          \
    prepare_message(_message_type, NULL, 0, &gp_message); \
                                                          \
    status = unsupported_callback(&gp_message);           \
                                                          \
    zassert_equal(STATUS_OK, status);

    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_OK);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_ERROR);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_DATA);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_MODEL);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_PROCESS);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_OUTPUT);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_STATS);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_IOSPEC);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_RUNTIME);

#undef TEST_UNSUPPORTED_CALLBACK
}

/**
 * Tests if ok callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_unsupported_callback_invalid_pointer)
{
    status_t status = STATUS_OK;

    status = unsupported_callback(NULL);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
}

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
    TEST_OK_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_OK_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_OK_CALLBACK(MESSAGE_TYPE_RUNTIME);

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
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_ERROR_CALLBACK(MESSAGE_TYPE_RUNTIME);

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
 * Tests if data callback fails for invalid request message type
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
    TEST_DATA_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_RUNTIME);

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
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_RUNTIME);

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
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_RUNTIME);

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
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_RUNTIME);

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
    TEST_STATS_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_RUNTIME);

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
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_OPTIMIZERS);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_OPTIMIZE_MODEL);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_RUNTIME);

#undef TEST_IOSPEC_CALLBACK
}

// ========================================================
// runtime_callback
// ========================================================

/**
 *
 */
ZTEST(kenning_inference_lib_test_callbacks, test_runtime_callback)
{
    status_t status = STATUS_OK;
    uint8_t payload[] = "abcdefg";

    struct llext *p_llext = NULL;

    llext_by_name_fake.return_val = p_llext;

    llext_load_fake.custom_fake = llext_load_success_mock;
    llext_bringup_fake.return_val = STATUS_OK;
    llext_unload_fake.return_val = STATUS_OK;
    llext_teardown_fake.return_val = STATUS_OK;

    model_init_fake.return_val = STATUS_OK;
    runtime_deinit_fake.return_val = STATUS_OK;

    prepare_message(MESSAGE_TYPE_RUNTIME, payload, sizeof(payload), &gp_message);

    status = runtime_callback(&gp_message);

    zassert_equal(1, llext_load_fake.call_count);
    zassert_equal(1, llext_bringup_fake.call_count);
    zassert_equal(0, llext_unload_fake.call_count);
    zassert_equal(0, llext_teardown_fake.call_count);

    zassert_equal(STATUS_OK, status);
}

// ========================================================
// mocks
// ========================================================

const char *get_status_str_mock(status_t status) { return "STATUS_STR"; }
int llext_load_success_mock(struct llext_loader *ldr, const char *name, struct llext **p_llext,
                            struct llext_load_param *ldr_param)
{
    *p_llext = prepare_llext();
    return 0;
}

// ========================================================
// helper functions
// ========================================================

void prepare_message(message_type_t msg_type, uint8_t *payload, size_t payload_size, message_t **msg)
{
    *msg = (message_t *)&g_message_buffer;
    (*msg)->message_size = sizeof(message_type_t) + payload_size;
    (*msg)->message_type = msg_type;
    if (payload_size > 0)
    {
        memcpy((*msg)->payload, payload, payload_size);
    }
}

struct llext *prepare_llext()
{
    static struct llext_symbol syms[2] = {
        (struct llext_symbol){.name = "sym1", .addr = NULL},
        (struct llext_symbol){.name = "sym2", .addr = NULL},
    };

    static struct llext p_llext = {.sym_tab =
                                       {
                                           .sym_cnt = 0,
                                           .syms = NULL,
                                       },
                                   .exp_tab = {
                                       .sym_cnt = 2,
                                       .syms = syms,
                                   }};

    return &p_llext;
}