/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kenning_inference_lib/core/runtime_wrapper.h>
#include <kenning_inference_lib/core/utils.h>

#include "iree.h"

#include <iree/base/status.h>
#include <iree/modules/hal/module.h>
#include <iree/vm/bytecode_module.h>
#include <iree/vm/ref.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(iree_runtime, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(RUNTIME_WRAPPER);

static const iree_hal_element_type_t G_HAL_ELEM_DTYPE_TO_IREE_HAL_ELEM_TYPE[] = {
    IREE_HAL_ELEMENT_TYPE_INT_8,    /* HAL_ELEMENT_TYPE_INT_8 */
    IREE_HAL_ELEMENT_TYPE_UINT_8,   /* HAL_ELEMENT_TYPE_UINT_8 */
    IREE_HAL_ELEMENT_TYPE_INT_16,   /* HAL_ELEMENT_TYPE_INT_16 */
    IREE_HAL_ELEMENT_TYPE_UINT_16,  /* HAL_ELEMENT_TYPE_UINT_16 */
    IREE_HAL_ELEMENT_TYPE_INT_32,   /* HAL_ELEMENT_TYPE_INT_32 */
    IREE_HAL_ELEMENT_TYPE_UINT_32,  /* HAL_ELEMENT_TYPE_UINT_32 */
    IREE_HAL_ELEMENT_TYPE_INT_64,   /* HAL_ELEMENT_TYPE_INT_64 */
    IREE_HAL_ELEMENT_TYPE_UINT_64,  /* HAL_ELEMENT_TYPE_UINT_64 */
    IREE_HAL_ELEMENT_TYPE_FLOAT_16, /* HAL_ELEMENT_TYPE_FLOAT_16 */
    IREE_HAL_ELEMENT_TYPE_FLOAT_32, /* HAL_ELEMENT_TYPE_FLOAT_32 */
};

/**
 * Initialization status
 */
static bool runtime_initialized = false;

/**
 * IREE runtime instance
 */
static iree_vm_instance_t *gp_instance = NULL;
/**
 * IREE device
 */
static iree_hal_device_t *gp_device = NULL;
/**
 * IREE execution context where modules are loaded
 */
static iree_vm_context_t *gp_context = NULL;

/**
 * Buffer for model inputs
 */
static iree_vm_list_t *gp_model_inputs = NULL;
/**
 * Buffer for model outputs
 */
static iree_vm_list_t *gp_model_outputs = NULL;

/**
 * Buffer for model weights
 */
static uint8_t *gp_model_weights = NULL;

/**
 * Struct describing model IO
 */
extern MlModel g_model_struct;

/**
 * Releases context that hold modules' state
 */
static void release_context()
{
    // release resources if already allocated
    if (NULL != gp_context)
    {
        iree_vm_context_release(gp_context);
        gp_context = NULL;
    }
    if (NULL != gp_model_weights)
    {
        free(gp_model_weights);
        gp_model_weights = NULL;
    }
}

/**
 * Creates context that hold modules' state
 *
 * @param model_data compiled model data
 * @param model_data_size size of compiled model data
 *
 * @returns error status
 */
status_t create_context(const uint8_t *model_data, const size_t model_data_size)
{
    status_t status = STATUS_OK;
    iree_status_t iree_status = iree_ok_status();
    iree_vm_module_t *hal_module = NULL;
    iree_vm_module_t *module = NULL;

    release_context();

    do
    {
        iree_allocator_t host_allocator = iree_allocator_system();

        // prepare model weights
        gp_model_weights = aligned_alloc(8, model_data_size);
        if (!IS_VALID_POINTER(gp_model_weights))
        {
            status = RUNTIME_WRAPPER_STATUS_INV_PTR;
            break;
        }
        memcpy(gp_model_weights, model_data, model_data_size);

        // create bytecode module
        iree_status =
            iree_vm_bytecode_module_create(gp_instance, iree_make_const_byte_span(gp_model_weights, model_data_size),
                                           iree_allocator_null(), host_allocator, &module);
        BREAK_ON_IREE_ERROR(iree_status);

        // create hal_module
        iree_status = iree_hal_module_create(gp_instance, gp_device, IREE_HAL_MODULE_FLAG_SYNCHRONOUS, host_allocator,
                                             &hal_module);
        BREAK_ON_IREE_ERROR(iree_status);

        iree_vm_module_t *modules[] = {hal_module, module};

        // allocate context
        iree_status = iree_vm_context_create_with_modules(
            gp_instance, IREE_VM_CONTEXT_FLAG_NONE, IREE_ARRAYSIZE(modules), &modules[0], host_allocator, &gp_context);
    } while (0);

    // cleanup
    if (IS_VALID_POINTER(hal_module))
    {
        iree_vm_module_release(hal_module);
    }
    if (IS_VALID_POINTER(module))
    {
        iree_vm_module_release(module);
    }
    if (!iree_status_is_ok(iree_status))
    {
        release_context();
    }
    CHECK_IREE_STATUS(iree_status);

    return status;
}

/**
 * Prepares model input HAL buffers
 *
 * @param model_input model input
 * @param arg_buffer_views output buffers views
 *
 * @returns error status
 */
static iree_status_t prepare_input_hal_buffer_views(const uint8_t *model_input,
                                                    iree_hal_buffer_view_t **arg_buffer_views)
{
    iree_status_t iree_status = iree_ok_status();

    iree_const_byte_span_t *byte_span[MAX_MODEL_INPUT_NUM] = {NULL};
    size_t offset = 0;

    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        size_t size = g_model_struct.input_size_bytes[i] * g_model_struct.input_length[i];
        byte_span[i] = malloc(sizeof(iree_const_byte_span_t));
        *byte_span[i] = iree_make_const_byte_span(model_input + offset, size);
        offset += size;
    }

    iree_hal_buffer_params_t buffer_params = {.type =
                                                  IREE_HAL_MEMORY_TYPE_HOST_LOCAL | IREE_HAL_MEMORY_TYPE_DEVICE_VISIBLE,
                                              .access = IREE_HAL_MEMORY_ACCESS_READ,
                                              .usage = IREE_HAL_BUFFER_USAGE_DEFAULT};
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        iree_status = iree_hal_buffer_view_allocate_buffer(
            iree_hal_device_allocator(gp_device), g_model_struct.num_input_dim[i], g_model_struct.input_shape[i],
            G_HAL_ELEM_DTYPE_TO_IREE_HAL_ELEM_TYPE[g_model_struct.hal_element_type],
            IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR, buffer_params, *byte_span[i], &(arg_buffer_views[i]));
        BREAK_ON_IREE_ERROR(iree_status);
    }

    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        if (NULL != byte_span[i])
        {
            free(byte_span[i]);
            byte_span[i] = NULL;
        }
    }

    return iree_status;
}

status_t prepare_input_buffer(const uint8_t *model_input)
{
    iree_status_t iree_status = iree_ok_status();

    iree_status = iree_vm_list_create(
        /*element_type=*/NULL, /*initial_capacity=*/g_model_struct.num_input, iree_allocator_system(),
        &gp_model_inputs);
    CHECK_IREE_STATUS(iree_status);

    iree_hal_buffer_view_t *arg_buffer_views[MAX_MODEL_INPUT_NUM] = {NULL};
    iree_status = prepare_input_hal_buffer_views(model_input, arg_buffer_views);
    CHECK_IREE_STATUS(iree_status);

    iree_vm_ref_t arg_buffer_view_ref;
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        arg_buffer_view_ref = iree_hal_buffer_view_move_ref(arg_buffer_views[i]);
        iree_status = iree_vm_list_push_ref_move(gp_model_inputs, &arg_buffer_view_ref);
        CHECK_IREE_STATUS(iree_status);
    }

    return STATUS_OK;
}

status_t prepare_output_buffer()
{
    iree_status_t iree_status = iree_ok_status();

    iree_status = iree_vm_list_create(
        /*element_type=*/NULL, /*initial_capacity=*/1, iree_allocator_system(), &gp_model_outputs);
    CHECK_IREE_STATUS(iree_status);

    return STATUS_OK;
}

static void release_input_buffer()
{
    if (NULL != gp_model_inputs)
    {
        iree_vm_list_release(gp_model_inputs);
        gp_model_inputs = NULL;
    }
}

static void release_output_buffer()
{
    if (NULL != gp_model_outputs)
    {
        iree_vm_list_release(gp_model_outputs);
        gp_model_outputs = NULL;
    }
}

status_t runtime_init()
{
    status_t status = STATUS_OK;
    iree_status_t iree_status = iree_ok_status();
    iree_allocator_t host_allocator = iree_allocator_system();

    if (runtime_initialized)
    {
        return STATUS_OK;
    }

    if (IS_VALID_POINTER(gp_instance))
    {
        iree_vm_instance_release(gp_instance);
        gp_instance = NULL;
    }
    if (IS_VALID_POINTER(gp_device))
    {
        iree_hal_device_release(gp_device);
        gp_instance = NULL;
    }
    do
    {
        // create vm instance
        iree_status = iree_vm_instance_create(host_allocator, &gp_instance);
        BREAK_ON_IREE_ERROR(iree_status);

        iree_status = iree_hal_module_register_all_types(gp_instance);
        BREAK_ON_IREE_ERROR(iree_status);

        // create device
        iree_status = create_device(gp_instance, host_allocator, &gp_device);
        BREAK_ON_IREE_ERROR(iree_status);

        runtime_initialized = true;
    } while (0);
    CHECK_IREE_STATUS(iree_status);

    return status;
}

status_t runtime_load_model_weights(const uint8_t *model_weights_data, const size_t data_size)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(model_weights_data, RUNTIME_WRAPPER_STATUS_INV_PTR);

    // free input/output resources
    release_output_buffer();
    release_input_buffer();

    status = create_context(model_weights_data, data_size);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}
status_t runtime_load_model_input(const uint8_t *model_input)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(model_input, RUNTIME_WRAPPER_STATUS_INV_PTR);

    // free resources
    release_input_buffer();

    // setup buffers for inputs
    status = prepare_input_buffer(model_input);
    RETURN_ON_ERROR(status, status);

    return STATUS_OK;
}

status_t runtime_run_model()
{
    status_t status = STATUS_OK;
    iree_status_t iree_status = iree_ok_status();
    iree_vm_function_t main_function;

    // free resources
    release_output_buffer();

    // setup buffers for outputs
    status = prepare_output_buffer();
    RETURN_ON_ERROR(status, status);

    iree_status = iree_vm_context_resolve_function(
        gp_context, iree_make_cstring_view((char *)g_model_struct.entry_func), &main_function);
    CHECK_IREE_STATUS(iree_status);

    // invoke model
    iree_status = iree_vm_invoke(gp_context, main_function, IREE_VM_INVOCATION_FLAG_NONE,
                                 /*policy=*/NULL, gp_model_inputs, gp_model_outputs, iree_allocator_system());
    CHECK_IREE_STATUS(iree_status);

    return status;
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    iree_status_t iree_status = iree_ok_status();

    RETURN_ERROR_IF_POINTER_INVALID(model_output, RUNTIME_WRAPPER_STATUS_INV_PTR);

    size_t model_output_offset = 0;
    for (int output_idx = 0; output_idx < g_model_struct.num_output; ++output_idx)
    {
        iree_hal_buffer_mapping_t mapped_memory = {0};
        iree_hal_buffer_view_t *ret_buffer_view = NULL;
        // get the result buffers from the invocation.
        ret_buffer_view = (iree_hal_buffer_view_t *)iree_vm_list_get_ref_deref(gp_model_outputs, output_idx,
                                                                               iree_hal_buffer_view_get_descriptor());
        if (NULL == ret_buffer_view)
        {
            return RUNTIME_WRAPPER_STATUS_INV_PTR;
        }
        iree_status =
            iree_hal_buffer_map_range(iree_hal_buffer_view_buffer(ret_buffer_view), IREE_HAL_MAPPING_MODE_SCOPED,
                                      IREE_HAL_MEMORY_ACCESS_READ, 0, IREE_WHOLE_BUFFER, &mapped_memory);
        CHECK_IREE_STATUS(iree_status);

        if ((output_idx > g_model_struct.num_output ||
             mapped_memory.contents.data_length / g_model_struct.output_size_bytes !=
                 g_model_struct.output_length[output_idx]) &&
            NULL == ret_buffer_view)
        {
            return RUNTIME_WRAPPER_STATUS_INV_PTR;
        }
        memcpy(&model_output[model_output_offset], mapped_memory.contents.data,
               g_model_struct.output_size_bytes * g_model_struct.output_length[output_idx]);

        model_output_offset += g_model_struct.output_size_bytes * g_model_struct.output_length[output_idx];

        iree_hal_buffer_unmap_range(&mapped_memory);
    }

    return STATUS_OK;
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    iree_hal_allocator_statistics_t iree_alloc_stats;
    runtime_statistic_t *runtime_stats_ptr;
    size_t stats_size =
        sizeof(runtime_statistic_t) * sizeof(iree_hal_allocator_statistics_t) / sizeof(iree_device_size_t);

    RETURN_ERROR_IF_POINTER_INVALID(statistics_buffer, RUNTIME_WRAPPER_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(statistics_size, RUNTIME_WRAPPER_STATUS_INV_PTR);

    if (stats_size > statistics_buffer_size)
    {
        LOG_ERR("Statistics buffer too small %zu, need at least %zu", statistics_buffer_size, stats_size);
        return RUNTIME_WRAPPER_STATUS_INV_ARG;
    }

    iree_hal_allocator_query_statistics(iree_hal_device_allocator(gp_device), &iree_alloc_stats);

    runtime_stats_ptr = (runtime_statistic_t *)statistics_buffer;

    LOAD_RUNTIME_STAT(runtime_stats_ptr, 0, iree_alloc_stats, device_bytes_allocated);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 1, iree_alloc_stats, device_bytes_freed);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 2, iree_alloc_stats, device_bytes_peak);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 3, iree_alloc_stats, host_bytes_allocated);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 4, iree_alloc_stats, host_bytes_freed);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 5, iree_alloc_stats, host_bytes_peak);

    *statistics_size = stats_size;

    return STATUS_OK;
}
