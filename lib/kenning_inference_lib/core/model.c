/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/model.h"
#include "kenning_inference_lib/core/loaders.h"
#include <string.h>

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(model, CONFIG_MODEL_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(MODEL);

MlModel g_model_struct;

ut_static MODEL_STATE g_model_state = MODEL_STATE_UNINITIALIZED;

MODEL_STATE model_get_state() { return g_model_state; }

void model_reset_state() { g_model_state = MODEL_STATE_UNINITIALIZED; }

status_t prepare_iospec_loader()
{
    static struct msg_loader msg_loader_iospec = MSG_LOADER_BUF((uint8_t *)(&g_model_struct), sizeof(MlModel));
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

    if (sizeof(MlModel) != msg_loader_iospec->written)
    {
        LOG_ERR("Wrong model struct size: %zu. Should be: %zu.", msg_loader_iospec->written, sizeof(MlModel));
        return MODEL_STATUS_INV_ARG;
    }

    // validate struct
    if (g_model_struct.num_input < 1 || g_model_struct.num_input > MAX_MODEL_INPUT_NUM ||
        g_model_struct.num_output < 1 || g_model_struct.num_output > MAX_MODEL_OUTPUTS)
    {
        LOG_ERR("Wrong number of inputs or outputs");
        return MODEL_STATUS_INV_ARG;
    }
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        if (0 == g_model_struct.num_input_dim[i] || g_model_struct.num_input_dim[i] > MAX_MODEL_INPUT_DIM)
        {
            LOG_ERR("Wrong input dim");
            return MODEL_STATUS_INV_ARG;
        }
        if (0 == g_model_struct.input_length[i])
        {
            LOG_ERR("Wrong input length");
            return MODEL_STATUS_INV_ARG;
        }
        for (int j = 0; j < g_model_struct.num_input_dim[i]; ++j)
        {
            if (0 == g_model_struct.input_shape[i][j])
            {
                LOG_ERR("Wrong input shape");
                return MODEL_STATUS_INV_ARG;
            }
        }
    }

    for (int i = 0; i < g_model_struct.num_output; ++i)
    {
        if (0 == g_model_struct.output_length[i])
        {
            LOG_ERR("Wrong output length");
            return MODEL_STATUS_INV_ARG;
        }
    }

    char *dtype = (char *)&g_model_struct.hal_element_type;

    // this x-macro retrieves string label and HAL element enum value from
    // ELEMENT_DTYPES table and compares this label with the string received with
    // the struct. If label is equal to this string then the relevand enum value
    // is assigned. If none of the labels is equal to this string then the final
    // else is hit and error is returned
#define CHECK_ELEM_TYPE(label, element_type)            \
    if (0 == strncmp(dtype, label, 4))                  \
    {                                                   \
        g_model_struct.hal_element_type = element_type; \
    }                                                   \
    else
    ELEMENT_DTYPES(CHECK_ELEM_TYPE)
    {
        LOG_ERR("Wrong dtype %.4s", dtype);
        return MODEL_STATUS_INV_ARG;
    }
#undef CHECK_ELEM_TYPE

    LOG_DBG("Loaded model struct. Model name: %s", g_model_struct.model_name);

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
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        size += (size_t)(g_model_struct.input_length[i] * g_model_struct.input_size_bytes[i]);
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

    msg_loader_model->reset(msg_loader_model, 0);
    status = msg_loader_model->save(msg_loader_model, model_weights_data, data_size);
    RETURN_ON_ERROR_LOG(status, status, "Model loader failed: %d", status);

    return model_load_weights_from_loader();
}

status_t model_load_struct(const uint8_t *model_struct_data, const size_t data_size)
{
    status_t status = STATUS_OK;
    struct msg_loader *msg_loader_iospec = g_ldr_tables[0][LOADER_TYPE_IOSPEC];

    RETURN_ERROR_IF_POINTER_INVALID(model_struct_data, MODEL_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(msg_loader_iospec, MODEL_STATUS_INV_PTR);

    msg_loader_iospec->reset(msg_loader_iospec, 0);
    status = msg_loader_iospec->save(msg_loader_iospec, model_struct_data, data_size);
    RETURN_ON_ERROR_LOG(status, status, "iospec loader failed: %d", status);

    return model_load_struct_from_loader();
}

status_t model_load_input(const uint8_t *model_input, const size_t model_input_size)
{
    status_t status = STATUS_OK;
    struct msg_loader *msg_loader_data = g_ldr_tables[1][LOADER_TYPE_DATA];

    RETURN_ERROR_IF_POINTER_INVALID(model_input, MODEL_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(msg_loader_data, MODEL_STATUS_INV_PTR);

    msg_loader_data->reset(msg_loader_data, 0);
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
    for (int i = 0; i < g_model_struct.num_output; ++i)
    {
        size += (size_t)(g_model_struct.output_length[i] * g_model_struct.output_size_bytes);
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
