/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include <kenning_inference_lib/core/loaders.h>
#include <kenning_inference_lib/core/model.h>
#include <kenning_inference_lib/core/runtime_wrapper.h>

#include "utils.h"

#define MODEL_SPEC_INPUT_LEN (28 * 28)
#define MODEL_SPEC_INPUT_SIZE (4)
#define MODEL_SPEC_OUTPUT_LEN (10)
#define MODEL_SPEC_OUTPUT_SIZE (4)
#define MODEL_SPEC_INPUT_NUM (1)
#define MODEL_SPEC_OUTPUT_NUM (1)
#define STRUCT_VALID_SHAPE_VALUE (1)
#define MODEL_SIZE (8)
#define MODEL_INPUT_SIZE (MODEL_SPEC_INPUT_LEN * MODEL_SPEC_INPUT_SIZE)

extern model_spec_t g_model_spec;
extern MODEL_STATE g_model_state;

const data_type_t MODEL_SPEC_INPUT_DATA_TYPE = {DATA_TYPE_INT, MODEL_SPEC_INPUT_SIZE * 8};
const data_type_t MODEL_SPEC_OUTPUT_DATA_TYPE = {DATA_TYPE_INT, MODEL_SPEC_OUTPUT_SIZE * 8};

static uint8_t __attribute__((aligned(8))) gp_modelBuffer[MODEL_SIZE * 1024];
static uint8_t __attribute__((aligned(8))) gp_inputBuffer[MODEL_INPUT_SIZE * 1024];

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define MOCKS(MOCK)                                                           \
    MOCK(status_t, runtime_init)                                              \
    MOCK(status_t, runtime_init_weights)                                      \
    MOCK(status_t, runtime_init_input)                                        \
    MOCK(status_t, runtime_run_model)                                         \
    MOCK(status_t, runtime_run_model_bench)                                   \
    MOCK(status_t, runtime_get_model_output, uint8_t *)                       \
    MOCK(status_t, runtime_get_statistics, const size_t, uint8_t *, size_t *) \
    MOCK(uint32_t, model_spec_input_length, const model_spec_t *, uint32_t)   \
    MOCK(uint32_t, model_spec_output_length, const model_spec_t *, uint32_t)

MOCKS(DECLARE_MOCK);

/**
 * This custom mock is required for testing the following model functions:
 *
 * model_load_struct_from_loader, model_get_input_size, model_load_struct, model_load_input_from_loader,
 * model_load_input
 *
 * In all tests, where these functions are used (or functions, that use these functions, are used) use the following
 * line:
 *
 *  model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
 *
 */
uint32_t model_spec_input_length_mock(const model_spec_t *model_iospec, uint32_t index);

/**
 * This custom mock is required for testing the following model functions:
 *
 * model_load_struct_from_loader, model_get_output_size, model_load_struct, model_get_output
 *
 * In all tests, where these functions are used (or functions, that use these functions, are used) use the following
 * line:
 *
 * model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;
 *
 */
uint32_t model_spec_output_length_mock(const model_spec_t *model_iospec, uint32_t index);

// ========================================================
// helper functions declarations
// ========================================================

/**
 * Returns example model struct data with passed dtype.
 *
 * @param dtype dtype of model input
 *
 * @returns prepared model struct
 */
static model_spec_t get_model_spec_data();

// ========================================================
// setup
// ========================================================

static void model_tests_setup_f()
{
    MOCKS(RESET_MOCK);

    g_model_spec = get_model_spec_data();
    g_model_state = MODEL_STATE_UNINITIALIZED;

    static struct msg_loader msg_loader_model = MSG_LOADER_BUF(gp_modelBuffer, MODEL_SIZE * 1024);
    static struct msg_loader msg_loader_input = MSG_LOADER_BUF(gp_inputBuffer, MODEL_INPUT_SIZE * 1024);
    g_ldr_tables[1][LOADER_TYPE_MODEL] = &msg_loader_model;
    g_ldr_tables[1][LOADER_TYPE_DATA] = &msg_loader_input;
}

ZTEST_SUITE(kenning_inference_lib_test_model, NULL, NULL, model_tests_setup_f, NULL, NULL);

// ========================================================
// model_get_state
// ========================================================

/**
 * Tests if model state getter returns proper model state
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_state)
{
#define TEST_GET_STATE(_model_state) \
    g_model_state = (_model_state);  \
    zassert_equal((_model_state), model_get_state(), "invalid model state returned");

    TEST_GET_STATE(MODEL_STATE_UNINITIALIZED);
    TEST_GET_STATE(MODEL_STATE_STRUCT_LOADED);
    TEST_GET_STATE(MODEL_STATE_WEIGHTS_LOADED);
    TEST_GET_STATE(MODEL_STATE_INPUT_LOADED);
    TEST_GET_STATE(MODEL_STATE_INFERENCE_DONE);
#undef TEST_GET_STATE
}

// ========================================================
// model_reset_state
// ========================================================

/**
 * Tests if model state reset sets model state to uninitialized
 */
ZTEST(kenning_inference_lib_test_model, test_model_reset_state)
{
#define TEST_RESET_STATE(_model_state) \
    g_model_state = (_model_state);    \
    model_reset_state();               \
    zassert_equal(MODEL_STATE_UNINITIALIZED, g_model_state, "invalid model state returned");

    TEST_RESET_STATE(MODEL_STATE_UNINITIALIZED);
    TEST_RESET_STATE(MODEL_STATE_STRUCT_LOADED);
    TEST_RESET_STATE(MODEL_STATE_WEIGHTS_LOADED);
    TEST_RESET_STATE(MODEL_STATE_INPUT_LOADED);
    TEST_RESET_STATE(MODEL_STATE_INFERENCE_DONE);
#undef TEST_REST_STATE
}

// ========================================================
// model_init
// ========================================================

/**
 * Tests model initialization
 */
ZTEST(kenning_inference_lib_test_model, test_model_init)
{
    status_t status = STATUS_OK;

    runtime_init_fake.return_val = STATUS_OK;

    status = model_init();

    zassert_equal(STATUS_OK, status);
    zassert_equal(MODEL_STATE_INITIALIZED, g_model_state);
}

/**
 * Tests model initialization when runtime init fails
 */
ZTEST(kenning_inference_lib_test_model, test_model_init_fail)
{
    status_t status = STATUS_OK;

    runtime_init_fake.return_val = RUNTIME_WRAPPER_STATUS_ERROR;

    status = model_init();

    zassert_equal(RUNTIME_WRAPPER_STATUS_ERROR, status);
    zassert_equal(MODEL_STATE_UNINITIALIZED, g_model_state);
}

// ========================================================
// model_load_struct
// ========================================================

/**
 * Tests model struct loading in valid model states
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_valid_state)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_model_state)                                        \
    g_model_state = (_model_state);                                           \
    model_spec = get_model_spec_data();                                       \
                                                                              \
    status = model_load_struct((uint8_t *)&model_spec, sizeof(model_spec_t)); \
                                                                              \
    zassert_equal(STATUS_OK, status);                                         \
    zassert_equal(g_model_state, MODEL_STATE_STRUCT_LOADED);

    TEST_LOAD_STRUCT(MODEL_STATE_INITIALIZED);
    TEST_LOAD_STRUCT(MODEL_STATE_STRUCT_LOADED);
    TEST_LOAD_STRUCT(MODEL_STATE_WEIGHTS_LOADED);
    TEST_LOAD_STRUCT(MODEL_STATE_INPUT_LOADED);
    TEST_LOAD_STRUCT(MODEL_STATE_INFERENCE_DONE);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for valid input_num values
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_num_input)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_num_input)                                          \
    g_model_state = MODEL_STATE_INITIALIZED;                                  \
    model_spec = get_model_spec_data();                                       \
    model_spec.num_input = (_num_input);                                      \
    for (int i = 0; i < (_num_input); ++i)                                    \
    {                                                                         \
        model_spec.num_input_dim[i] = 1;                                      \
        model_spec.input_shape[i][0] = STRUCT_VALID_SHAPE_VALUE;              \
    }                                                                         \
                                                                              \
    status = model_load_struct((uint8_t *)&model_spec, sizeof(model_spec_t)); \
                                                                              \
    zassert_equal(STATUS_OK, status);                                         \
    zassert_equal((_num_input), g_model_spec.num_input);                      \
    zassert_equal(MODEL_STATE_STRUCT_LOADED, g_model_state);

    TEST_LOAD_STRUCT(1);
    TEST_LOAD_STRUCT(MAX_MODEL_INPUT_NUM);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for invalid input_num values
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_invalid_num_input)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_num_input)                                          \
    g_model_state = MODEL_STATE_INITIALIZED;                                  \
    model_spec = get_model_spec_data();                                       \
    model_spec.num_input = (_num_input);                                      \
                                                                              \
    status = model_load_struct((uint8_t *)&model_spec, sizeof(model_spec_t)); \
                                                                              \
    zassert_equal(MODEL_STATUS_INV_ARG, status);                              \
    zassert_equal(MODEL_STATE_INITIALIZED, g_model_state);

    TEST_LOAD_STRUCT(0);
    TEST_LOAD_STRUCT(-1);
    TEST_LOAD_STRUCT(MAX_MODEL_INPUT_NUM + 1);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for valid num_output values
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_num_output)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_num_output)                                         \
    g_model_state = MODEL_STATE_INITIALIZED;                                  \
    model_spec = get_model_spec_data();                                       \
    model_spec.num_output = (_num_output);                                    \
    for (int i = 0; i < (_num_output); ++i)                                   \
    {                                                                         \
        model_spec.num_output_dim[i] = 2;                                     \
        model_spec.output_shape[i][0] = STRUCT_VALID_SHAPE_VALUE;             \
        model_spec.output_shape[i][1] = STRUCT_VALID_SHAPE_VALUE;             \
        model_spec.output_data_type[i] = MODEL_SPEC_OUTPUT_DATA_TYPE;         \
    }                                                                         \
                                                                              \
    status = model_load_struct((uint8_t *)&model_spec, sizeof(model_spec_t)); \
                                                                              \
    zassert_equal(STATUS_OK, status);                                         \
    zassert_equal((_num_output), g_model_spec.num_output);                    \
    zassert_equal(MODEL_STATE_STRUCT_LOADED, g_model_state);

    TEST_LOAD_STRUCT(1);
    TEST_LOAD_STRUCT(MAX_MODEL_OUTPUT_NUM);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for invalid num_output values
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_invalid_num_output)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_num_output)                                         \
    g_model_state = MODEL_STATE_INITIALIZED;                                  \
    model_spec = get_model_spec_data();                                       \
    model_spec.num_output = (_num_output);                                    \
                                                                              \
    status = model_load_struct((uint8_t *)&model_spec, sizeof(model_spec_t)); \
                                                                              \
    zassert_equal(MODEL_STATUS_INV_ARG, status);                              \
    zassert_equal(MODEL_STATE_INITIALIZED, g_model_state);

    TEST_LOAD_STRUCT(0);
    TEST_LOAD_STRUCT(-1);
    TEST_LOAD_STRUCT(MAX_MODEL_OUTPUT_NUM + 1);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for valid input_dim values
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_num_input_dim)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_num_input_dim)                                      \
    g_model_state = MODEL_STATE_INITIALIZED;                                  \
    model_spec = get_model_spec_data();                                       \
    model_spec.num_input_dim[0] = (_num_input_dim);                           \
                                                                              \
    status = model_load_struct((uint8_t *)&model_spec, sizeof(model_spec_t)); \
                                                                              \
    zassert_equal(STATUS_OK, status);                                         \
    zassert_equal((_num_input_dim), g_model_spec.num_input_dim[0]);           \
    zassert_equal(MODEL_STATE_STRUCT_LOADED, g_model_state);

    TEST_LOAD_STRUCT(1);
    TEST_LOAD_STRUCT(MAX_MODEL_INPUT_DIM);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for invalid input_dim values
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_invalid_num_input_dim)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_num_input_dim)                                      \
    g_model_state = MODEL_STATE_INITIALIZED;                                  \
    model_spec = get_model_spec_data();                                       \
    model_spec.num_input_dim[0] = (_num_input_dim);                           \
                                                                              \
    status = model_load_struct((uint8_t *)&model_spec, sizeof(model_spec_t)); \
                                                                              \
    zassert_equal(MODEL_STATUS_INV_ARG, status);                              \
    zassert_equal(MODEL_STATE_INITIALIZED, g_model_state);

    TEST_LOAD_STRUCT(0);
    TEST_LOAD_STRUCT(-1);
    TEST_LOAD_STRUCT(MAX_MODEL_INPUT_DIM + 1);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for data with invalid size
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_invalid_size)
{
    status_t status = STATUS_OK;
    model_spec_t model_spec = get_model_spec_data();

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_LOAD_STRUCT(_struct_size, _expected_error)                 \
    g_model_state = MODEL_STATE_INITIALIZED;                            \
                                                                        \
    status = model_load_struct((uint8_t *)&model_spec, (_struct_size)); \
                                                                        \
    zassert_equal(_expected_error, status);                             \
    zassert_equal(MODEL_STATE_INITIALIZED, g_model_state);

    TEST_LOAD_STRUCT(sizeof(model_spec_t) - 1, MODEL_STATUS_INV_ARG);
    TEST_LOAD_STRUCT(sizeof(model_spec_t) + 1, LOADERS_STATUS_NOT_ENOUGH_MEMORY);
    TEST_LOAD_STRUCT(0, MODEL_STATUS_INV_ARG);

#undef TEST_LOAD_STRUCT
}

/**
 * Tests model struct parsing for invalid pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_struct_invalid_ptr)
{
    status_t status = STATUS_OK;

    g_model_state = MODEL_STATE_INITIALIZED;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;
    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

    status = model_load_struct(NULL, sizeof(model_spec_t));

    zassert_equal(MODEL_STATUS_INV_PTR, status);
    zassert_equal(MODEL_STATE_INITIALIZED, g_model_state);
}

// ========================================================
// model_load_weights
// ========================================================

/**
 * Tests model weights loading in valid model states
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_weights_valid_state)
{
    status_t status = STATUS_OK;
    uint8_t model_weights[128] = {0};

#define TEST_LOAD_WEIGHTS(_model_state)                                \
    g_model_state = (_model_state);                                    \
                                                                       \
    status = model_load_weights(model_weights, sizeof(model_weights)); \
                                                                       \
    zassert_equal(STATUS_OK, status);                                  \
    zassert_equal(g_model_state, MODEL_STATE_WEIGHTS_LOADED);

    TEST_LOAD_WEIGHTS(MODEL_STATE_STRUCT_LOADED);
    TEST_LOAD_WEIGHTS(MODEL_STATE_WEIGHTS_LOADED);
    TEST_LOAD_WEIGHTS(MODEL_STATE_INPUT_LOADED);
    TEST_LOAD_WEIGHTS(MODEL_STATE_INFERENCE_DONE);

#undef TEST_LOAD_WEIGHTS
}

/**
 * Tests model weights loading when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_weights_invalid_state)
{
    status_t status = STATUS_OK;
    uint8_t model_weights[128] = {0};

#define TEST_LOAD_WEIGHTS(_model_state)                                \
    g_model_state = _model_state;                                      \
                                                                       \
    status = model_load_weights(model_weights, sizeof(model_weights)); \
                                                                       \
    zassert_equal(MODEL_STATUS_INV_STATE, status);                     \
    zassert_equal(_model_state, g_model_state);

    TEST_LOAD_WEIGHTS(MODEL_STATE_UNINITIALIZED);
    TEST_LOAD_WEIGHTS(MODEL_STATE_INITIALIZED);

#undef TEST_LOAD_WEIGHTS
}

/**
 * Tests model weights loading for invalid pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_weights_invalid_pointer)
{
    status_t status = STATUS_OK;

    g_model_state = MODEL_STATE_STRUCT_LOADED;

    status = model_load_weights(NULL, 0);

    zassert_equal(MODEL_STATUS_INV_PTR, status);
    zassert_equal(MODEL_STATE_STRUCT_LOADED, g_model_state);
}

/**
 * Tests model weights loading when runtime model init fails
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_weights_runtime_fail)
{
    status_t status = STATUS_OK;
    uint8_t model_weights[128] = {0};

    g_model_state = MODEL_STATE_STRUCT_LOADED;
    runtime_init_weights_fake.return_val = RUNTIME_WRAPPER_STATUS_ERROR;

    status = model_load_weights(model_weights, sizeof(model_weights));

    zassert_equal(RUNTIME_WRAPPER_STATUS_ERROR, status);
    zassert_equal(g_model_state, MODEL_STATE_STRUCT_LOADED);
}

// ========================================================
// model_get_input_size
// ========================================================

/**
 * Tests if get model input size properly computes the input size
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_input_size)
{
    status_t status = STATUS_OK;
    size_t input_size = 0;
    data_type_t input_data_type;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

#define TEST_GET_INPUT_SIZE(_model_state, _input_size_bytes, _input_length, _input_size) \
    g_model_state = (_model_state);                                                      \
    input_data_type.bits = _input_size_bytes * 8;                                        \
                                                                                         \
    g_model_spec.input_data_type[0] = input_data_type;                                   \
    g_model_spec.num_input_dim[0] = 1;                                                   \
    g_model_spec.input_shape[0][0] = (_input_length);                                    \
                                                                                         \
    status = model_get_input_size(&input_size);                                          \
                                                                                         \
    zassert_equal(STATUS_OK, status);                                                    \
    zassert_equal((_input_size), input_size);

    TEST_GET_INPUT_SIZE(MODEL_STATE_STRUCT_LOADED, 1, 1, 1);
    TEST_GET_INPUT_SIZE(MODEL_STATE_WEIGHTS_LOADED, 2, 2, 4);
    TEST_GET_INPUT_SIZE(MODEL_STATE_INPUT_LOADED, 4, 4, 16);
    TEST_GET_INPUT_SIZE(MODEL_STATE_INFERENCE_DONE, 3, 128, 384);

#undef TEST_GET_INPUT_SIZE
}

/**
 * Tests if get model input size fails when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_input_size_invalid_state)
{
    status_t status = STATUS_OK;
    size_t input_size = 0;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

    g_model_state = MODEL_STATE_UNINITIALIZED;

    status = model_get_input_size(&input_size);

    zassert_equal(MODEL_STATUS_INV_STATE, status);
}

/**
 * Tests if get model input size fails for invalid pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_input_size_invalid_pointer)
{
    status_t status = STATUS_OK;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

    status = model_get_input_size(NULL);

    zassert_equal(MODEL_STATUS_INV_PTR, status);
}

// ========================================================
// model_load_input
// ========================================================

/**
 * Tests model input loading for valid model states
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_input)
{
    status_t status = STATUS_OK;
    uint8_t model_input[MODEL_SPEC_INPUT_LEN * MODEL_SPEC_INPUT_SIZE] = {0};

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    status = model_load_input(model_input, sizeof(model_input));

    zassert_equal(STATUS_OK, status);
    zassert_equal(MODEL_STATE_INPUT_LOADED, g_model_state);
}

/**
 * Tests model input loading when allocation fails
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_input_runtime_fail)
{
    status_t status = STATUS_OK;
    uint8_t model_input[MODEL_SPEC_INPUT_LEN * MODEL_SPEC_INPUT_SIZE] = {0};

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;
    runtime_init_input_fake.return_val = RUNTIME_WRAPPER_STATUS_ERROR;

    status = model_load_input(model_input, sizeof(model_input));

    zassert_equal(RUNTIME_WRAPPER_STATUS_ERROR, status);
    zassert_equal(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

/**
 * Tests model input loading for invalid pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_input_invalid_pointer)
{
    status_t status = STATUS_OK;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

    status = model_load_input(NULL, MODEL_SPEC_INPUT_LEN * MODEL_SPEC_INPUT_SIZE);

    zassert_equal(MODEL_STATUS_INV_PTR, status);
    zassert_equal(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

/**
 * Tests model input loading when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_input_invalid_state)
{
    status_t status = STATUS_OK;
    uint8_t model_input[MODEL_SPEC_INPUT_LEN * MODEL_SPEC_INPUT_SIZE] = {0};

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

#define TEST_LOAD_INPUT(_model_state)                            \
    g_model_state = (_model_state);                              \
    status = model_load_input(model_input, sizeof(model_input)); \
                                                                 \
    zassert_equal(MODEL_STATUS_INV_STATE, status);               \
    zassert_equal((_model_state), g_model_state);

    TEST_LOAD_INPUT(MODEL_STATE_UNINITIALIZED);
    TEST_LOAD_INPUT(MODEL_STATE_STRUCT_LOADED);

#undef TEST_LOAD_INPUT
}

/**
 * Tests model input loading when input size is invalid
 */
ZTEST(kenning_inference_lib_test_model, test_model_load_input_invalid_size)
{
    status_t status = STATUS_OK;
    uint8_t model_input[MODEL_SPEC_INPUT_LEN * MODEL_SPEC_INPUT_SIZE] = {0};

    model_spec_input_length_fake.custom_fake = model_spec_input_length_mock;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

#define TEST_LOAD_INPUT(_input_size)                       \
    status = model_load_input(model_input, (_input_size)); \
                                                           \
    zassert_equal(MODEL_STATUS_INV_ARG, status);           \
    zassert_equal(MODEL_STATE_WEIGHTS_LOADED, g_model_state);

    TEST_LOAD_INPUT(sizeof(model_input) - 1);
    TEST_LOAD_INPUT(sizeof(model_input) + 1);
    TEST_LOAD_INPUT(0);
#undef TEST_LOAD_INPUT
}

// ========================================================
// model_run
// ========================================================

/**
 * Tests model execution for valid model states
 */
ZTEST(kenning_inference_lib_test_model, test_model_run)
{
    status_t status = STATUS_OK;

#define TEST_RUN(_model_state)        \
    g_model_state = (_model_state);   \
                                      \
    status = model_run();             \
                                      \
    zassert_equal(STATUS_OK, status); \
    zassert_equal(MODEL_STATE_INFERENCE_DONE, g_model_state);

    TEST_RUN(MODEL_STATE_INPUT_LOADED);
    TEST_RUN(MODEL_STATE_INFERENCE_DONE);

#undef TEST_RUN
}

/**
 * Tests model execution when inference fails
 */
ZTEST(kenning_inference_lib_test_model, test_model_run_runtime_fail)
{
    status_t status = STATUS_OK;

    g_model_state = MODEL_STATE_INPUT_LOADED;
    runtime_run_model_fake.return_val = RUNTIME_WRAPPER_STATUS_ERROR;

    status = model_run();

    zassert_equal(RUNTIME_WRAPPER_STATUS_ERROR, status);
    zassert_equal(MODEL_STATE_INPUT_LOADED, g_model_state);
}

/**
 * Tests model execution when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_run_invalid_state)
{
    status_t status = STATUS_OK;

#define TEST_RUN(_model_state)                     \
    g_model_state = (_model_state);                \
                                                   \
    status = model_run();                          \
                                                   \
    zassert_equal(MODEL_STATUS_INV_STATE, status); \
    zassert_equal((_model_state), g_model_state);

    TEST_RUN(MODEL_STATE_UNINITIALIZED);
    TEST_RUN(MODEL_STATE_STRUCT_LOADED);
    TEST_RUN(MODEL_STATE_WEIGHTS_LOADED);

#undef TEST_RUN
}

// ========================================================
// model_run_bench
// ========================================================

/**
 * Tests model execution for valid model states
 */
ZTEST(kenning_inference_lib_test_model, test_model_run_bench)
{
    status_t status = STATUS_OK;

#define TEST_RUN(_model_state)        \
    g_model_state = (_model_state);   \
                                      \
    status = model_run_bench();       \
                                      \
    zassert_equal(STATUS_OK, status); \
    zassert_equal(MODEL_STATE_INFERENCE_DONE, g_model_state);

    TEST_RUN(MODEL_STATE_INPUT_LOADED);
    TEST_RUN(MODEL_STATE_INFERENCE_DONE);

#undef TEST_RUN
}

/**
 * Tests model execution when inference fails
 */
ZTEST(kenning_inference_lib_test_model, test_model_run_bench_runtime_fail)
{
    status_t status = STATUS_OK;

    g_model_state = MODEL_STATE_INPUT_LOADED;
    runtime_run_model_bench_fake.return_val = RUNTIME_WRAPPER_STATUS_ERROR;

    status = model_run_bench();

    zassert_equal(RUNTIME_WRAPPER_STATUS_ERROR, status);
    zassert_equal(MODEL_STATE_INPUT_LOADED, g_model_state);
}

/**
 * Tests model execution when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_run_bench_invalid_state)
{
    status_t status = STATUS_OK;

#define TEST_RUN(_model_state)                     \
    g_model_state = (_model_state);                \
                                                   \
    status = model_run_bench();                    \
                                                   \
    zassert_equal(MODEL_STATUS_INV_STATE, status); \
    zassert_equal((_model_state), g_model_state);

    TEST_RUN(MODEL_STATE_UNINITIALIZED);
    TEST_RUN(MODEL_STATE_STRUCT_LOADED);
    TEST_RUN(MODEL_STATE_WEIGHTS_LOADED);

#undef TEST_RUN
}

// ========================================================
// model_get_output_size
// ========================================================

/**
 * Tests if get model output size properly computes the output size
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output_size)
{
    status_t status = STATUS_OK;
    size_t output_size = 0;
    data_type_t output_data_type;

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_GET_OUTPUT_SIZE(_model_state, _output_size_bytes, _output_length, _output_size) \
    g_model_state = (_model_state);                                                          \
    output_data_type.bits = (_output_size_bytes * 8);                                        \
                                                                                             \
    g_model_spec.num_output_dim[0] = 1;                                                      \
    g_model_spec.output_shape[0][0] = _output_length;                                        \
    g_model_spec.output_data_type[0] = output_data_type;                                     \
                                                                                             \
    status = model_get_output_size(&output_size);                                            \
                                                                                             \
    zassert_equal(STATUS_OK, status);                                                        \
    zassert_equal((_output_size), output_size);

    TEST_GET_OUTPUT_SIZE(MODEL_STATE_STRUCT_LOADED, 1, 1, 1);
    TEST_GET_OUTPUT_SIZE(MODEL_STATE_WEIGHTS_LOADED, 2, 2, 4);
    TEST_GET_OUTPUT_SIZE(MODEL_STATE_INPUT_LOADED, 4, 4, 16);
    TEST_GET_OUTPUT_SIZE(MODEL_STATE_INFERENCE_DONE, 3, 128, 384);

#undef TEST_GET_OUTPUT_SIZE
}

/**
 * Tests if get model output size fails when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output_size_invalid_state)
{
    status_t status = STATUS_OK;
    size_t output_size = 0;

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

    g_model_state = MODEL_STATE_UNINITIALIZED;

    status = model_get_output_size(&output_size);

    zassert_equal(MODEL_STATUS_INV_STATE, status);
}

// ========================================================
// model_get_output
// ========================================================

/**
 * Tests model get output for valid model states
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output)
{
    status_t status = STATUS_OK;
    uint8_t model_output[MODEL_SPEC_OUTPUT_LEN * MODEL_SPEC_OUTPUT_SIZE];
    size_t model_output_size = 0;

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    status = model_get_output(sizeof(model_output), model_output, &model_output_size);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MODEL_STATE_INFERENCE_DONE, g_model_state);
    zassert_equal(MODEL_SPEC_OUTPUT_LEN * MODEL_SPEC_OUTPUT_SIZE, model_output_size);
}

/**
 * Tests model get output for invalid buffer pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output_invalid_buffer_pointer)
{
    status_t status = STATUS_OK;
    size_t model_output_size = 0;

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    status = model_get_output(MODEL_SPEC_OUTPUT_LEN * MODEL_SPEC_OUTPUT_SIZE, NULL, &model_output_size);

    zassert_equal(MODEL_STATUS_INV_PTR, status);
    zassert_equal(MODEL_STATE_INFERENCE_DONE, g_model_state);
}

/**
 * Tests model get output for invalid output size pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output_invalid_size_pointer)
{
    status_t status = STATUS_OK;
    uint8_t model_output[MODEL_SPEC_OUTPUT_LEN * MODEL_SPEC_OUTPUT_SIZE];

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    status = model_get_output(sizeof(model_output), model_output, NULL);

    zassert_equal(STATUS_OK, status);
    zassert_equal(MODEL_STATE_INFERENCE_DONE, g_model_state);
}

/**
 * Tests model get output for not enough large buffer
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output_buffer_too_small)
{
    status_t status = STATUS_OK;
    uint8_t model_output[MODEL_SPEC_OUTPUT_LEN * MODEL_SPEC_OUTPUT_SIZE - 1];
    size_t model_output_size = 0;

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    status = model_get_output(sizeof(model_output), model_output, &model_output_size);

    zassert_equal(MODEL_STATUS_INV_ARG, status);
    zassert_equal(MODEL_STATE_INFERENCE_DONE, g_model_state);
}

/**
 * Tests model get output when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output_invalid_state)
{
    status_t status = STATUS_OK;
    uint8_t model_output[MODEL_SPEC_OUTPUT_LEN * MODEL_SPEC_OUTPUT_SIZE];
    size_t model_output_size = 0;

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

#define TEST_GET_OUTPUT(_model_state)                                                  \
    g_model_state = (_model_state);                                                    \
                                                                                       \
    status = model_get_output(sizeof(model_output), model_output, &model_output_size); \
                                                                                       \
    zassert_equal(MODEL_STATUS_INV_STATE, status);                                     \
    zassert_equal((_model_state), g_model_state);

    TEST_GET_OUTPUT(MODEL_STATE_UNINITIALIZED);
    TEST_GET_OUTPUT(MODEL_STATE_STRUCT_LOADED);
    TEST_GET_OUTPUT(MODEL_STATE_WEIGHTS_LOADED);
    TEST_GET_OUTPUT(MODEL_STATE_INPUT_LOADED);

#undef TEST_GET_OUTPUT
}

/**
 * Tests model get output when runtime get output fails
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_output_runtime_fail)
{
    status_t status = STATUS_OK;
    uint8_t model_output[MODEL_SPEC_OUTPUT_LEN * MODEL_SPEC_OUTPUT_SIZE];
    size_t model_output_size = 0;

    model_spec_output_length_fake.custom_fake = model_spec_output_length_mock;

    g_model_state = MODEL_STATE_INFERENCE_DONE;
    runtime_get_model_output_fake.return_val = RUNTIME_WRAPPER_STATUS_ERROR;

    status = model_get_output(sizeof(model_output), model_output, &model_output_size);

    zassert_equal(RUNTIME_WRAPPER_STATUS_ERROR, status);
    zassert_equal(MODEL_STATE_INFERENCE_DONE, g_model_state);
}

// ========================================================
// model_get_statistics
// ========================================================

/**
 * Tests model get statistics for valid model states
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_statistics)
{
    status_t status = STATUS_OK;
    uint8_t statistics_buffer[128] = {0};
    size_t statistics_size = 0;

#define TEST_GET_STATISTICS(_model_state)                                                          \
    g_model_state = (_model_state);                                                                \
                                                                                                   \
    status = model_get_statistics(sizeof(statistics_buffer), statistics_buffer, &statistics_size); \
                                                                                                   \
    zassert_equal(STATUS_OK, status);                                                              \
    zassert_equal((_model_state), g_model_state);

    TEST_GET_STATISTICS(MODEL_STATE_WEIGHTS_LOADED);
    TEST_GET_STATISTICS(MODEL_STATE_INPUT_LOADED);
    TEST_GET_STATISTICS(MODEL_STATE_INFERENCE_DONE);

#undef TEST_GET_STATISTICS
}

/**
 * Tests model get statistics for invalid buffer pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_statistics_invalid_buffer_pointer)
{
    status_t status = STATUS_OK;
    size_t statistics_size = 0;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    status = model_get_statistics(128, NULL, &statistics_size);

    zassert_equal(MODEL_STATUS_INV_PTR, status);
    zassert_equal(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

/**
 * Tests model get statistics for invalid size pointer
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_statistics_invalid_size_pointer)
{
    status_t status = STATUS_OK;
    uint8_t statistics_buffer[128] = {0};

    g_model_state = MODEL_STATE_INPUT_LOADED;

    status = model_get_statistics(sizeof(statistics_buffer), statistics_buffer, NULL);

    zassert_equal(MODEL_STATUS_INV_PTR, status);
    zassert_equal(MODEL_STATE_INPUT_LOADED, g_model_state);
}

/**
 * Tests model get statistics when model is in invalid state
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_statistics_invalid_state)
{
    status_t status = STATUS_OK;
    uint8_t statistics_buffer[128] = {0};
    size_t statistics_size = 0;

#define TEST_GET_STATISTICS(_model_state)                                                          \
    g_model_state = (_model_state);                                                                \
                                                                                                   \
    status = model_get_statistics(sizeof(statistics_buffer), statistics_buffer, &statistics_size); \
                                                                                                   \
    zassert_equal(MODEL_STATUS_INV_STATE, status);                                                 \
    zassert_equal((_model_state), g_model_state);

    TEST_GET_STATISTICS(MODEL_STATE_UNINITIALIZED);
    TEST_GET_STATISTICS(MODEL_STATE_STRUCT_LOADED);

#undef TEST_GET_STATISTICS
}

/**
 * Tests model get statistics when get model stats fails
 */
ZTEST(kenning_inference_lib_test_model, test_model_get_statistics_runtime_fails)
{
    status_t status = STATUS_OK;
    uint8_t statistics_buffer[128] = {0};
    size_t statistics_size = 0;

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;
    runtime_get_statistics_fake.return_val = RUNTIME_WRAPPER_STATUS_ERROR;

    status = model_get_statistics(sizeof(statistics_buffer), statistics_buffer, &statistics_size);

    zassert_equal(RUNTIME_WRAPPER_STATUS_ERROR, status);
    zassert_equal(MODEL_STATE_WEIGHTS_LOADED, g_model_state);
}

// ========================================================
// helper functions
// ========================================================

model_spec_t get_model_spec_data()
{
    model_spec_t model_spec = {
        MODEL_SPEC_INPUT_NUM,
        {4},
        {{1, 28, 28, 1}},
        {MODEL_SPEC_INPUT_DATA_TYPE},
        MODEL_SPEC_OUTPUT_NUM,
        {2},
        {{1, MODEL_SPEC_OUTPUT_LEN}},
        {MODEL_SPEC_OUTPUT_DATA_TYPE},
        "module.main",
        "module",
    };
    return model_spec;
}

// ========================================================
// mocks
// ========================================================

// We use macros to generate definitions for these functions, since they are almost identical.
#define GENERATE_MODEL_SPEC_LENGTH_CUSTOM_MOCK_DEFINITION(name)                                \
    uint32_t model_spec_##name##_length_mock(const model_spec_t *model_iospec, uint32_t index) \
    {                                                                                          \
        int result = 1;                                                                        \
        if (model_iospec == NULL)                                                              \
        {                                                                                      \
            return 0;                                                                          \
        }                                                                                      \
        if (index > model_iospec->num_##name)                                                  \
        {                                                                                      \
            return 0;                                                                          \
        }                                                                                      \
        for (int i = 0; i < model_iospec->num_##name##_dim[index]; i++)                        \
        {                                                                                      \
            result = result * model_iospec->name##_shape[index][i];                            \
        }                                                                                      \
        return result;                                                                         \
    }

// Generating definition for model_spec_input_length_mock
GENERATE_MODEL_SPEC_LENGTH_CUSTOM_MOCK_DEFINITION(input);
// Generating definition for model_spec_output_length_mock
GENERATE_MODEL_SPEC_LENGTH_CUSTOM_MOCK_DEFINITION(output);
