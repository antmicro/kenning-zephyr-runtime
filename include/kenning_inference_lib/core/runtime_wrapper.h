/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
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

/*
 * Valid values for the code field in the data_type_t struct (that encodes data types):
 *  Integers:
 *    Int = 0U,
 *    UInt = 1U,
 *  IEEE floats:
 *    Float = 2U,
 *  Other floats:
 *    Bfloat = 4U,
 *    Float8_e3m4 = 7U,
 *    Float8_e4m3 = 8U,
 *    Float8_e4m3b11fnuz = 9U,
 *    Float8_e4m3fn = 10U,
 *    Float8_e4m3fnuz = 11U,
 *    Float8_e5m2 = 12U,
 *    Float8_e5m2fnuz = 13U,
 *    Float8_e8m0fnu = 14U,
 *    Float6_e2m3fn = 15U,
 *    Float6_e3m2fn = 16U,
 *    Float4_e2m1fn = 17U,
 *  Complex numbers:
 *    Complex = 5U,
 *  Booleans:
 *    Bool = 6U,
 *  Reserved:
 *    OpaqueHandle = 3U,
 *
 *  Valid data type codes are placed in the data_type_code_t enum below.
 */
#define DATA_TYPE_CODE(type, code) DATA_TYPE_##type = code,

#define DATA_TYPE_CODES(DATA_TYPE_CODE)    \
    DATA_TYPE_CODE(INT, 0U)                \
    DATA_TYPE_CODE(UINT, 1U)               \
    DATA_TYPE_CODE(FLOAT, 2U)              \
    DATA_TYPE_CODE(OPAQUE_HANDLE, 3U)      \
    DATA_TYPE_CODE(BFLOAT, 4U)             \
    DATA_TYPE_CODE(COMPLEX, 5U)            \
    DATA_TYPE_CODE(BOOL, 6U)               \
    DATA_TYPE_CODE(FLOAT8_E3M4, 7U)        \
    DATA_TYPE_CODE(FLOAT8_E4M3, 8U)        \
    DATA_TYPE_CODE(FLOAT8_E4M3B11FNUZ, 9U) \
    DATA_TYPE_CODE(FLOAT8_E4M3FN, 10U)     \
    DATA_TYPE_CODE(FLOAT8_E4M3FNUZ, 11U)   \
    DATA_TYPE_CODE(FLOAT8_E5M2, 12U)       \
    DATA_TYPE_CODE(FLOAT8_E5M2FNUZ, 13U)   \
    DATA_TYPE_CODE(FLOAT8_E8M0FNU, 14U)    \
    DATA_TYPE_CODE(FLOAT6_E2M3FN, 15U)     \
    DATA_TYPE_CODE(FLOAT6_E3M2FN, 16U)     \
    DATA_TYPE_CODE(FLOAT4_E2M1FN, 17U)     \
    DATA_TYPE_CODE(CODE_END, 18U)

typedef enum
{
    DATA_TYPE_CODES(DATA_TYPE_CODE)
} data_type_code_t;

#undef DATA_TYPE_CODE

/**
 * A type for storing/representing data types (uint32, int16, float32 etc)
 */
typedef struct __attribute__((packed))
{
    uint8_t code; // Data type code (compatible ith the DLPack's DLDataTypeCode
                  // - see block comment and macros above for more information)
    uint8_t bits; // Data type size in bits (for example 32 for uint32_t)
                  // - not using the data_type_code_t enum, because we need this to be 8-bit wide
} data_type_t;

/**
 * A struct that contains metadata of the Machine Learning model's input and output data.
 * It is used to globally store the metadata loaded by functions in model.c and model.h.
 * Then this information is used by runtimes in lib/kenning_inference_lib/runtimes, in loading inputs and reading
 * outputs.
 *
 * The struct can store information about size and data type for multiple input and output tensors, as well as some
 * other data. It is loaded directly from messages sent by Kening Protocol, by Kenning - so it has to be binary
 * compatible with serialization on the Kenning's side.
 *
 * Maximum capacities of struct fields are defined in model_contraints.h
 */

typedef struct __attribute__((packed))
{
    // Number of separate model inputs (each input can be a multi-dimensional tensor)
    uint32_t num_input;
    // Number of dimensions for each model input
    uint32_t num_input_dim[MAX_MODEL_INPUT_NUM];
    // Full shapes of all model inputs
    uint32_t input_shape[MAX_MODEL_INPUT_NUM][MAX_MODEL_INPUT_DIM];
    // Data type for all model inputs (stored in format compatible with DLPack's DLType - see data_tyee_t struct above)
    data_type_t input_data_type[MAX_MODEL_INPUT_NUM];

    // Number of separate model outputs (each output can be a multi-dimensional tensor)
    uint32_t num_output;
    // Number of dimensions for each model output
    uint32_t num_output_dim[MAX_MODEL_OUTPUT_NUM];
    // Full shapes of all model outputs
    uint32_t output_shape[MAX_MODEL_OUTPUT_NUM][MAX_MODEL_OUTPUT_DIM];
    // Data type for all model outputs (stored in format compatible with DLPack's DLType - see data_tyee_t struct above)
    data_type_t output_data_type[MAX_MODEL_OUTPUT_NUM];

    // ASCII string - Entry function of a model - in some runtimes it is necesarry for the runtime to call the specific
    // entry function in a compiled model
    uint8_t entry_func[MAX_LENGTH_ENTRY_FUNC_NAME];

    // ASCII string - Name of the model
    uint8_t model_name[MAX_LENGTH_MODEL_NAME];
} model_spec_t;

/**
 * The macros below are used for generating 2 functions: model_spec_input_length and model_spec_output_length
 * We use the macros, because those two functions are almost identical
 * - except one operates on the input_shape field, while the other operates on the output_shape field.
 * Macros generate both declarations and definitions for those functions (definitions are generated in rutime_wrapper.c
 * file)
 *
 * Below are the proper specifications for the generated functions:
 *
 * Function computes total length (product of all dimensions) for a given model input/output.
 *
 * @param model_spec pointer to a model_spec_t struct with model io specifications loaded
 * @param index number of input/output
 *
 * @returns Total length of the given input/output (given as number of elements in the tensor)
 */
#define GENERATE_MODEL_SPEC_LENGTH_FUNCTIONS(GENERATOR) \
    GENERATOR(input)                                    \
    GENERATOR(output)

#define GENERATE_MODEL_SPEC_LENGTH_FUNCTION_DECLARATION(name) \
    uint32_t model_spec_##name##_length(const model_spec_t *model_spec, uint32_t index);

#define GENERATE_MODEL_SPEC_LENGTH_FUNCTION_DEFINITION(name)                                                       \
    uint32_t model_spec_##name##_length(const model_spec_t *model_spec, uint32_t index)                            \
    {                                                                                                              \
        RETURN_ERROR_IF_POINTER_INVALID(model_spec, RUNTIME_WRAPPER_STATUS_INV_PTR);                               \
        if (index >= model_spec->num_##name)                                                                       \
        {                                                                                                          \
            LOG_ERR("Attempted to compute length of a non existent " #name " tensor: %d (only %d tensors exist).", \
                    index, model_spec->num_##name);                                                                \
            return 0;                                                                                              \
        }                                                                                                          \
        uint32_t result = 1;                                                                                       \
        for (unsigned int i = 0; i < model_spec->num_##name##_dim[index]; i++)                                     \
        {                                                                                                          \
            result = result * model_spec->name##_shape[index][i];                                                  \
        }                                                                                                          \
        return result;                                                                                             \
    }

/**
 * We are using the above macros to generate function declarations:
 */
GENERATE_MODEL_SPEC_LENGTH_FUNCTIONS(GENERATE_MODEL_SPEC_LENGTH_FUNCTION_DECLARATION);

#undef GENERATE_MODEL_SPEC_LENGTH_FUNCTION_DECLARATION

// Loads runtime stat into stats array
#define LOAD_RUNTIME_STAT(stats_array, stat_idx, src_struct, src_stat_name, stats_type)   \
    memset(stats_array[stat_idx].stat_name, 0, RUNTIME_STAT_NAME_MAX_LEN);                \
    snprintf(stats_array[stat_idx].stat_name, RUNTIME_STAT_NAME_MAX_LEN, #src_stat_name); \
    stats_array[stat_idx].stat_value = src_struct.src_stat_name;                          \
    stats_array[stat_idx].stat_type = (uint64_t)(stats_type)

#define MEASURE_TIME(stats, func)                                                     \
    do                                                                                \
    {                                                                                 \
        int64_t __timer_start = k_cycle_get_64();                                     \
        func;                                                                         \
        int64_t __timer_delta = k_cycle_get_64() - __timer_start;                     \
        (stats).target_inference_step = k_cyc_to_ns_floor64(__timer_delta);           \
        (stats).target_inference_step_timestamp = k_cyc_to_ns_floor64(__timer_start); \
    } while (0);

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
 * Runs model inference with a benchmark using wrapped runtime
 *
 * @returns status of the runtime
 */
status_t runtime_run_model_bench();

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
    LL_EXTENSION_SYMBOL(runtime_run_model_bench);  \
    LL_EXTENSION_SYMBOL(runtime_get_model_output); \
    LL_EXTENSION_SYMBOL(runtime_get_statistics);   \
    LL_EXTENSION_SYMBOL(runtime_deinit);

#endif // KENNING_INFERENCE_LIB_CORE_RUNTIME_WRAPPER_H_
