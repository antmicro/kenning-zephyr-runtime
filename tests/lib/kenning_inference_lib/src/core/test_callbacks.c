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
#include <kenning_inference_lib/core/loaders.h>
#include <kenning_inference_lib/core/model.h>
#include <kenning_inference_lib/core/utils.h>

#include <mocks/llext.h>

#include "utils.h"

GENERATE_MODULE_STATUSES_STR(MODEL);
GENERATE_MODULE_STATUSES_STR(KENNING_PROTOCOL);
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};

struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_LOADER_TYPES];

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                                                                        \
    MOCK(const char *, get_status_str, status_t)                                                           \
    MOCK(status_t, protocol_prepare_success_resp, resp_message_t *)                                        \
    MOCK(status_t, protocol_prepare_fail_resp, resp_message_t *)                                           \
    MOCK(status_t, model_load_struct_from_loader)                                                          \
    MOCK(status_t, model_load_weights_from_loader)                                                         \
    MOCK(status_t, model_load_input_from_loader)                                                           \
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
status_t model_get_output_mock(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size);

status_t model_get_statistics_mock(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                   size_t *statistics_size);

int llext_load_success_mock(struct llext_loader *ldr, const char *name, struct llext **p_llext,
                            struct llext_load_param *ldr_param);

// ========================================================
// helper functions declarations
// ========================================================

message_hdr_t prepare_message_header(message_type_t msg_type, size_t payload_size);

/**
 * Prepares message of given type and payload
 *
 * @param msg_type type of the message
 * @param payload payload of the message
 * @param payload_size size of the payload
 * @param msg prepared message
 */

struct llext *prepare_llext();

struct llext *prepare_llext_replacement();

// ========================================================
// setup
// ========================================================

static void callbacks_tests_setup_f()
{
    MOCKS(RESET_MOCK);

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
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_UNSUPPORTED_CALLBACK(_message_type)    \
    hdr = prepare_message_header(_message_type, 0); \
    status = unsupported_callback(&hdr, &resp);     \
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

// ========================================================
// ok_callback
// ========================================================

/**
 * Tests if ok callback has no response
 */
ZTEST(kenning_inference_lib_test_callbacks, test_ok_callback)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_OK, 0);
    resp_message_t resp;

    status = ok_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if ok callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_ok_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;
    status = ok_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
}

/**
 * Tests if ok callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_ok_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_OK_CALLBACK(_message_type)             \
    hdr = prepare_message_header(_message_type, 0); \
    status = ok_callback(&hdr, &resp);              \
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
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_ERROR, 0);
    resp_message_t resp;

    status = error_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if error callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_error_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;
    status = error_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
}

/**
 * Tests if error callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_error_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_ERROR_CALLBACK(_message_type)          \
    hdr = prepare_message_header(_message_type, 0); \
    status = error_callback(&hdr, &resp);           \
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
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_DATA, 0);
    resp_message_t resp;

    model_load_input_from_loader_fake.return_val = STATUS_OK;

    status = data_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_load_input_from_loader_fake.call_count, 1);
    zassert_equal(protocol_prepare_success_resp_fake.call_count, 1);
}
/**
 * Tests if data callback fails if model input loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_model_error)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_DATA, 0);
    resp_message_t resp;

    model_load_input_from_loader_fake.return_val = MODEL_STATUS_INV_STATE;

    status = data_callback(&hdr, &resp);

    zassert_equal(MODEL_STATUS_INV_STATE, status);
    zassert_equal(model_load_input_from_loader_fake.call_count, 1);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if data callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;

    status = data_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_load_input_from_loader_fake.call_count, 0);
}

/**
 * Tests if data callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_DATA_CALLBACK(_message_type)                 \
    hdr = prepare_message_header(_message_type, 0);       \
    status = data_callback(&hdr, &resp);                  \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_load_input_from_loader_fake.call_count, 0);

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
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_MODEL, 0);
    resp_message_t resp;

    model_load_weights_from_loader_fake.return_val = STATUS_OK;

    status = model_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_load_weights_from_loader_fake.call_count, 1);
    zassert_equal(protocol_prepare_success_resp_fake.call_count, 1);
}

/**
 * Tests if model callback fails if model weights loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_model_error)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_MODEL, 0);
    resp_message_t resp;

    model_load_weights_from_loader_fake.return_val = MODEL_STATUS_ERROR;

    status = model_callback(&hdr, &resp);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_load_weights_from_loader_fake.call_count, 1);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if model callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;

    status = model_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_load_weights_from_loader_fake.call_count, 0);
}

/**
 * Tests if model callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_MODEL_CALLBACK(_message_type)                \
    hdr = prepare_message_header(_message_type, 0);       \
    status = model_callback(&hdr, &resp);                 \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_load_weights_from_loader_fake.call_count, 0);

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
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_PROCESS, 0);
    resp_message_t resp;

    model_run_fake.return_val = STATUS_OK;

    status = process_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(protocol_prepare_success_resp_fake.call_count, 1);
    zassert_equal(model_run_fake.call_count, 1);
}

/**
 * Tests if process callback fails if model inference fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_model_error)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_PROCESS, 0);
    resp_message_t resp;

    model_run_fake.return_val = MODEL_STATUS_ERROR;

    status = process_callback(&hdr, &resp);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_run_fake.call_count, 1);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if process callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;

    status = process_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_run_fake.call_count, 0);
    zassert_equal(model_load_input_from_loader_fake.call_count, 0);
}

/**
 * Tests if process callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_PROCESS_CALLBACK(_message_type)              \
    hdr = prepare_message_header(_message_type, 0);       \
    status = process_callback(&hdr, &resp);               \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_run_fake.call_count, 0);

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
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_OUTPUT, 0);
    resp_message_t resp = {.payload = (uint8_t *)0x12345};

    model_get_output_fake.custom_fake = model_get_output_mock;

    status = output_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_get_output_fake.call_count, 1);
    zassert_equal(resp.hdr.message_size, 10 + sizeof(message_type_t));
    zassert_equal(model_get_output_fake.arg1_val, resp.payload);
    zassert_equal(resp.hdr.message_type, MESSAGE_TYPE_OK);
}

/**
 * Tests if output callback fails when model output loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_model_error)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_OUTPUT, 0);
    resp_message_t resp = {.payload = (uint8_t *)0x12345};

    model_get_output_fake.return_val = MODEL_STATUS_ERROR;

    status = output_callback(&hdr, &resp);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_get_output_fake.call_count, 1);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if output callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;

    status = output_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_get_output_fake.call_count, 0);
}

/**
 * Tests if output callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_OUTPUT_CALLBACK(_message_type)               \
    hdr = prepare_message_header(_message_type, 0);       \
    status = output_callback(&hdr, &resp);                \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_get_output_fake.call_count, 0);

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
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_STATS, 0);
    resp_message_t resp = {.hdr = {.message_size = 0}, .payload = (uint8_t *)0x12345};

    model_get_statistics_fake.custom_fake = model_get_statistics_mock;

    status = stats_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_get_statistics_fake.call_count, 1);
    zassert_equal(resp.hdr.message_size, 10 + sizeof(message_type_t));
    zassert_equal(model_get_statistics_fake.arg1_val, resp.payload);
    zassert_equal(resp.hdr.message_type, MESSAGE_TYPE_OK);
}

/**
 * Tests if stats callback fails if get statistics fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_model_error)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_STATS, 0);
    resp_message_t resp = {.payload = (uint8_t *)0x12345};

    model_get_statistics_fake.return_val = MODEL_STATUS_ERROR;

    status = stats_callback(&hdr, &resp);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_get_statistics_fake.call_count, 1);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if stats callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;

    status = stats_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_get_statistics_fake.call_count, 0);
}

/**
 * Tests if stats callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_STATS_CALLBACK(_message_type)                \
    hdr = prepare_message_header(_message_type, 0);       \
    status = stats_callback(&hdr, &resp);                 \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_get_statistics_fake.call_count, 0);

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
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_IOSPEC, 0);
    resp_message_t resp;

    model_load_struct_from_loader_fake.return_val = STATUS_OK;

    status = iospec_callback(&hdr, &resp);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_load_struct_from_loader_fake.call_count, 1);
    zassert_equal(protocol_prepare_success_resp_fake.call_count, 1);
}

/**
 * Tests if IO spec callback fails when struct loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_model_error)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_IOSPEC, 0);
    resp_message_t resp;

    model_load_struct_from_loader_fake.return_val = MODEL_STATUS_ERROR;

    status = iospec_callback(&hdr, &resp);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_load_struct_from_loader_fake.call_count, 1);
    zassert_equal(protocol_prepare_fail_resp_fake.call_count, 1);
}

/**
 * Tests if IO spec callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    resp_message_t resp;

    status = iospec_callback(NULL, &resp);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_load_struct_from_loader_fake.call_count, 0);
}

/**
 * Tests if IO spec callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr;
    resp_message_t resp;

#define TEST_IOSPEC_CALLBACK(_message_type)               \
    hdr = prepare_message_header(_message_type, 0);       \
    status = iospec_callback(&hdr, &resp);                \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_load_struct_from_loader_fake.call_count, 0);

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
 * Test if runtime_callback handles loading the first runtime properly
 */
ZTEST(kenning_inference_lib_test_callbacks, test_runtime_callback)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_RUNTIME, 1234);
    resp_message_t resp;

    llext_by_name_fake.return_val = NULL;

    llext_load_fake.custom_fake = llext_load_success_mock;
    llext_bringup_fake.return_val = STATUS_OK;
    llext_unload_fake.return_val = STATUS_OK;
    llext_teardown_fake.return_val = STATUS_OK;

    model_init_fake.return_val = STATUS_OK;
    runtime_deinit_fake.return_val = STATUS_OK;

    status = runtime_callback(&hdr, &resp);

    zassert_equal(1, llext_load_fake.call_count);
    zassert_equal(1, llext_bringup_fake.call_count);
    zassert_equal(0, llext_unload_fake.call_count);
    zassert_equal(0, llext_teardown_fake.call_count);
    zassert_equal(protocol_prepare_success_resp_fake.call_count, 1);
    zassert_equal(prepare_llext(), llext_bringup_fake.arg0_val);

    zassert_equal(STATUS_OK, status);
}

/**
 * Test if runtime_callback fails when the runtime is already loaded
 */
ZTEST(kenning_inference_lib_test_callbacks, test_runtime_callback_fail)
{
    status_t status = STATUS_OK;
    message_hdr_t hdr = prepare_message_header(MESSAGE_TYPE_RUNTIME, 1234);
    resp_message_t resp;

    llext_by_name_fake.return_val = (struct llext *)0x123456;

    llext_load_fake.custom_fake = 0;
    llext_bringup_fake.return_val = STATUS_OK;
    llext_unload_fake.return_val = STATUS_OK;
    llext_teardown_fake.return_val = STATUS_OK;

    model_init_fake.return_val = STATUS_OK;
    runtime_deinit_fake.return_val = STATUS_OK;

    status = runtime_callback(&hdr, &resp);

    zassert_equal(0, llext_load_fake.call_count);
    zassert_equal(0, llext_bringup_fake.call_count);
    zassert_equal(0, llext_unload_fake.call_count);
    zassert_equal(0, llext_teardown_fake.call_count);

    zassert_equal(CALLBACKS_STATUS_ERROR, status);
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

struct llext *prepare_llext_replacement()
{
    static struct llext_symbol syms[2] = {
        (struct llext_symbol){.name = "sym3", .addr = NULL},
    };

    static struct llext p_llext = {.sym_tab =
                                       {
                                           .sym_cnt = 0,
                                           .syms = NULL,
                                       },
                                   .exp_tab = {
                                       .sym_cnt = 1,
                                       .syms = syms,
                                   }};

    return &p_llext;
}

// ========================================================
// mocks
// ========================================================
status_t model_get_output_mock(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size)
{
    *model_output_size = 10;
    return STATUS_OK;
}

status_t model_get_statistics_mock(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                   size_t *statistics_size)
{
    *statistics_size = 10;
    return STATUS_OK;
}

int llext_load_success_mock(struct llext_loader *ldr, const char *name, struct llext **p_llext,
                            struct llext_load_param *ldr_param)
{
    *p_llext = prepare_llext();
    return 0;
}