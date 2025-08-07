/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Implemenation of Zephyr Logging Backend API for sending logs back to the client over Kenning Protocol.
 */

#ifndef KENNING_INFERENCE_LIB_CORE_LOGGER_H_
#define KENNING_INFERENCE_LIB_CORE_LOGGER_H_

#ifdef CONFIG_KENNING_SEND_LOGS
#include "kenning_inference_lib/core/utils.h"
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_backend.h>
#include <zephyr/logging/log_backend_std.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/logging/log_output.h>

#define LOGGER_STATUSES(STATUS)

GENERATE_MODULE_STATUSES(LOGGER);

/**
 * Starts sending back to Kenning all logs using Kenning Protocol, except the ones generated in the process of sending
 * logs.
 *
 * @returns STATUS_OK if the initialization was successfull
 */
status_t logger_start();

/**
 * Stops sending logs to Kenning
 *
 * @returns STATUS_OK if the de-initialization was successfull
 */
status_t logger_stop();

#endif // CONFIG_KENNING_SEND_LOGS

#endif // KENNING_INFERENCE_LIB_CORE_LOGGERS_H_
