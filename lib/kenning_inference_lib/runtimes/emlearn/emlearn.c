/*
 * Copyright (c) 2026 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/model.h"
#include "kenning_inference_lib/core/runtime_wrapper.h"
#include "kenning_inference_lib/core/utils.h"

#include <zephyr/kernel.h>

GENERATE_MODULE_STATUSES_STR(RUNTIME_WRAPPER);

static runtime_statistics_execution_time_t gp_emlearn_time_stats;

static uint8_t gp_emlearn_input_buffer[CONFIG_KENNING_EMLEARN_INPUT_BUFFER_SIZE * 1024];
static uint8_t gp_emlearn_output_buffer[CONFIG_KENNING_EMLEARN_OUTPUT_BUFFER_SIZE];

void emlearn_model(const uint8_t *input, uint8_t *output);

int buf_save_ignore(struct msg_loader *ldr, const uint8_t *src, size_t n) { return STATUS_OK; }

int buf_save_one_ignore(struct msg_loader *ldr, void *c) { return STATUS_OK; }

int buf_reset_ignore(struct msg_loader *ldr) { return STATUS_OK; }

status_t runtime_deinit() { return STATUS_OK; }

status_t runtime_init()
{
    // For this framework we build with the model, so we can ignore model data we receive.
    static struct msg_loader msg_loader_model;
    msg_loader_model.save = buf_save_ignore;
    msg_loader_model.save_one = buf_save_one_ignore;
    msg_loader_model.reset = buf_reset_ignore;

    static struct msg_loader msg_loader_input =
        MSG_LOADER_BUF(gp_emlearn_input_buffer, CONFIG_KENNING_EMLEARN_INPUT_BUFFER_SIZE * 1024);
    memset(&g_ldr_tables[1], 0, NUM_LOADER_TYPES * sizeof(struct msg_loader *));
    g_ldr_tables[1][LOADER_TYPE_MODEL] = &msg_loader_model;
    g_ldr_tables[1][LOADER_TYPE_DATA] = &msg_loader_input;
    return STATUS_OK;
}

status_t runtime_init_weights() { return STATUS_OK; }

status_t runtime_init_input() { return STATUS_OK; }

status_t runtime_run_model_bench()
{
    status_t status = STATUS_OK;
    MEASURE_TIME(gp_emlearn_time_stats, status = runtime_run_model())
    return STATUS_OK;
}

status_t runtime_run_model()
{
    emlearn_model(gp_emlearn_input_buffer, gp_emlearn_output_buffer);
    return STATUS_OK;
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    size_t output_size = 0;
    model_get_output_size(&output_size);
    memcpy(model_output, gp_emlearn_output_buffer, output_size);
    return STATUS_OK;
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    runtime_statistic_t *runtime_stats_ptr;
    const size_t stats_size = 2 * sizeof(runtime_statistic_t);

    RETURN_ERROR_IF_POINTER_INVALID(statistics_buffer, RUNTIME_WRAPPER_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(statistics_size, RUNTIME_WRAPPER_STATUS_INV_PTR);

    if (statistics_buffer_size < stats_size)
    {
        return RUNTIME_WRAPPER_STATUS_INV_ARG;
    }

    runtime_stats_ptr = (runtime_statistic_t *)statistics_buffer;

    LOAD_RUNTIME_STAT(runtime_stats_ptr, 0, gp_emlearn_time_stats, target_inference_step,
                      RUNTIME_STATISTICS_INFERENCE_TIME);
    LOAD_RUNTIME_STAT(runtime_stats_ptr, 1, gp_emlearn_time_stats, target_inference_step_timestamp,
                      RUNTIME_STATISTICS_INFERENCE_TIME);

    *statistics_size = stats_size;

    return STATUS_OK;
}
