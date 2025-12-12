/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_TVM_GENERATED_MAGIC_WAND_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_TVM_GENERATED_MAGIC_WAND_H_

#include <tvm/runtime/crt/module.h>
#include <tvm/runtime/crt/packed_func.h>

#define TVMGEN_FUNCTIONS(FUNC)                                                           \
    FUNC(tvmgen_fused_layout_transform_cast_subtract)                                    \
    FUNC(tvmgen_fused_layout_transform_reshape_cast_subtract)                            \
    FUNC(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast)         \
    FUNC(tvmgen_fused_nn_conv2d_add_fixed_point_multiply_per_axis_add_clip_cast_1)       \
    FUNC(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_cast)                   \
    FUNC(tvmgen_fused_nn_dense_add_fixed_point_multiply_add_clip_subtract_cast_multiply) \
    FUNC(tvmgen_fused_nn_max_pool2d)                                                     \
    FUNC(tvmgen_fused_nn_max_pool2d_cast_subtract)                                       \
    FUNC(tvmgen_fused_nn_softmax_divide_round_add_clip_cast)                             \
    FUNC(tvmgen_fused_reshape_cast_subtract)

#define TVMGEN_FUNCTIONS_COUNT "\x0a"

#define TVMGEN_DECLARE(func_name)                                                                      \
    extern int32_t func_name(void *args, int32_t *arg_type_ids, int32_t num_args, void *out_ret_value, \
                             int32_t *out_ret_tcode, void *resource_handle);

#define TVMGEN_FUNC_ARRAY(func_name) (TVMBackendPackedCFunc) func_name,

#define TVMGEN_FUNC_REGISTRY(func_name) "\0" #func_name

TVMGEN_FUNCTIONS(TVMGEN_DECLARE)

const TVMBackendPackedCFunc tvm_func_array[] = {TVMGEN_FUNCTIONS(TVMGEN_FUNC_ARRAY)};

const TVMFuncRegistry tvm_func_reg = {.names = TVMGEN_FUNCTIONS_COUNT TVMGEN_FUNCTIONS(TVMGEN_FUNC_REGISTRY) "\0",
                                      .funcs = tvm_func_array};

const TVMModule g_tvm_module = {&tvm_func_reg};

const TVMModule *TVMSystemLibEntryPoint(void) { return &g_tvm_module; }

#endif // KENNING_INFERENCE_LIB_RUNTIMES_TVM_GENERATED_MAGIC_WAND_H_
