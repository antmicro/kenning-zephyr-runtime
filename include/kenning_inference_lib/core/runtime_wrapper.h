/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_RUNTIME_WRAPPER_H_
#define KENNING_INFERENCE_LIB_CORE_RUNTIME_WRAPPER_H_

#include "kenning_inference_lib/core/model_constraints.h"
#include "kenning_inference_lib/core/utils.h"

/**
 * Runtime custom error codes
 */
#define RUNTIME_WRAPPER_STATUSES(STATUS)

GENERATE_MODULE_STATUSES(RUNTIME_WRAPPER);

#define RUNTIME_STAT_NAME_MAX_LEN 32

/**
 * Data types of tensor elements
 */
typedef enum
{
    HAL_ELEMENT_TYPE_INT_8,
    HAL_ELEMENT_TYPE_UINT_8,
    HAL_ELEMENT_TYPE_INT_16,
    HAL_ELEMENT_TYPE_UINT_16,
    HAL_ELEMENT_TYPE_INT_32,
    HAL_ELEMENT_TYPE_UINT_32,
    HAL_ELEMENT_TYPE_INT_64,
    HAL_ELEMENT_TYPE_UINT_64,
    HAL_ELEMENT_TYPE_FLOAT_16,
    HAL_ELEMENT_TYPE_FLOAT_32,
    HAL_ELEMENT_TYPE_FLOAT_64,
} HAL_ELEMENT_TYPE;

/**
 * Data types of tensor elements
 */
#define ELEMENT_DTYPES(ELEMENT_DTYPE)               \
    ELEMENT_DTYPE("i8", HAL_ELEMENT_TYPE_INT_8)     \
    ELEMENT_DTYPE("u8", HAL_ELEMENT_TYPE_UINT_8)    \
    ELEMENT_DTYPE("i16", HAL_ELEMENT_TYPE_INT_16)   \
    ELEMENT_DTYPE("u16", HAL_ELEMENT_TYPE_UINT_16)  \
    ELEMENT_DTYPE("i32", HAL_ELEMENT_TYPE_INT_32)   \
    ELEMENT_DTYPE("u32", HAL_ELEMENT_TYPE_UINT_32)  \
    ELEMENT_DTYPE("i64", HAL_ELEMENT_TYPE_INT_64)   \
    ELEMENT_DTYPE("u64", HAL_ELEMENT_TYPE_UINT_64)  \
    ELEMENT_DTYPE("f16", HAL_ELEMENT_TYPE_FLOAT_16) \
    ELEMENT_DTYPE("f32", HAL_ELEMENT_TYPE_FLOAT_32)

/**
 * A struct that contains model parameters
 */
typedef struct __attribute__((packed))
{
    uint32_t num_input;                                             // number of model inputs
    uint32_t num_input_dim[MAX_MODEL_INPUT_NUM];                    // dimensions of model inputs
    uint32_t input_shape[MAX_MODEL_INPUT_NUM][MAX_MODEL_INPUT_DIM]; // shapes of model inputs
    uint32_t input_length[MAX_MODEL_INPUT_NUM];                     // lengths of model inputs
                                                                    // (product of all dimensions)
    uint32_t input_size_bytes[MAX_MODEL_INPUT_NUM];                 // size in bytes of single
                                                                    // input element
    uint32_t num_output;                                            // number of model outputs
    uint32_t output_length[MAX_MODEL_OUTPUTS];                      // lengths of model outputs
    uint32_t output_size_bytes;                                     // size in bytes of single output element
    uint32_t hal_element_type;                                      // dtype of tensors elements
    uint8_t entry_func[MAX_LENGTH_ENTRY_FUNC_NAME];                 // entry function of a model
    uint8_t model_name[MAX_LENGTH_MODEL_NAME];                      // name of the model
} MlModel;

// Loads runtime stat into stats array
#define LOAD_RUNTIME_STAT(stats_array, stat_idx, src_struct, src_stat_name, stats_type)   \
    memset(stats_array[stat_idx].stat_name, 0, RUNTIME_STAT_NAME_MAX_LEN);                \
    snprintf(stats_array[stat_idx].stat_name, RUNTIME_STAT_NAME_MAX_LEN, #src_stat_name); \
    stats_array[stat_idx].stat_value = src_struct.src_stat_name;                          \
    stats_array[stat_idx].stat_type = (uint64_t)(stats_type)

typedef enum
{
    RUNTIME_STATISTICS_DEFAULT = 0,
    RUNTIME_STATISTICS_ALLOCATION = 1,
    RUNTIME_STATISTICS_INFERENCE_TIME = 2,
} runtime_statistic_type_t;

typedef struct
{
    uint64_t target_inference_step;
    uint64_t target_inference_step_timestamp;
} runtime_statistics_execution_time_t;

/**
 * Struct for holding various runtime statistics
 */
typedef struct
{
    char stat_name[RUNTIME_STAT_NAME_MAX_LEN];
    uint64_t stat_type;
    uint64_t stat_value;
} runtime_statistic_t;

/**
 * Initializes runtime
 *
 * @returns status of the runtime
 */
status_t runtime_init();

/**
 * Loads model weights using wrapped runtime
 *
 * @param model_weights_data buffer that contains model weights
 * @param model_data_size size of the buffer
 *
 * @returns status of the runtime
 */
status_t runtime_init_weights();

/**
 * Loads model input using wrapped runtime
 *
 * @param model_input buffer that contains model input
 *
 * @returns status of the runtime
 */
status_t runtime_init_input();

/**
 * Runs model inference using wrapped runtime
 *
 * @returns status of the runtime
 */
status_t runtime_run_model();

/**
 * Retrieves model output using wrapped runtime
 *
 * @param model_output buffer to save model output
 *
 * @returns status of the runtime
 */
status_t runtime_get_model_output(uint8_t *model_output);

/**
 * Retrieves runtime statistics
 *
 * @param statistics_buffer_size size of the provided buffer
 * @param statistics_buffer buffer provided for statistics
 * @param statistics_size size of the returned statistics data
 *
 * @returns status of the runtime
 */
status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size);

/**
 * Deinitializes runtime
 *
 * @returns status of the runtime
 */
status_t runtime_deinit();

/*
 * Creates LLEXT symbols for runtime methods
 */
#define RUNTIME_LL_EXTENSION_SYMBOLS               \
    LL_EXTENSION_SYMBOL(runtime_init);             \
    LL_EXTENSION_SYMBOL(runtime_init_weights);     \
    LL_EXTENSION_SYMBOL(runtime_init_input);       \
    LL_EXTENSION_SYMBOL(runtime_run_model);        \
    LL_EXTENSION_SYMBOL(runtime_get_model_output); \
    LL_EXTENSION_SYMBOL(runtime_get_statistics);   \
    LL_EXTENSION_SYMBOL(runtime_deinit);

#endif // KENNING_INFERENCE_LIB_CORE_RUNTIME_WRAPPER_H_
