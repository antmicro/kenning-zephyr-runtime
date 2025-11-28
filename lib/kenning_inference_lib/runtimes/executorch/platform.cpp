/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <executorch/runtime/platform/compiler.h>
#include <executorch/runtime/platform/log.h>
#include <executorch/runtime/platform/platform.h>

extern "C"
{
#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/runtime_wrapper.h"
#include "kenning_inference_lib/core/utils.h"
}

#include <memory>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(executorch, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

void executorch::runtime::internal::vlogf(ET_UNUSED LogLevel level, et_timestamp_t timestamp, const char *filename,
                                          ET_UNUSED const char *function, size_t line, const char *format, va_list args)
{

#define MAX_LOG_MESSAGE_LENGTH 256
    char message[MAX_LOG_MESSAGE_LENGTH];
    snprintf(message, MAX_LOG_MESSAGE_LENGTH, format, args);
#undef MAX_LOG_MESSAGE_LENGTH

    using namespace executorch::runtime;

    switch (level)
    {
    case LogLevel::Info:
        LOG_INF("%s:%lu: %s", filename, line, message);
        break;
    case LogLevel::Debug:
        LOG_DBG("%s:%lu: %s", filename, line, message);
        break;
    case LogLevel::Error:
        LOG_WRN("%s:%lu: %s", filename, line, message);
        break;
    case LogLevel::Fatal:
        LOG_ERR("%s:%lu: %s", filename, line, message);
        break;
    default:
        LOG_ERR("Log message from ExecuTorch library with invalid level code at %s:%lu: %s", filename, line, message);
    }
}
