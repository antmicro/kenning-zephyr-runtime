/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_TVM_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_TVM_H_

#include <stdint.h>

#if defined(TVM_MODEL_LLEXT)
#define TVM_FUNC_REGISTRY_MAX_NAMES_LEN 512
#endif // TVM_MODEL_LLEXT

#define CHECK_TVM_STATUS_BREAK(status, tvm_status, log_format, log_args...) \
    if (0 != tvm_status)                                                    \
    {                                                                       \
        LOG_ERR(log_format, ##log_args);                                    \
        status = RUNTIME_WRAPPER_STATUS_ERROR;                              \
        break;                                                              \
    }

/**
 * Struct with TVM execution graph data
 */
typedef struct
{
    uint32_t graph_json_size;
    uint32_t graph_params_size;
    uint8_t graph_data[0]; /* concatenated graph JSON and params */
} tvm_graph_t;

/**
 * Returns pointer to graph JSON
 *
 * @param tvm_graph struct with graph data
 *
 * @returns pointer to graph JSON
 */
const uint8_t *tvm_graph_json_ptr(const tvm_graph_t *tvm_graph);

/**
 * Returns pointer to graph params
 *
 * @param tvm_graph struct with graph data
 *
 * @returns pointer to graph params
 */
const uint8_t *tvm_graph_params_ptr(const tvm_graph_t *tvm_graph);

/**
 * Returns TVM allocation statistics
 *
 * @param tvm_alloc_stats struct to save allocation statistics
 */
void tvm_get_allocation_stats(runtime_statistics_allocation_t *tvm_alloc_stats);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_TVM_H_
