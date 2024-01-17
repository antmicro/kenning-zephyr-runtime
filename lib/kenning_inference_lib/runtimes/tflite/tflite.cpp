/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

extern "C"
{
#include "kenning_inference_lib/core/runtime_wrapper.h"
}

#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <zephyr/logging/log.h>

#include "resolver.h"

LOG_MODULE_REGISTER(tflite_runtime, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

extern MlModel g_model_struct;

extern tflite::MicroMutableOpResolver<TFLITE_RESOLVER_SIZE> g_tflite_resolver;
static tflite::MicroInterpreter *gp_tflite_interpreter = nullptr;

static uint8_t __attribute__((aligned(8))) g_tfliteBuffer[CONFIG_TFLITE_BUFFER_SIZE];

status_t runtime_init()
{
    tflite_initialize_resolver();
    return STATUS_OK;
}

status_t runtime_load_model_weights(const uint8_t *model_weights_data, const size_t model_size)
{
    uint8_t *modelWeights = g_tfliteBuffer;
    uint8_t *tensorArena = g_tfliteBuffer + model_size;
    size_t tensorArenaSize = CONFIG_TFLITE_BUFFER_SIZE - model_size;

    if (model_size > CONFIG_TFLITE_BUFFER_SIZE)
        return RUNTIME_WRAPPER_STATUS_ERROR;

    memcpy(modelWeights, model_weights_data, model_size);
    const tflite::Model *model = tflite::GetModel(modelWeights);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        LOG_ERR("Model provided is schema version %d not equal to supported version %d.\n", model->version(),
                TFLITE_SCHEMA_VERSION);
        return RUNTIME_WRAPPER_STATUS_ERROR;
    }

    static tflite::MicroInterpreter interpreter(model, g_tflite_resolver, tensorArena, tensorArenaSize);

    gp_tflite_interpreter = &interpreter;

    // this hack is used for reloading the interpreter on model change
    interpreter.~MicroInterpreter();
    new (&interpreter) tflite::MicroInterpreter(model, g_tflite_resolver, tensorArena, tensorArenaSize);

    TfLiteStatus allocate_status = interpreter.AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        LOG_ERR("AllocateTensors() failed\n");
        return RUNTIME_WRAPPER_STATUS_ERROR;
    }

    return STATUS_OK;
}

status_t runtime_load_model_input(const uint8_t *model_input)
{
    TfLiteTensor *input = gp_tflite_interpreter->input(0);
    memcpy(input->data.data, model_input, input->bytes);
    return STATUS_OK;
}

status_t runtime_run_model()
{
    TfLiteStatus status = gp_tflite_interpreter->Invoke();
    if (status == kTfLiteOk)
        return STATUS_OK;

    return RUNTIME_WRAPPER_STATUS_ERROR;
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    TfLiteTensor *output = gp_tflite_interpreter->output(0);
    memcpy(model_output, output->data.data, output->bytes);
    return STATUS_OK;
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    *statistics_size = 0;
    return STATUS_OK;
}
