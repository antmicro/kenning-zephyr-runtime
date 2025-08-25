/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/model.h"
#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/runtime_wrapper.h"
#include <string.h>
#include <zephyr/sys/util.h>

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(model, CONFIG_MODEL_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(MODEL);

/*
 * Struct storing the metadata for the loaded Machine Learning model's inputs and outputs (definition in
 * runtime_wrapper.h) - loaded and validated in this file, used by runtimes.
 */
model_spec_t g_model_spec;

ut_static MODEL_STATE g_model_state = MODEL_STATE_UNINITIALIZED;

MODEL_STATE model_get_state() { return g_model_state; }

void model_reset_state() { g_model_state = MODEL_STATE_UNINITIALIZED; }

/**
 * Validates metadata (shapes and data types) of tensors stored in a struct
 * (intended for use with the model_spec_t struct from runtime_wrapper.h)
 * This funcition serves to minimize code repetition (code validating input and output tensors was
 * essentially identical, so instead this function can be called twice - once of input and once for output)
 * Note: The function takes a pointer to the struct and offsets to arrays,
 * instead of just pointers to arrays, in order to avoid alignment issues.
 *
 * @param tensor_struct Pointer to the struct containing the validated values
 * @param num_tensors Number of tensors being validated
 * @param num_tensor_dim_offset Offset (relative to the beginning of the struct) to array storing number of dimesnsions
 * for each tensor
 * @param tensor_shape_offset Offset (relative to the beginning of the struct) to a 2D array storing shape for each
 * tensor
 * @param tensor_data_type_offset Offset (relative to the beginning of the struct) to array storing data type for each
 * tensor
 * @param max_num_tensors Maximum number of tensors to validate (maximum value of num_tensors parameter) - should be
 * either MAX_MODEL_INPUT_NUM or MAX_MODEL_OUTPUT_NUM
 * @param max_num_tensor_dim Maximum number of dimensions for any tensor - should be either MAX_MODEL_OUTPUT_DIM or
 * MAX_MODEL_INPUT_DIM
 * @param tensors_name String, that will be used to refer to the tensors in the logs (either "input" or "output")
 *
 * @returns 1 if values are valid, 0 if values are not valid
 */
static int validate_tensors(model_spec_t *tensor_struct, uint32_t num_tensors, size_t num_tensor_dim_offset,
                            size_t tensor_shape_offset, size_t tensor_data_type_offset, uint32_t max_num_tensors,
                            uint32_t max_tensor_dim, const char tensors_name[])
{
    int tensor_invalid = 0;
    uint32_t *num_tensor_dim = (uint32_t *)((size_t)tensor_struct + num_tensor_dim_offset);
    uint32_t *tensor_shape = (uint32_t *)((size_t)tensor_struct + tensor_shape_offset);
    data_type_t *tensor_data_type = (data_type_t *)((size_t)tensor_struct + tensor_data_type_offset);
    if (num_tensors < 1 || num_tensors > max_num_tensors)
    {
        LOG_ERR("Wrong number of %s tensors: %d", tensors_name, num_tensors);
        tensor_invalid = 1;
    }
    for (int i = 0; i < MIN(num_tensors, max_num_tensors); ++i)
    {
        if (0 == num_tensor_dim[i] || num_tensor_dim[i] > max_tensor_dim)
        {
            LOG_ERR("Unsupported %s dimension number: %d (tensor: %d)", tensors_name, num_tensor_dim[i], i);
            tensor_invalid = 1;
        }

        if (tensor_data_type[i].bits % KENNING_BITS_PER_BYTE != 0)
        {
            LOG_ERR("Unsupported %s data type size: %d. (tensor: %d)", tensors_name, tensor_data_type[i].bits, i);
            tensor_invalid = 1;
        }
        if (tensor_data_type[i].code >= DATA_TYPE_CODE_END)
        {
            // Data type codes are compatible with DLPack's DLDataTypeCode enum - see commens in runtime_wrapper.h
            LOG_ERR(
                "Unsupported %s data type code: %d (tensor: %d). Check DATA_TYPE_CODES macro for available options.",
                tensors_name, tensor_data_type[i].code, i);
            tensor_invalid = 1;
        }
        for (int j = 0; j < MIN(num_tensor_dim[i], max_tensor_dim); j++)
        {
            if (0 == tensor_shape[max_tensor_dim * i + j])
            {
                LOG_ERR("Unsupported %s shape (can't be 0): %d (tensor: %d, dimension: %d)", tensors_name,
                        tensor_shape[max_tensor_dim * i + j], i, j);
                tensor_invalid = 1;
            }
        }
    }
    return tensor_invalid;
}

status_t prepare_iospec_loader()
{
    static struct msg_loader msg_loader_iospec = MSG_LOADER_BUF((uint8_t *)(&g_model_spec), sizeof(model_spec_t));
    g_ldr_tables[0][LOADER_TYPE_IOSPEC] = &msg_loader_iospec;
    return STATUS_OK;
}

status_t model_init()
{
    status_t status = STATUS_OK;

    status = runtime_init();
    RETURN_ON_ERROR(status, status);

    if (STATUS_OK == status)
    {
        g_model_state = MODEL_STATE_INITIALIZED;
    }

    status = prepare_iospec_loader();
    return status;
}

status_t model_load_struct_from_loader()
{
    status_t status = STATUS_OK;

    struct msg_loader *msg_loader_iospec = g_ldr_tables[0][LOADER_TYPE_IOSPEC];

    if (g_model_state < MODEL_STATE_INITIALIZED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    if (sizeof(model_spec_t) != msg_loader_iospec->written)
    {
        LOG_ERR("Wrong model struct size: %zu. Should be: %zu.", msg_loader_iospec->written, sizeof(model_spec_t));
        return MODEL_STATUS_INV_ARG;
    }

#define VALIDATE_TENSORS_CALL(name)                                                                           \
    if (validate_tensors(&g_model_spec, g_model_spec.num_##name, offsetof(model_spec_t, num_##name##_dim),    \
                         offsetof(model_spec_t, name##_shape), offsetof(model_spec_t, name##_data_type),      \
                         ARRAY_SIZE(g_model_spec.num_##name##_dim), ARRAY_SIZE(g_model_spec.name##_shape[0]), \
                         "" #name))                                                                           \
    {                                                                                                         \
        status = MODEL_STATUS_INV_ARG;                                                                        \
        return status;                                                                                        \
    }

    // Validate model input tensor metadata:
    VALIDATE_TENSORS_CALL(input);

    // Validate model output tensor metadata:
    VALIDATE_TENSORS_CALL(output);
#undef VALIDATE_TENSORS_CALL

    LOG_DBG("Loaded model struct. Model name: %s", g_model_spec.model_name);

    g_model_state = MODEL_STATE_STRUCT_LOADED;

    return status;
}

status_t model_load_weights_from_loader()
{
    status_t status = STATUS_OK;

    if (g_model_state < MODEL_STATE_STRUCT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    status = runtime_init_weights();
    RETURN_ON_ERROR(status, status);

    LOG_DBG("Initialized model weights");

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    return STATUS_OK;
}

status_t model_get_input_size(size_t *model_input_size)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(model_input_size, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_STRUCT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // compute input size
    size_t size = 0;
    for (int i = 0; i < g_model_spec.num_input; ++i)
    {
        size += (size_t)((model_spec_input_length(&g_model_spec, i) * g_model_spec.input_data_type[i].bits) /
                         KENNING_BITS_PER_BYTE);
    }

    *model_input_size = size;

    return status;
}

status_t model_load_input_from_loader(const size_t expected_size)
{
    status_t status = STATUS_OK;

    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // validate size of received data
    size_t computed_size = 0;
    status = model_get_input_size(&computed_size);

    RETURN_ON_ERROR(status, status);
    if (computed_size != expected_size)
    {
        return MODEL_STATUS_INV_ARG;
    }

    status = runtime_init_input();

    RETURN_ON_ERROR(status, status);

    LOG_DBG("Loaded model input");

    g_model_state = MODEL_STATE_INPUT_LOADED;

    return status;
}

status_t model_load_weights(const uint8_t *model_weights_data, const size_t data_size)
{
    status_t status = STATUS_OK;
    struct msg_loader *msg_loader_model = g_ldr_tables[1][LOADER_TYPE_MODEL];

    RETURN_ERROR_IF_POINTER_INVALID(model_weights_data, MODEL_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(msg_loader_model, MODEL_STATUS_INV_PTR);

    msg_loader_model->reset(msg_loader_model);
    status = msg_loader_model->save(msg_loader_model, model_weights_data, data_size);
    RETURN_ON_ERROR_LOG(status, status, "Model loader failed: %d", status);

    return model_load_weights_from_loader();
}

status_t model_load_struct(const uint8_t *model_spec_data, const size_t data_size)
{
    status_t status = STATUS_OK;
    struct msg_loader *msg_loader_iospec = g_ldr_tables[0][LOADER_TYPE_IOSPEC];

    RETURN_ERROR_IF_POINTER_INVALID(model_spec_data, MODEL_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(msg_loader_iospec, MODEL_STATUS_INV_PTR);

    msg_loader_iospec->reset(msg_loader_iospec);
    status = msg_loader_iospec->save(msg_loader_iospec, model_spec_data, data_size);
    RETURN_ON_ERROR_LOG(status, status, "iospec loader failed: %d", status);

    return model_load_struct_from_loader();
}

status_t model_load_input(const uint8_t *model_input, const size_t model_input_size)
{
    status_t status = STATUS_OK;
    struct msg_loader *msg_loader_data = g_ldr_tables[1][LOADER_TYPE_DATA];

    RETURN_ERROR_IF_POINTER_INVALID(model_input, MODEL_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(msg_loader_data, MODEL_STATUS_INV_PTR);

    msg_loader_data->reset(msg_loader_data);
    status = msg_loader_data->save(msg_loader_data, model_input, model_input_size);
    RETURN_ON_ERROR_LOG(status, status, "Data loader failed: %d", status);

    return model_load_input_from_loader(model_input_size);
}

status_t model_run()
{
    status_t status = STATUS_OK;

    if (g_model_state < MODEL_STATE_INPUT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // perform inference
    status = runtime_run_model();
    RETURN_ON_ERROR(status, status);

    LOG_DBG("Model inference done");

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    return status;
}

status_t model_run_bench()
{
    status_t status = STATUS_OK;

    if (g_model_state < MODEL_STATE_INPUT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // perform inference
    status = runtime_run_model_bench();
    RETURN_ON_ERROR(status, status);

    LOG_DBG("Model inference with a benchmark done");

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    return status;
}

status_t model_get_output_size(size_t *model_output_size)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(model_output_size, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_STRUCT_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    // compute output size
    size_t size = 0;
    for (int i = 0; i < g_model_spec.num_output; ++i)
    {
        size += (size_t)((model_spec_output_length(&g_model_spec, i) * g_model_spec.output_data_type[i].bits) /
                         KENNING_BITS_PER_BYTE);
    }

    *model_output_size = size;

    return status;
}

status_t model_get_output(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(model_output, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_INFERENCE_DONE)
    {
        return MODEL_STATUS_INV_STATE;
    }

    size_t output_size = 0;

    status = model_get_output_size(&output_size);
    RETURN_ON_ERROR(status, status);

    if (buffer_size < output_size)
    {
        LOG_ERR("Buffer is too small. Buffer size: %zu. Model output size: %zu", buffer_size, output_size);
        return MODEL_STATUS_INV_ARG;
    }
    if (IS_VALID_POINTER(model_output_size))
    {
        *model_output_size = output_size;
    }

    status = runtime_get_model_output(model_output);
    RETURN_ON_ERROR(status, status);

    LOG_DBG("Model output retrieved");

    return status;
}

status_t model_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer, size_t *statistics_size)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(statistics_buffer, MODEL_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(statistics_size, MODEL_STATUS_INV_PTR);

    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INV_STATE;
    }

    status = runtime_get_statistics(statistics_buffer_size, statistics_buffer, statistics_size);
    RETURN_ON_ERROR(status, status);

    LOG_DBG("Model statistics retrieved");

    return status;
}
