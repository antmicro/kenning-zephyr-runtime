/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

extern "C"
{
#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/runtime_wrapper.h"
#include "kenning_inference_lib/core/utils.h"
}

#include <memory>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <executorch/extension/data_loader/buffer_data_loader.h>
#include <executorch/extension/memory_allocator/malloc_memory_allocator.h>
#include <executorch/runtime/core/array_ref.h>
#include <executorch/runtime/core/error.h>
#include <executorch/runtime/core/portable_type/scalar_type.h>
#include <executorch/runtime/core/result.h>
#include <executorch/runtime/core/tensor_shape_dynamism.h>
#include <executorch/runtime/executor/program.h>
#include <executorch/runtime/platform/platform.h>
#include <executorch/runtime/platform/runtime.h>

using executorch::aten::Tensor;
using executorch::aten::TensorImpl;
using executorch::extension::BufferDataLoader;
using executorch::extension::MallocMemoryAllocator;
using executorch::runtime::Error;
using executorch::runtime::EValue;
using executorch::runtime::HierarchicalAllocator;
using executorch::runtime::MemoryManager;
using executorch::runtime::Method;
using executorch::runtime::MethodMeta;
using executorch::runtime::Program;
using executorch::runtime::Result;
using executorch::runtime::Span;
using executorch::runtime::etensor::ScalarType;

LOG_MODULE_REGISTER(executorch_runtime, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

extern model_spec_t g_model_spec;

static runtime_statistics_execution_time_t gp_executorch_time_stats;

static uint8_t
    __attribute__((aligned(16))) gp_executorchModelBuffer[CONFIG_KENNING_EXECUTORCH_MODEL_BUFFER_SIZE * 1024];
static uint8_t
    __attribute__((aligned(16))) gp_executorchInputBuffer[CONFIG_KENNING_EXECUTORCH_INPUT_BUFFER_SIZE * 1024];

struct planned_buffers_descriptor_t
{
    uint64_t total_size = 0;
    size_t num_planned_buffers;
    Span<uint8_t> *planned_buffers = nullptr;
    std::unique_ptr<HierarchicalAllocator> allocator;
};
static std::unique_ptr<MallocMemoryAllocator> gp_method_allocator;
static std::unique_ptr<MemoryManager> gp_memory_manager;
static planned_buffers_descriptor_t g_planned_buffers;
static std::unique_ptr<BufferDataLoader> gp_model;
static std::unique_ptr<Program> gp_program;
static std::unique_ptr<Method> gp_method;

static ScalarType kenning_elem_dtype_to_executorch_scalar_type(data_type_t *dtype)
{
    switch (static_cast<data_type_code_t>(dtype->code))
    {
    case DATA_TYPE_INT:
    {
        switch (dtype->bits)
        {
        case 8:
            return ScalarType::Char;
        case 16:
            return ScalarType::Short;
        case 32:
            return ScalarType::Int;
        case 64:
            return ScalarType::Long;
        default:
            LOG_ERR("Unsupported signed integer size: %d", dtype->bits);
            return ScalarType::Undefined;
        }
    };
    case DATA_TYPE_UINT:
    {
        switch (dtype->bits)
        {
        case 8:
            return ScalarType::Byte;
        case 16:
            return ScalarType::UInt16;
        case 32:
            return ScalarType::UInt32;
        case 64:
            return ScalarType::UInt64;
        default:
            LOG_ERR("Unsupported unsigned integer size: %d", dtype->bits);
            return ScalarType::Undefined;
        }
    };
    case DATA_TYPE_FLOAT:
    {
        switch (dtype->bits)
        {
        case 16:
            return ScalarType::Half;
        case 32:
            return ScalarType::Float;
        case 64:
            return ScalarType::Double;
        default:
            LOG_ERR("Unsupported IEEE float size: %d", dtype->bits);
            return ScalarType::Undefined;
        }
    }
    case DATA_TYPE_COMPLEX:
    {
        switch (dtype->bits)
        {
        case 16:
            return ScalarType::ComplexHalf;
        case 32:
            return ScalarType::ComplexFloat;
        case 64:
            return ScalarType::ComplexDouble;
        default:
            LOG_ERR("Unsupported complex float size: %d", dtype->bits);
            return ScalarType::Undefined;
        }
    }
    case DATA_TYPE_BFLOAT:
    {
        switch (dtype->bits)
        {
        case 16:
            return ScalarType::BFloat16;
        default:
            LOG_ERR("Unsupported bfloat size: %d", dtype->bits);
            return ScalarType::Undefined;
        }
    }
    case DATA_TYPE_FLOAT8_E4M3FN:
    {
        switch (dtype->bits)
        {
        case 8:
            return ScalarType::Float8_e4m3fn;
        default:
            LOG_ERR("Float8_e4m3fn should have 8 bits");
            return ScalarType::Undefined;
        }
    }
    case DATA_TYPE_FLOAT8_E4M3FNUZ:
    {
        switch (dtype->bits)
        {
        case 8:
            return ScalarType::Float8_e4m3fnuz;
        default:
            LOG_ERR("Float8_e4m3fnuz should have 8 bits");
            return ScalarType::Undefined;
        }
    }
    case DATA_TYPE_FLOAT8_E5M2:
    {
        switch (dtype->bits)
        {
        case 8:
            return ScalarType::Float8_e5m2;
        default:
            LOG_ERR("Float8_e5m2 should have 8 bits");
            return ScalarType::Undefined;
        }
    }
    case DATA_TYPE_FLOAT8_E5M2FNUZ:
    {
        switch (dtype->bits)
        {
        case 8:
            return ScalarType::Float8_e5m2fnuz;
        default:
            LOG_ERR("Float8_e5m2fnuz should have 8 bits");
            return ScalarType::Undefined;
        }
    }
    case DATA_TYPE_BOOL:
        return ScalarType::Bool;
    default:
        LOG_ERR("Unsupported type code: %d", dtype->code);
        return ScalarType::Undefined;
    }
}

static void deallocate_planned_buffers()
{
    g_planned_buffers.total_size = 0;
    if (g_planned_buffers.planned_buffers == nullptr)
    {
        return;
    }
    for (size_t i = 0; i < g_planned_buffers.num_planned_buffers; i++)
    {
        k_free(g_planned_buffers.planned_buffers[i].data());
    }
    k_free(g_planned_buffers.planned_buffers);
    g_planned_buffers.planned_buffers = nullptr;
}

status_t runtime_deinit()
{
    deallocate_planned_buffers();
    return STATUS_OK;
}

status_t runtime_init()
{
    static struct msg_loader msg_loader_model =
        MSG_LOADER_BUF(gp_executorchModelBuffer, CONFIG_KENNING_EXECUTORCH_MODEL_BUFFER_SIZE * 1024);
    static struct msg_loader msg_loader_input =
        MSG_LOADER_BUF(gp_executorchInputBuffer, CONFIG_KENNING_EXECUTORCH_INPUT_BUFFER_SIZE * 1024);
    memset(&g_ldr_tables[1], 0, NUM_LOADER_TYPES * sizeof(struct msg_loader *));
    g_ldr_tables[1][LOADER_TYPE_MODEL] = &msg_loader_model;
    g_ldr_tables[1][LOADER_TYPE_DATA] = &msg_loader_input;

    executorch::runtime::runtime_init();

    return STATUS_OK;
}

status_t runtime_init_weights()
{
    // De-allocating heap space, from the previously loaded model.
    deallocate_planned_buffers();

    static struct msg_loader *msg_loader_model = g_ldr_tables[1][LOADER_TYPE_MODEL];
    gp_model = std::make_unique<BufferDataLoader>(gp_executorchModelBuffer, msg_loader_model->written);
    Result<Program> program_result = Program::load(gp_model.get());
    RETURN_IF_FALSE_LOG(program_result.ok(), RUNTIME_WRAPPER_STATUS_ERROR, "Error loading model weights.");
    gp_program = std::make_unique<Program>(std::move(program_result.get()));

    const char *method_name = reinterpret_cast<const char *>(g_model_spec.entry_func);

    // Executorch requires us to pre-allocate buffers for tensors and pass them to a memory management object.
    // We retrieve the information about size and number of these buffers from a special object.
    Result<MethodMeta> method_meta = gp_program->method_meta(method_name);
    RETURN_IF_FALSE_LOG(method_meta.ok(), RUNTIME_WRAPPER_STATUS_ERROR, "Error retrieving inference method metadata.");
    g_planned_buffers.num_planned_buffers = method_meta->num_memory_planned_buffers();
    g_planned_buffers.planned_buffers =
        static_cast<Span<uint8_t> *>(k_malloc(sizeof(uint8_t *) * g_planned_buffers.num_planned_buffers));
    RETURN_IF_FALSE_LOG(IS_VALID_POINTER(g_planned_buffers.planned_buffers), RUNTIME_WRAPPER_STATUS_OUT_OF_MEMORY_ERROR,
                        "Insufficient heap space to allocate planned buffer pointer array.");
    g_planned_buffers.total_size = 0;
    for (size_t i = 0; i < g_planned_buffers.num_planned_buffers; i++)
    {
        size_t planned_buffer_size = static_cast<size_t>(method_meta->memory_planned_buffer_size(i).get());
        g_planned_buffers.planned_buffers[i] =
            Span(static_cast<uint8_t *>(k_malloc(planned_buffer_size)), planned_buffer_size);
        g_planned_buffers.total_size += planned_buffer_size;
        RETURN_IF_FALSE_LOG(IS_VALID_POINTER(g_planned_buffers.planned_buffers),
                            RUNTIME_WRAPPER_STATUS_OUT_OF_MEMORY_ERROR,
                            "Insufficient heap space to allocate planned buffer %lu.", i);
    }
    Span planned_buffer_span(g_planned_buffers.planned_buffers, g_planned_buffers.num_planned_buffers);

    g_planned_buffers.allocator = std::make_unique<HierarchicalAllocator>(planned_buffer_span);
    gp_method_allocator = std::make_unique<MallocMemoryAllocator>();
    gp_memory_manager = std::make_unique<MemoryManager>(gp_method_allocator.get(), g_planned_buffers.allocator.get());

    // We are loading the main inference method. It can either do all computations organically, to delegate some
    // of the work to a backend (for example XNNPack Backend). An error here likely means, that the model uses an
    // unsupported backend, because that's where 'delegates' are initialized.
    Result<Method> method_result = gp_program->load_method(method_name, gp_memory_manager.get());
    RETURN_IF_FALSE_LOG(method_result.ok(), RUNTIME_WRAPPER_STATUS_ERROR, "Error loading inference method.");
    gp_method = std::make_unique<Method>(std::move(method_result.get()));

    return STATUS_OK;
}

status_t runtime_init_input()
{
    for (unsigned int i = 0; i < g_model_spec.num_input; i++)
    {
        Tensor::SizesType shape[MAX_MODEL_INPUT_DIM];
        Tensor::DimOrderType dimension_order[MAX_MODEL_INPUT_DIM];

        for (unsigned int j = 0; j < g_model_spec.num_input_dim[i]; j++)
        {
            // Executorch uses it's own type for this array, so we cannot just pass our
            // (furthermore, since we have a packed struct, there may be alignment issues)
            shape[j] = g_model_spec.input_shape[i][j];
            dimension_order[j] = j;
        }
        TensorImpl impl(kenning_elem_dtype_to_executorch_scalar_type(&g_model_spec.input_data_type[i]),
                        g_model_spec.num_input_dim[i], shape, gp_executorchInputBuffer, dimension_order);

        Tensor input_tensor(&impl);

        // Implicitly casts to to EValue
        Error set_input_error = gp_method->set_input(input_tensor, i);
        RETURN_IF_FALSE_LOG(set_input_error == Error::Ok, RUNTIME_WRAPPER_STATUS_ERROR, "Error initializing input %d.",
                            i);
    }
    return STATUS_OK;
}

status_t runtime_run_model_bench()
{
    status_t status = STATUS_OK;
    MEASURE_TIME(gp_executorch_time_stats, status = runtime_run_model())
    return STATUS_OK;
}

status_t runtime_run_model()
{
    Error execute_error = gp_method->execute();
    RETURN_IF_FALSE_LOG(execute_error == Error::Ok, RUNTIME_WRAPPER_STATUS_ERROR, "Error while executing model.")
    return STATUS_OK;
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    RETURN_ERROR_IF_POINTER_INVALID(model_output, RUNTIME_WRAPPER_STATUS_INV_PTR);
    size_t offset = 0;
    for (unsigned int i = 0; i < g_model_spec.num_output; i++)
    {
        EValue output = gp_method->get_output(i);
        RETURN_IF_FALSE_LOG(output.isTensor(), RUNTIME_WRAPPER_STATUS_ERROR, "Error retrieving output %d.", i);
        const uint8_t *proc_output = output.toTensor().const_data_ptr<uint8_t>();
        size_t current_output_size = model_spec_output_length(&g_model_spec, i) *
                                     ((g_model_spec.output_data_type[i].bits - 1) / KENNING_BITS_PER_BYTE + 1);
        memcpy(model_output + offset, proc_output, current_output_size);
        offset += current_output_size;
    }
    return STATUS_OK;
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    size_t stats_size = sizeof(runtime_statistic_t);

    RETURN_ERROR_IF_POINTER_INVALID(statistics_buffer, RUNTIME_WRAPPER_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(statistics_size, RUNTIME_WRAPPER_STATUS_INV_PTR);

    if (stats_size > statistics_buffer_size)
    {
        LOG_ERR("Statistics buffer too small %zu, need at least %zu", statistics_buffer_size, stats_size);
        return RUNTIME_WRAPPER_STATUS_INV_ARG;
    }

    runtime_statistic_t *runtime_stats_ptr = (runtime_statistic_t *)statistics_buffer;

    LOAD_RUNTIME_STAT_FROM_VALUE(runtime_stats_ptr, 0, g_planned_buffers.total_size, planned_memory_size,
                                 RUNTIME_STATISTICS_ALLOCATION);

    *statistics_size = stats_size;
    return STATUS_OK;
}
