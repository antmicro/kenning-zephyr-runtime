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

#define MODEL_OUTPUT_SIZE 10
#define STATISTICS_SIZE 10

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                                                                        \
    MOCK(const char *, get_status_str, status_t)                                                           \
    MOCK(status_t, model_load_struct_from_loader)                                                          \
    MOCK(status_t, model_load_weights_from_loader)                                                         \
    MOCK(status_t, model_load_input_from_loader, const size_t)                                             \
    MOCK(status_t, model_run)                                                                              \
    MOCK(status_t, model_run_bench)                                                                        \
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
status_t model_get_output_mock(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size);

status_t model_get_statistics_mock(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                   size_t *statistics_size);

int llext_load_success_mock(struct llext_loader *ldr, const char *name, struct llext **p_llext,
                            struct llext_load_param *ldr_param);

// ========================================================
// helper functions declarations
// ========================================================

protocol_event_t prepare_request(message_type_t msg_type, size_t payload_size);

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

    static struct msg_loader msg_loader_llext = {0};
    g_ldr_tables[0][LOADER_TYPE_RUNTIME] = &msg_loader_llext;
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
    protocol_event_t request;
    protocol_payload_t resp_payload;

#define TEST_UNSUPPORTED_CALLBACK(_message_type)            \
    request = prepare_request(_message_type, 0);            \
    status = unsupported_callback(&request, &resp_payload); \
    zassert_equal(STATUS_OK, status);

    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_PING);
    TEST_UNSUPPORTED_CALLBACK(MESSAGE_TYPE_STATUS);
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
// data_callback
// ========================================================

/**
 * Tests if data callback properly loads model input
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback)
{
    status_t status = STATUS_OK;

    protocol_event_t request = prepare_request(MESSAGE_TYPE_DATA, 0);
    protocol_payload_t resp_payload;

    model_load_input_from_loader_fake.return_val = STATUS_OK;

    status = data_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_load_input_from_loader_fake.call_count, 1);
}
/**
 * Tests if data callback fails if model input loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_model_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_DATA, 0);
    protocol_payload_t resp_payload;

    model_load_input_from_loader_fake.return_val = MODEL_STATUS_INV_STATE;

    status = data_callback(&request, &resp_payload);

    zassert_equal(MODEL_STATUS_INV_STATE, status);
    zassert_equal(model_load_input_from_loader_fake.call_count, 1);
}

/**
 * Tests if data callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    protocol_payload_t resp_payload;

    status = data_callback(NULL, &resp_payload);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_load_input_from_loader_fake.call_count, 0);
}

/**
 * Tests if data callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_data_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_DATA, 0);
    protocol_payload_t resp_payload;

#define TEST_DATA_CALLBACK(_message_type)                 \
    request = prepare_request(_message_type, 0);          \
    status = data_callback(&request, &resp_payload);      \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_load_input_from_loader_fake.call_count, 0);

    TEST_DATA_CALLBACK(MESSAGE_TYPE_PING);
    TEST_DATA_CALLBACK(MESSAGE_TYPE_STATUS);
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
    protocol_event_t request = prepare_request(MESSAGE_TYPE_MODEL, 0);
    protocol_payload_t resp_payload;

    model_load_weights_from_loader_fake.return_val = STATUS_OK;

    status = model_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_load_weights_from_loader_fake.call_count, 1);
}

/**
 * Tests if model callback fails if model weights loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_model_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_MODEL, 0);
    protocol_payload_t resp_payload;

    model_load_weights_from_loader_fake.return_val = MODEL_STATUS_ERROR;

    status = model_callback(&request, &resp_payload);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_load_weights_from_loader_fake.call_count, 1);
}

/**
 * Tests if model callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    protocol_payload_t resp_payload;
    status = model_callback(NULL, &resp_payload);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_load_weights_from_loader_fake.call_count, 0);
}

/**
 * Tests if model callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_model_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_MODEL, 0);
    protocol_payload_t resp_payload;

#define TEST_MODEL_CALLBACK(_message_type)                \
    request = prepare_request(_message_type, 0);          \
    status = model_callback(&request, &resp_payload);     \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_load_weights_from_loader_fake.call_count, 0);

    TEST_MODEL_CALLBACK(MESSAGE_TYPE_PING);
    TEST_MODEL_CALLBACK(MESSAGE_TYPE_STATUS);
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
    protocol_event_t request = prepare_request(MESSAGE_TYPE_PROCESS, 0);
    protocol_payload_t resp_payload;

    model_run_bench_fake.return_val = STATUS_OK;

    status = process_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);

    zassert_equal(model_run_bench_fake.call_count, 1);
}

/**
 * Tests if process callback fails if model inference fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_model_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_PROCESS, 0);
    protocol_payload_t resp_payload;

    model_run_bench_fake.return_val = MODEL_STATUS_ERROR;

    status = process_callback(&request, &resp_payload);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_run_bench_fake.call_count, 1);
}

/**
 * Tests if process callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    protocol_payload_t resp_payload;

    status = process_callback(NULL, &resp_payload);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_run_bench_fake.call_count, 0);
}

/**
 * Tests if process callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_process_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_PROCESS, 0);
    protocol_payload_t resp_payload;

#define TEST_PROCESS_CALLBACK(_message_type)              \
    request = prepare_request(_message_type, 0);          \
    status = process_callback(&request, &resp_payload);   \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_run_bench_fake.call_count, 0);

    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_PING);
    TEST_PROCESS_CALLBACK(MESSAGE_TYPE_STATUS);
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
    protocol_event_t request = prepare_request(MESSAGE_TYPE_OUTPUT, 0);
    protocol_payload_t resp_payload;

    model_get_output_fake.custom_fake = model_get_output_mock;

    status = output_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_get_output_fake.call_count, 1);
    zassert_equal(resp_payload.size, MODEL_OUTPUT_SIZE);
    zassert_equal(model_get_output_fake.arg1_val, resp_payload.raw_bytes);
}

/**
 * Tests if output callback fails when model output loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_model_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_OUTPUT, 0);
    protocol_payload_t resp_payload = {.raw_bytes = (uint8_t *)0x12345};

    model_get_output_fake.return_val = MODEL_STATUS_ERROR;

    status = output_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_get_output_fake.call_count, 1);
}

/**
 * Tests if output callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    protocol_payload_t resp_payload;

    status = output_callback(NULL, &resp_payload);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_get_output_fake.call_count, 0);
}

/**
 * Tests if output callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_output_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    protocol_event_t request;
    protocol_payload_t resp_payload;

#define TEST_OUTPUT_CALLBACK(_message_type)               \
    request = prepare_request(_message_type, 0);          \
    status = output_callback(&request, &resp_payload);    \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_get_output_fake.call_count, 0);

    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_PING);
    TEST_OUTPUT_CALLBACK(MESSAGE_TYPE_STATUS);
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
    protocol_event_t request = prepare_request(MESSAGE_TYPE_STATS, 0);
    protocol_payload_t resp_payload = {.size = 0, .raw_bytes = (uint8_t *)0x12345};

    model_get_statistics_fake.custom_fake = model_get_statistics_mock;

    status = stats_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_get_statistics_fake.call_count, 1);
    zassert_equal(resp_payload.size, STATISTICS_SIZE);
    zassert_equal(model_get_statistics_fake.arg1_val, resp_payload.raw_bytes);
}

/**
 * Tests if stats callback fails if get statistics fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_model_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_STATS, 0);
    protocol_payload_t resp_payload = {.raw_bytes = (uint8_t *)0x12345};

    model_get_statistics_fake.return_val = MODEL_STATUS_ERROR;

    status = stats_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_get_statistics_fake.call_count, 1);
}

/**
 * Tests if stats callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    protocol_payload_t resp_payload;

    status = stats_callback(NULL, &resp_payload);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_get_statistics_fake.call_count, 0);
}

/**
 * Tests if stats callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_stats_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_STATS, 0);
    protocol_payload_t resp_payload;

#define TEST_STATS_CALLBACK(_message_type)                \
    request = prepare_request(_message_type, 0);          \
    status = stats_callback(&request, &resp_payload);     \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_get_statistics_fake.call_count, 0);

    TEST_STATS_CALLBACK(MESSAGE_TYPE_PING);
    TEST_STATS_CALLBACK(MESSAGE_TYPE_STATUS);
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
    protocol_event_t request = prepare_request(MESSAGE_TYPE_IOSPEC, 0);
    protocol_payload_t resp_payload;

    model_load_struct_from_loader_fake.return_val = STATUS_OK;

    status = iospec_callback(&request, &resp_payload);

    zassert_equal(STATUS_OK, status);
    zassert_equal(model_load_struct_from_loader_fake.call_count, 1);
}

/**
 * Tests if IO spec callback fails when struct loading fails
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_model_error)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_IOSPEC, 0);
    protocol_payload_t resp_payload;

    model_load_struct_from_loader_fake.return_val = MODEL_STATUS_ERROR;

    status = iospec_callback(&request, &resp_payload);

    zassert_equal(MODEL_STATUS_ERROR, status);
    zassert_equal(model_load_struct_from_loader_fake.call_count, 1);
}

/**
 * Tests if IO spec callback fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_invalid_pointer)
{
    status_t status = STATUS_OK;
    protocol_payload_t resp_payload;

    status = iospec_callback(NULL, &resp_payload);

    zassert_equal(CALLBACKS_STATUS_INV_PTR, status);
    zassert_equal(model_load_struct_from_loader_fake.call_count, 0);
}

/**
 * Tests if IO spec callback fails for invalid request message type
 */
ZTEST(kenning_inference_lib_test_callbacks, test_iospec_callback_invalid_message_type)
{
    status_t status = STATUS_OK;
    protocol_event_t request;
    protocol_payload_t resp_payload;

#define TEST_IOSPEC_CALLBACK(_message_type)               \
    request = prepare_request(_message_type, 0);          \
    status = iospec_callback(&request, &resp_payload);    \
    zassert_equal(CALLBACKS_STATUS_INV_MSG_TYPE, status); \
    zassert_equal(model_load_struct_from_loader_fake.call_count, 0);

    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_PING);
    TEST_IOSPEC_CALLBACK(MESSAGE_TYPE_STATUS);
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
    protocol_event_t request = prepare_request(MESSAGE_TYPE_RUNTIME, 1234);
    protocol_payload_t resp_payload;

    llext_by_name_fake.return_val = NULL;

    llext_load_fake.custom_fake = llext_load_success_mock;
    llext_bringup_fake.return_val = STATUS_OK;
    llext_unload_fake.return_val = STATUS_OK;
    llext_teardown_fake.return_val = STATUS_OK;

    model_init_fake.return_val = STATUS_OK;
    runtime_deinit_fake.return_val = STATUS_OK;

    status = runtime_callback(&request, &resp_payload);

    zassert_equal(1, llext_load_fake.call_count);
    zassert_equal(1, llext_bringup_fake.call_count);
    zassert_equal(0, llext_unload_fake.call_count);
    zassert_equal(0, llext_teardown_fake.call_count);
    zassert_equal(prepare_llext(), llext_bringup_fake.arg0_val);

    zassert_equal(STATUS_OK, status);
}

/**
 * Test if runtime_callback fails when the runtime is already loaded
 */
ZTEST(kenning_inference_lib_test_callbacks, test_runtime_callback_fail)
{
    status_t status = STATUS_OK;
    protocol_event_t request = prepare_request(MESSAGE_TYPE_RUNTIME, 1234);
    protocol_payload_t resp_payload;

    llext_by_name_fake.return_val = (struct llext *)0x123456;

    llext_load_fake.custom_fake = 0;
    llext_bringup_fake.return_val = STATUS_OK;
    llext_unload_fake.return_val = STATUS_OK;
    llext_teardown_fake.return_val = STATUS_OK;

    model_init_fake.return_val = STATUS_OK;
    runtime_deinit_fake.return_val = STATUS_OK;

    status = runtime_callback(&request, &resp_payload);

    zassert_equal(0, llext_load_fake.call_count);
    zassert_equal(0, llext_bringup_fake.call_count);
    zassert_equal(0, llext_unload_fake.call_count);
    zassert_equal(0, llext_teardown_fake.call_count);
    zassert_equal(CALLBACKS_STATUS_ERROR, status);
}

// ========================================================
// helper functions
// ========================================================

protocol_event_t prepare_request(message_type_t msg_type, size_t payload_size)
{
    protocol_event_t event;
    event.payload.size = payload_size;
    event.message_type = msg_type;
    return event;
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
    *model_output_size = MODEL_OUTPUT_SIZE;
    return STATUS_OK;
}

status_t model_get_statistics_mock(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                   size_t *statistics_size)
{
    *statistics_size = STATISTICS_SIZE;
    return STATUS_OK;
}

int llext_load_success_mock(struct llext_loader *ldr, const char *name, struct llext **p_llext,
                            struct llext_load_param *ldr_param)
{
    *p_llext = prepare_llext();
    return 0;
}
