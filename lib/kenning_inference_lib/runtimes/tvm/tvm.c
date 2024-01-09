/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kenning_inference_lib/core/runtime_wrapper.h>

#include <dlpack/dlpack.h>
#include <stdbool.h>
#include <string.h>
#include <tvm/runtime/c_backend_api.h>
#include <tvm/runtime/c_runtime_api.h>
#include <tvm/runtime/crt/crt.h>
#include <tvm/runtime/crt/func_registry.h>
#include <tvm/runtime/crt/graph_executor.h>
#include <tvm/runtime/crt/internal/graph_executor/graph_executor.h>
#include <tvm/runtime/crt/module.h>
#include <tvm/runtime/crt/packed_func.h>
#include <zephyr/logging/log.h>

#include "tvm.h"

#if defined(TVM_MODEL_MAGIC_WAND)
#include "generated/magic_wand.h"
#else
#error "No model selected"
#endif

LOG_MODULE_REGISTER(tvm_runtime, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(RUNTIME_WRAPPER);

extern MlModel g_model_struct;
extern const TVMModule g_tvm_module;

static bool g_tvm_runtime_initialized = false;
static const DLDevice g_device = {kDLCPU, 1};
static TVMGraphExecutor *gp_tvm_graph_executor = NULL;
static TVMModuleHandle g_tvm_module_handle;

static const DLDataType G_HAL_ELEM_DTYPE_TO_DLPACK_DTYPE[] = {
    {kDLInt, 8, 0},    /* HAL_ELEMENT_TYPE_INT_8 */
    {kDLUInt, 8, 0},   /* HAL_ELEMENT_TYPE_UINT_8 */
    {kDLInt, 16, 0},   /* HAL_ELEMENT_TYPE_INT_16 */
    {kDLUInt, 16, 0},  /* HAL_ELEMENT_TYPE_UINT_16 */
    {kDLInt, 32, 0},   /* HAL_ELEMENT_TYPE_INT_32 */
    {kDLUInt, 32, 0},  /* HAL_ELEMENT_TYPE_UINT_32 */
    {kDLInt, 64, 0},   /* HAL_ELEMENT_TYPE_INT_64 */
    {kDLUInt, 64, 0},  /* HAL_ELEMENT_TYPE_UINT_64 */
    {kDLFloat, 16, 0}, /* HAL_ELEMENT_TYPE_FLOAT_16 */
    {kDLFloat, 32, 0}, /* HAL_ELEMENT_TYPE_FLOAT_32 */
    {kDLFloat, 64, 0}, /* HAL_ELEMENT_TYPE_FLOAT_64 */
};

void TVMLogf(const char *msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);
    vfprintf(stderr, msg, argptr);
    va_end(argptr);
}

status_t runtime_init()
{
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

        tvm_status = TVMModCreateFromCModule(&g_tvm_module, &g_tvm_module_handle);
        CHECK_TVM_STATUS_BREAK(status, tvm_status, "TVM module create error 0x%x", tvm_status);

        g_tvm_runtime_initialized = true;
    } while (0);

    return status;
}

status_t runtime_load_model_weights(const uint8_t *model_weights_data, const size_t data_size)
{
    status_t status = STATUS_OK;
    int tvm_status = 0;

    RETURN_ERROR_IF_POINTER_INVALID(model_weights_data, RUNTIME_WRAPPER_STATUS_INV_PTR);

    do
    {
        const tvm_graph_t *tvm_graph = (tvm_graph_t *)model_weights_data;

        if (NULL != gp_tvm_graph_executor)
        {
            tvm_status = TVMGraphExecutor_Release(&gp_tvm_graph_executor);
            CHECK_TVM_STATUS_BREAK(status, tvm_status, "Release graph executor error 0x%x", tvm_status);
            gp_tvm_graph_executor = NULL;
        }

        if (data_size != sizeof(tvm_graph_t) + tvm_graph->graph_json_size + tvm_graph->graph_params_size)
        {
            LOG_ERR("Invalid TVM graph size");
            status = RUNTIME_WRAPPER_STATUS_INV_ARG;
            break;
        }

        tvm_status = TVMGraphExecutor_Create(tvm_graph_json_ptr(tvm_graph), g_tvm_module_handle, &g_device,
                                             &gp_tvm_graph_executor);
        CHECK_TVM_STATUS_BREAK(status, tvm_status, "Create graph executor error 0x%x", tvm_status);

        tvm_status = TVMGraphExecutor_LoadParams(gp_tvm_graph_executor, tvm_graph_params_ptr(tvm_graph),
                                                 tvm_graph->graph_params_size);
        CHECK_TVM_STATUS_BREAK(status, tvm_status, "Load graph executor params error 0x%x", tvm_status);
    } while (0);

    return status;
}

status_t runtime_load_model_input(const uint8_t *model_input)
{
    status_t status = STATUS_OK;
    DLTensor tensor_in;

    RETURN_ERROR_IF_POINTER_INVALID(model_input, RUNTIME_WRAPPER_STATUS_INV_PTR);

    tensor_in.device = g_device;
    tensor_in.ndim = g_model_struct.num_input_dim[0];
    tensor_in.dtype = G_HAL_ELEM_DTYPE_TO_DLPACK_DTYPE[g_model_struct.hal_element_type];
    int64_t shape[MAX_MODEL_INPUT_DIM];
    for (int i = 0; i < g_model_struct.num_input_dim[0]; ++i)
    {
        shape[i] = g_model_struct.input_shape[0][i];
    }
    tensor_in.shape = shape;
    tensor_in.strides = NULL;
    tensor_in.byte_offset = 0;

    tensor_in.data = (void *)model_input;

    TVMGraphExecutor_SetInput(gp_tvm_graph_executor, "input_1", &tensor_in);

    return status;
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
    tensor_out.ndim = 2;
    tensor_out.dtype = G_HAL_ELEM_DTYPE_TO_DLPACK_DTYPE[g_model_struct.hal_element_type];
    int64_t shape[] = {1, g_model_struct.output_length[0]};
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
    tvm_alloc_stats_t tvm_alloc_stats;
    runtime_statistic_t *runtime_stats_ptr;
    size_t stats_size = sizeof(runtime_statistic_t) * sizeof(tvm_alloc_stats_t) / sizeof(uint64_t);

    RETURN_ERROR_IF_POINTER_INVALID(statistics_buffer, RUNTIME_WRAPPER_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(statistics_size, RUNTIME_WRAPPER_STATUS_INV_PTR);

    if (statistics_buffer_size < stats_size)
    {
        return RUNTIME_WRAPPER_STATUS_INV_ARG;
    }

    tvm_get_allocation_stats(&tvm_alloc_stats);

    runtime_stats_ptr = (runtime_statistic_t *)statistics_buffer;

    memset(runtime_stats_ptr[0].stat_name, 0, RUNTIME_STAT_NAME_MAX_LEN);
    snprintf(runtime_stats_ptr[0].stat_name, RUNTIME_STAT_NAME_MAX_LEN, "tvm_total_allocated");
    runtime_stats_ptr[0].stat_value = tvm_alloc_stats.total_allocated;

    memset(runtime_stats_ptr[1].stat_name, 0, RUNTIME_STAT_NAME_MAX_LEN);
    snprintf(runtime_stats_ptr[1].stat_name, RUNTIME_STAT_NAME_MAX_LEN, "tvm_total_freed");
    runtime_stats_ptr[1].stat_value = tvm_alloc_stats.total_freed;

    memset(runtime_stats_ptr[2].stat_name, 0, RUNTIME_STAT_NAME_MAX_LEN);
    snprintf(runtime_stats_ptr[2].stat_name, RUNTIME_STAT_NAME_MAX_LEN, "tvm_peak_allocated");
    runtime_stats_ptr[2].stat_value = tvm_alloc_stats.peak_allocated;

    *statistics_size = stats_size;

    return STATUS_OK;
}
