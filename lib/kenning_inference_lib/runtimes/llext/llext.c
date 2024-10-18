/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
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
    CHECK_RUNTIME_LLEXT_LOADED_RET(gp_llext_runtime);

    runtime_init_ptr_t p_func = llext_find_sym(&gp_llext_runtime->exp_tab, "runtime_init");

    CHECK_RUNTIME_LLEXT_FUNC_LOADED_RET(p_func, "runtime_init")

    return p_func();
}

status_t runtime_load_model_weights(const uint8_t *model_weights_data, const size_t data_size)
{
    CHECK_RUNTIME_LLEXT_LOADED_RET(gp_llext_runtime);

    runtime_load_model_weights_ptr_t p_fun = llext_find_sym(&gp_llext_runtime->exp_tab, "runtime_load_model_weights");

    CHECK_RUNTIME_LLEXT_FUNC_LOADED_RET(p_fun, "runtime_load_model_weights");

    return p_fun(model_weights_data, data_size);
}

status_t runtime_load_model_input(const uint8_t *model_input)
{
    CHECK_RUNTIME_LLEXT_LOADED_RET(gp_llext_runtime);

    runtime_load_model_input_ptr_t p_func = llext_find_sym(&gp_llext_runtime->exp_tab, "runtime_load_model_input");

    CHECK_RUNTIME_LLEXT_FUNC_LOADED_RET(p_func, "runtime_load_model_input");

    return p_func(model_input);
}

status_t runtime_run_model()
{
    CHECK_RUNTIME_LLEXT_LOADED_RET(gp_llext_runtime);

    runtime_run_model_ptr_t p_func = llext_find_sym(&gp_llext_runtime->exp_tab, "runtime_run_model");

    CHECK_RUNTIME_LLEXT_FUNC_LOADED_RET(p_func, "runtime_run_model");

    return p_func();
}

status_t runtime_get_model_output(uint8_t *model_output)
{
    CHECK_RUNTIME_LLEXT_LOADED_RET(gp_llext_runtime);

    runtime_get_model_output_ptr_t p_func = llext_find_sym(&gp_llext_runtime->exp_tab, "runtime_get_model_output");

    CHECK_RUNTIME_LLEXT_FUNC_LOADED_RET(p_func, "runtime_get_model_output");

    return p_func(model_output);
}

status_t runtime_get_statistics(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                size_t *statistics_size)
{
    CHECK_RUNTIME_LLEXT_LOADED_RET(gp_llext_runtime);

    runtime_get_statistics_ptr_t p_func = llext_find_sym(&gp_llext_runtime->exp_tab, "runtime_get_statistics");

    CHECK_RUNTIME_LLEXT_FUNC_LOADED_RET(p_func, "runtime_get_statistics");

    return p_func(statistics_buffer_size, statistics_buffer, statistics_size);
}

status_t runtime_deinit()
{
    g_runtime_initialized = false;

    return STATUS_OK;
}
