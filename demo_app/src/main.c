/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <input_data.h>
#include <kenning_inference_lib/core/model.h>
#include <kenning_inference_lib/core/utils.h>
#include <model_data.h> /* header with model weights generated during build from ./models/magic_wand/<runtime>/ */
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo_app, CONFIG_DEMO_APP_LOG_LEVEL);

/* encode type string as uint32_t */
#define ENCODE_TYPE(t0, t1, t2, t3) ((t0) | ((t1) << 8) | ((t2) << 16) | ((t3) << 24))

/**
 * Struct with Magic Wand model params
 */
const MlModel model_struct = {
    .num_input = 1,
    .num_input_dim = {3},
    .input_shape = {{128, 3, 1}},
    .input_length = {384},
    .input_size_bytes = {4},
    .num_output = 1,
    .output_length = {4},
    .output_size_bytes = 4,
    .hal_element_type = ENCODE_TYPE('f', '3', '2', 0),
    .entry_func = "module.main",
    .model_name = "module",
};

/**
 * Magic Wand dataset classes
 */
const char *class_names[] = {"wing", "ring", "slope", "negative"};

/**
 * Helper function for formatting model output
 */
void format_output(uint8_t *buffer, const size_t buffer_size, uint8_t *model_output);

int main(void)
{
    status_t status = STATUS_OK;
    uint8_t *model_output = NULL;
    size_t model_output_size = 0;
    uint8_t output_str[512];

    do
    {
        // initialize model
        status = model_init();
        if (STATUS_OK != status)
        {
            LOG_ERR("Model initialization error 0x%x (%s)", status, get_status_str(status));
            break;
        }

        // load model structure
        status = model_load_struct((uint8_t *)&model_struct, sizeof(MlModel));
        BREAK_ON_ERROR_LOG(status, "Model struct load error 0x%x (%s)", status, get_status_str(status));

        // load model weights
        status = model_load_weights(model_data, model_data_len);
        BREAK_ON_ERROR_LOG(status, "Model weights load error 0x%x (%s)", status, get_status_str(status));

        // allocate buffer for output;
        model_get_output_size(&model_output_size);
        model_output = malloc(model_output_size);

        // inference loop
        for (size_t batch_index = 0; batch_index < sizeof(data) / sizeof(data[0]); ++batch_index)
        {
            status = model_load_input((uint8_t *)data[batch_index], sizeof(data[0]));
            BREAK_ON_ERROR_LOG(status, "Model input load error 0x%x (%s)", status, get_status_str(status));

            status = model_run();
            BREAK_ON_ERROR_LOG(status, "Model run error 0x%x (%s)", status, get_status_str(status));

            status = model_get_output(model_output_size, model_output, NULL);
            BREAK_ON_ERROR_LOG(status, "Model get output error 0x%x (%s)", status, get_status_str(status));

            format_output(output_str, sizeof(output_str), model_output);
            LOG_INF("model output: %s", output_str);
        }
    } while (0);

    if (IS_VALID_POINTER(model_output))
    {
        free(model_output);
    }
    if (STATUS_OK == status)
    {
        LOG_INF("inference done");
    }
    for (;;)
        ;
    return 0;
}

void format_output(uint8_t *buffer, const size_t buffer_size, uint8_t *model_output)
{
    uint8_t *buffer_end = buffer + buffer_size;

    buffer += snprintf(buffer, buffer_end - buffer, "[");
    for (int i = 0; i < model_struct.output_size_bytes * model_struct.output_length[0] / sizeof(float); ++i)
    {
        if (i > 0)
        {
            buffer += snprintf(buffer, buffer_end - buffer, ", ");
        }
        buffer += snprintf(buffer, buffer_end - buffer, "%s: ", class_names[i]);
        buffer += snprintf(buffer, buffer_end - buffer, "%f", ((float *)model_output)[i]);
    }
    buffer += snprintf(buffer, buffer_end - buffer, "]");
}
