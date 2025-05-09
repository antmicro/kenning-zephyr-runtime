/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kenning_inference_lib/core/loaders.h>
#include <kenning_inference_lib/core/runtime_wrapper.h>

#include "ai8x_loaders.h"
#include "cnn.h"
#include "cnn_model.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ai8x_runtime, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(RUNTIME_WRAPPER);

status_t runtime_init()
{
    status_t status = STATUS_OK;

    status = prepare_ai8x_ldr_table();
    RETURN_ON_ERROR(status, status);

    cnn_enable();

    status = cnn_init();
    RETURN_ON_ERROR(status, RUNTIME_WRAPPER_STATUS_ERROR);

    return 0;
}

status_t runtime_init_weights()
{
    status_t status = STATUS_OK;

    status = cnn_configure();
    RETURN_ON_ERROR(status, RUNTIME_WRAPPER_STATUS_ERROR);

    return STATUS_OK;
}

status_t runtime_init_input() { return STATUS_OK; }

status_t runtime_run_model()
{
    status_t status = STATUS_OK;
    int wait_status = CNN_OK;

    cnn_set_clock(4, 3);
    cnn_start();
    wait_status = cnn_wait(CONFIG_KENNING_AI8X_CNN_TIMEOUT_MS);
    if (CNN_FAIL == wait_status)
    {
        status = RUNTIME_WRAPPER_STATUS_ERROR;
    }
    cnn_set_clock(1, 3);
    return status;
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    cnn_unload((uint32_t *)model_output);

    return STATUS_OK;
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    return STATUS_OK;
}

status_t runtime_deinit()
{
    cnn_disable();
    return STATUS_OK;
}
