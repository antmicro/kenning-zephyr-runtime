/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/runtime_wrapper.h"

GENERATE_MODULE_STATUSES_STR(RUNTIME_WRAPPER);

status_t runtime_init() { return STATUS_OK; }

status_t runtime_init_weights() { return STATUS_OK; }

status_t runtime_init_input() { return STATUS_OK; }

status_t runtime_run_model() { return STATUS_OK; }

status_t runtime_get_model_output(uint8_t *model_output) { return STATUS_OK; }

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    return STATUS_OK;
}

status_t runtime_deinit() { return STATUS_OK; }
