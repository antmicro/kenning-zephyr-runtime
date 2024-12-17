/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

extern "C"
{
#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/runtime_wrapper.h"
}

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include "resolver.h"

LOG_MODULE_REGISTER(tflite_runtime, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

extern MlModel g_model_struct;

extern tflite::MicroMutableOpResolver<TFLITE_RESOLVER_SIZE> g_tflite_resolver;
static tflite::MicroInterpreter *gp_tflite_interpreter = nullptr;

static uint8_t __attribute__((aligned(8))) g_tfliteBuffer[CONFIG_KENNING_TFLITE_BUFFER_SIZE * 1024];

status_t prepare_tflite_ldr_table()
{
    static struct msg_loader msg_loader_model =
        MSG_LOADER_BUF(g_tfliteBuffer, CONFIG_KENNING_TFLITE_BUFFER_SIZE * 1024);
    static struct msg_loader msg_loader_input = MSG_LOADER_BUF(NULL, 0);
    memset(&g_ldr_tables[1], 0, NUM_LOADER_TYPES * sizeof(struct msg_loader *));
    g_ldr_tables[1][LOADER_TYPE_MODEL] = &msg_loader_model;
    g_ldr_tables[1][LOADER_TYPE_DATA] = &msg_loader_input;
    return STATUS_OK;
}
status_t runtime_deinit() { return STATUS_OK; }

status_t runtime_init()
{
    prepare_tflite_ldr_table();
    tflite_initialize_resolver();
    return STATUS_OK;
}

status_t runtime_init_weights()
{
    struct msg_loader *msg_loader_model = g_ldr_tables[1][LOADER_TYPE_MODEL];
    struct msg_loader *msg_loader_input = g_ldr_tables[1][LOADER_TYPE_DATA];

    size_t model_size = msg_loader_model->written;
    uint8_t *modelWeights = g_tfliteBuffer;
    uint8_t *tensorArena = g_tfliteBuffer + model_size;
    size_t tensorArenaSize = CONFIG_KENNING_TFLITE_BUFFER_SIZE * 1024 - model_size;

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

    TfLiteTensor *input = gp_tflite_interpreter->input(0);
    msg_loader_input->addr = input->data.data;
    msg_loader_input->max_size = input->bytes;

    return STATUS_OK;
}

status_t runtime_init_input() { return STATUS_OK; }

status_t runtime_run_model()
{
    TfLiteStatus status = gp_tflite_interpreter->Invoke();
    if (status == kTfLiteOk)
    {
        return STATUS_OK;
    }

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

extern "C"
{
#if defined(CONFIG_LLEXT)

    void __attribute__((optimize("O0"))) tflite_opt_entrypoint()
    {
        runtime_deinit();
        runtime_init();
        runtime_init_weights();
        runtime_init_input();
        runtime_run_model();
        runtime_get_model_output(NULL);
        runtime_get_statistics(0, NULL, NULL);
        prepare_tflite_ldr_table();
    }

#endif
}

extern "C"
{
#if defined(CONFIG_LLEXT)
    RUNTIME_LL_EXTENSION_SYMBOLS
#endif // defined(CONFIG_LLEXT)
}