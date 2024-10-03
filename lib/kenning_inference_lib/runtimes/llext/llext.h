/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_H_

#include <kenning_inference_lib/core/utils.h>

#define CHECK_RUNTIME_LLEXT_LOADED_RET(p_llext) \
    if (!IS_VALID_POINTER(gp_llext_runtime))    \
    {                                           \
        LOG_ERR("Runtime LLEXT not loaded");    \
        return RUNTIME_WRAPPER_STATUS_ERROR;    \
    }

#define CHECK_RUNTIME_LLEXT_FUNC_LOADED_RET(p_func, func_name)     \
    if (!IS_VALID_POINTER(p_func))                                 \
    {                                                              \
        LOG_ERR("LLEXT runtime function " func_name " not found"); \
        return RUNTIME_WRAPPER_STATUS_ERROR;                       \
    }

#define EXPORT_SYMBOL_NAMED(x, n)                                         \
    static const STRUCT_SECTION_ITERABLE(llext_const_symbol, n##_sym) = { \
        .name = STRINGIFY(n),                                             \
        .addr = (const void *)&x,                                         \
    };

typedef status_t (*runtime_init_ptr_t)(void);
typedef status_t (*runtime_load_model_weights_ptr_t)(const uint8_t *model_weights_data, const size_t data_size);
typedef status_t (*runtime_load_model_input_ptr_t)(const uint8_t *model_input);
typedef status_t (*runtime_run_model_ptr_t)(void);
typedef status_t (*runtime_get_model_output_ptr_t)(uint8_t *model_output);
typedef status_t (*runtime_get_statistics_ptr_t)(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                                 size_t *statistics_size);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_H_
