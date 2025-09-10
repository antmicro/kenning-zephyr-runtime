/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/runtime_wrapper.h"

#include <dlpack/dlpack.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <tvm/runtime/c_backend_api.h>
#include <tvm/runtime/c_runtime_api.h>
#include <tvm/runtime/crt/crt.h>
#include <tvm/runtime/crt/func_registry.h>
#include <tvm/runtime/crt/graph_executor.h>
#include <tvm/runtime/crt/internal/graph_executor/graph_executor.h>
#include <tvm/runtime/crt/module.h>
#include <tvm/runtime/crt/packed_func.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#if defined(CONFIG_LLEXT)
#include <zephyr/llext/symbol.h>
#endif // defined(CONFIG_LLEXT)

#include "tvm.h"

#if defined(CONFIG_KENNING_TVM_MODEL_MAGIC_WAND)
#include "generated/magic_wand.h"
#elif defined(CONFIG_KENNING_TVM_MODEL_MAGIC_WAND_INT8)
#include "generated/magic_wand_int8.h"
#elif defined(CONFIG_KENNING_TVM_MODEL_GEN) || defined(CONFIG_KENNING_TVM_MODEL_PRE_GEN)
#include "generated/model_impl.h"
#else
#error "No model selected"
#endif

LOG_MODULE_REGISTER(tvm_runtime, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(RUNTIME_WRAPPER);

static runtime_statistics_execution_time_t gp_tvm_time_stats;

static uint8_t __attribute__((aligned(8))) gp_tvmGraphBuffer[CONFIG_KENNING_TVM_GRAPH_BUFFER_SIZE * 1024];
static uint8_t __attribute__((aligned(8))) gp_tvmInputBuffer[CONFIG_KENNING_TVM_INPUT_BUFFER_SIZE * 1024];

extern model_spec_t g_model_spec;

static bool g_tvm_runtime_initialized = false;
static const DLDevice g_device = {kDLCPU, 1};
static TVMGraphExecutor *gp_tvm_graph_executor = NULL;
static TVMModuleHandle g_tvm_module_handle;

void TVMLogf(const char *msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);
    vfprintf(stderr, msg, argptr);
    va_end(argptr);
}

status_t prepare_tvm_ldr_table()
{
    static struct msg_loader msg_loader_model =
        MSG_LOADER_BUF(gp_tvmGraphBuffer, CONFIG_KENNING_TVM_GRAPH_BUFFER_SIZE * 1024);
    static struct msg_loader msg_loader_input =
        MSG_LOADER_BUF(gp_tvmInputBuffer, CONFIG_KENNING_TVM_INPUT_BUFFER_SIZE * 1024);
    memset(&g_ldr_tables[1], 0, NUM_LOADER_TYPES * sizeof(struct msg_loader *));
    g_ldr_tables[1][LOADER_TYPE_MODEL] = &msg_loader_model;
    g_ldr_tables[1][LOADER_TYPE_DATA] = &msg_loader_input;
    return STATUS_OK;
}

status_t runtime_init()
{
    prepare_tvm_ldr_table();
    status_t status = STATUS_OK;
    int tvm_status = 0;

    do
    {
        if (g_tvm_runtime_initialized)
        {
            break;
        }

        tvm_status = TVMInitializeRuntime();
        CHECK_TVM_STATUS_BREAK(status, tvm_status, "TVM runtime init error 0x%x", tvm_status);

        g_tvm_runtime_initialized = true;
    } while (0);

    return status;
}

status_t runtime_init_weights()
{
    struct msg_loader *msg_loader_model = g_ldr_tables[1][LOADER_TYPE_MODEL];
    status_t status = STATUS_OK;
    int tvm_status = 0;

    do
    {
        const tvm_graph_t *tvm_graph = (tvm_graph_t *)gp_tvmGraphBuffer;

        if (IS_VALID_POINTER(gp_tvm_graph_executor))
        {
            tvm_status = TVMGraphExecutor_Release(&gp_tvm_graph_executor);
            CHECK_TVM_STATUS_BREAK(status, tvm_status, "Release graph executor error 0x%x", tvm_status);
            gp_tvm_graph_executor = NULL;
        }
        if (IS_VALID_POINTER(g_tvm_module_handle))
        {
            tvm_status = TVMModFree(g_tvm_module_handle);
            CHECK_TVM_STATUS_BREAK(status, tvm_status, "Release TVM module error 0x%x", tvm_status);
            g_tvm_module_handle = NULL;
        }

        if (msg_loader_model->written !=
            sizeof(tvm_graph_t) + tvm_graph->graph_json_size + tvm_graph->graph_params_size)
        {
            LOG_ERR("Invalid TVM graph or params size: %u %u", tvm_graph->graph_json_size,
                    tvm_graph->graph_params_size);
            status = RUNTIME_WRAPPER_STATUS_INV_ARG;
            break;
        }

        const TVMModule *tvm_module = TVMSystemLibEntryPoint();
        if (!IS_VALID_POINTER(tvm_module))
        {
            LOG_ERR("Invalid TVM lib entry point");
            return RUNTIME_WRAPPER_STATUS_INV_PTR;
        }

        tvm_status = TVMModCreateFromCModule(tvm_module, &g_tvm_module_handle);
        CHECK_TVM_STATUS_BREAK(status, tvm_status, "TVM module create error 0x%x", tvm_status);

        tvm_status = TVMGraphExecutor_Create(tvm_graph_json_ptr(tvm_graph), g_tvm_module_handle, &g_device,
                                             &gp_tvm_graph_executor);
        CHECK_TVM_STATUS_BREAK(status, tvm_status, "Create graph executor error 0x%x", tvm_status);

        tvm_status = TVMGraphExecutor_LoadParams(gp_tvm_graph_executor, tvm_graph_params_ptr(tvm_graph),
                                                 tvm_graph->graph_params_size);
        CHECK_TVM_STATUS_BREAK(status, tvm_status, "Load graph executor params error 0x%x", tvm_status);
    } while (0);

    return status;
}

status_t runtime_init_input()
{
    status_t status = STATUS_OK;
    DLTensor tensor_in;

    tensor_in.device = g_device;
    tensor_in.ndim = g_model_spec.num_input_dim[0];
    // Converting Kenning Zephyr Runtime data type format, to DLPack's DLDataType format used by TVM
    tensor_in.dtype.code = g_model_spec.input_data_type[0].code; // Data type codes in the Kenning format are the same
                                                                 // as in the DLPack's DLDataType
    tensor_in.dtype.bits = g_model_spec.input_data_type[0].bits;
    tensor_in.dtype.lanes = 0; // Default value
    int64_t shape[MAX_MODEL_INPUT_DIM];
    for (int i = 0; i < g_model_spec.num_input_dim[0]; ++i)
    {
        shape[i] = g_model_spec.input_shape[0][i];
    }
    tensor_in.shape = shape;
    tensor_in.strides = NULL;
    tensor_in.byte_offset = 0;

    tensor_in.data = (void *)gp_tvmInputBuffer;

    // TVM does not allow setting input by index, so we need to retrieve its name
    uint32_t input_node_id = gp_tvm_graph_executor->input_nodes[0];
    char *input_name = gp_tvm_graph_executor->nodes[input_node_id].name;

    TVMGraphExecutor_SetInput(gp_tvm_graph_executor, input_name, &tensor_in);

    return status;
}

status_t runtime_run_model_bench()
{
    status_t status = STATUS_OK;
    MEASURE_TIME(gp_tvm_time_stats, status = runtime_run_model())
    return STATUS_OK;
}

status_t runtime_run_model()
{
    status_t status = STATUS_OK;

    TVMGraphExecutor_Run(gp_tvm_graph_executor);

    return status;
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    status_t status = STATUS_OK;
    int tvm_status = 0;
    DLTensor tensor_out;

    RETURN_ERROR_IF_POINTER_INVALID(model_output, RUNTIME_WRAPPER_STATUS_INV_PTR);

    tensor_out.device = g_device;
    tensor_out.ndim = g_model_spec.num_output_dim[0];
    // Converting Kenning Zephyr Runtime data type format, to DLPack's DLDataType format used by TVM
    tensor_out.dtype.bits = g_model_spec.output_data_type[0].bits;
    tensor_out.dtype.code = g_model_spec.output_data_type[0].code; // Data type codes in the Kenning format are the
                                                                   // same as in the DLPack's DLDataType
    tensor_out.dtype.lanes = 0;                                    // Default value
    int64_t shape[MAX_MODEL_OUTPUT_DIM];
    for (int i = 0; i < g_model_spec.num_output_dim[0]; ++i)
    {
        shape[i] = g_model_spec.output_shape[0][i];
    }
    tensor_out.shape = shape;
    tensor_out.strides = NULL;
    tensor_out.byte_offset = 0;

    tensor_out.data = (void *)model_output;

    tvm_status = TVMGraphExecutor_GetOutput(gp_tvm_graph_executor, 0, &tensor_out);

    if (0 != tvm_status)
    {
        LOG_ERR("Get graph executor output error: 0x%x", tvm_status);
        status = RUNTIME_WRAPPER_STATUS_ERROR;
    }

    return status;
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    runtime_statistics_allocation_t tvm_alloc_stats;
    runtime_statistic_t *runtime_stats_ptr;
    size_t stats_size = sizeof(runtime_statistic_t) *
                        (sizeof(runtime_statistics_allocation_t) + sizeof(runtime_statistics_execution_time_t)) /
                        sizeof(uint64_t);

    RETURN_ERROR_IF_POINTER_INVALID(statistics_buffer, RUNTIME_WRAPPER_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(statistics_size, RUNTIME_WRAPPER_STATUS_INV_PTR);

    if (statistics_buffer_size < stats_size)
    {
        return RUNTIME_WRAPPER_STATUS_INV_ARG;
    }

    tvm_get_allocation_stats(&tvm_alloc_stats);

    runtime_stats_ptr = (runtime_statistic_t *)statistics_buffer;

    LOAD_RUNTIME_STAT(runtime_stats_ptr, 0, tvm_alloc_stats, total_allocated, RUNTIME_STATISTICS_ALLOCATION);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 1, tvm_alloc_stats, total_freed, RUNTIME_STATISTICS_ALLOCATION);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 2, tvm_alloc_stats, peak_allocated, RUNTIME_STATISTICS_ALLOCATION);

    LOAD_RUNTIME_STAT(runtime_stats_ptr, 3, gp_tvm_time_stats, target_inference_step,
                      RUNTIME_STATISTICS_INFERENCE_TIME);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 4, gp_tvm_time_stats, target_inference_step_timestamp,
                      RUNTIME_STATISTICS_INFERENCE_TIME);

    *statistics_size = stats_size;

    return STATUS_OK;
}

status_t runtime_deinit() { return STATUS_OK; }

#ifdef CONFIG_LLEXT
RUNTIME_LL_EXTENSION_SYMBOLS
#endif
