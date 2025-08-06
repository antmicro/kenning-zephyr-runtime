/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "llext.h"
#include "exports/exports.h"
#include <kenning_inference_lib/core/runtime_wrapper.h>

#include <zephyr/llext/buf_loader.h>
#include <zephyr/llext/llext.h>
#include <zephyr/llext/symbol.h>

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(llext_runtime, CONFIG_MODEL_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(RUNTIME_WRAPPER);

static bool g_runtime_initialized = false;
static struct llext *gp_llext_runtime = NULL;

status_t runtime_init()
{
    if (g_runtime_initialized)
    {
        return STATUS_OK;
    }

    gp_llext_runtime = llext_by_name("runtime");
    FIND_P_FUNC(runtime_init)

    return p_func();
}

status_t runtime_init_weights()
{
    FIND_P_FUNC(runtime_init_weights)

    return p_func();
}

status_t runtime_init_input()
{
    FIND_P_FUNC(runtime_init_input)

    return p_func();
}

status_t runtime_run_model_bench()
{
    FIND_P_FUNC(runtime_run_model_bench)

    return p_func();
}

status_t runtime_run_model()
{
    FIND_P_FUNC(runtime_run_model)

    return p_func();
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    FIND_P_FUNC(runtime_get_model_output)

    return p_func(model_output);
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    FIND_P_FUNC(runtime_get_statistics)

    return p_func(statistics_buffer_size, statistics_buffer, statistics_size);
}

status_t runtime_deinit()
{
    g_runtime_initialized = false;

    return STATUS_OK;
}
