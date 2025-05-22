/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_MODEL_H_
#define KENNING_INFERENCE_LIB_CORE_MODEL_H_

#include "kenning_inference_lib/core/runtime_wrapper.h"
#include "kenning_inference_lib/core/utils.h"

/**
 * Model custom error codes
 */
#define MODEL_STATUSES(STATUS) STATUS(MODEL_STATUS_INV_STATE)

GENERATE_MODULE_STATUSES(MODEL);

/**
 * An enum that describes model state
 */
typedef enum
{
    MODEL_STATE_UNINITIALIZED = 0,
    MODEL_STATE_INITIALIZED = 1,
    MODEL_STATE_STRUCT_LOADED = 2,
    MODEL_STATE_WEIGHTS_LOADED = 3,
    MODEL_STATE_INPUT_LOADED = 4,
    MODEL_STATE_INFERENCE_DONE = 5,
} MODEL_STATE;

/**
 * Returns current model state
 *
 * @returns model state
 */
MODEL_STATE model_get_state();

/**
 * Resets model state
 */
void model_reset_state();

/**
 * Initializes model
 *
 * @returns status of the model
 */
status_t model_init();

/**
 * Loads model struct from given buffer
 *
 * @param model_struct_data buffer that contains model struct
 * @param data_size size of the buffer
 *
 * @returns status of the model
 */
status_t model_load_struct(const uint8_t *model_struct_data, const size_t data_size);

/**
 * Loads model weights from given buffer
 *
 * @param model_weights_data buffer that contains model weights
 * @param model_data_size size of the buffer
 *
 * @returns status of the model
 */
status_t model_load_weights(const uint8_t *model_weights_data, const size_t data_size);

/**
 * Calculates model input size based on data from model struct
 *
 * @param model_input_size output value
 *
 * @returns status of the model
 */
status_t model_get_input_size(size_t *model_input_size);

/**
 * Loads model input from given buffer
 *
 * @param model_input buffer that contains model input
 * @param model_input_size size of the buffer
 *
 * @returns status of the model
 */
status_t model_load_input(const uint8_t *model_input, const size_t model_input_size);

status_t model_load_struct_from_loader();

status_t model_load_weights_from_loader();

status_t model_load_input_from_loader(const size_t expected_size);

/**
 * Runs model inference with a benchmark
 *
 * @returns status of the model
 */
status_t model_run_bench();

/**
 * Runs model inference
 *
 * @returns status of the model
 */
status_t model_run();

/**
 * Calculates model output size based on data from model struct
 *
 * @param model_output_size output value
 *
 * @returns status of the model
 */
status_t model_get_output_size(size_t *model_output_size);

/**
 * Writes model output to given buffer
 *
 * @param buffer_size size of the buffer
 * @param model_output buffer to save model output
 * @param model_output_size actual size of the saved data
 *
 * @returns status of the model
 */
status_t model_get_output(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size);

/**
 * Retrieves model statistics
 *
 * @param statistics_buffer_size size of the provided buffer
 * @param statistics_buffer buffer provided for statistics
 * @param statistics_size size of the returned statistics data
 *
 * @returns status of the model
 */
status_t model_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer, size_t *statistics_size);

extern MlModel g_model_struct;

#endif // KENNING_INFERENCE_LIB_CORE_MODEL_H_
