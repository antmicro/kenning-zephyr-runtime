/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_INFERENCE_SERVER_H_
#define KENNING_INFERENCE_LIB_CORE_INFERENCE_SERVER_H_

#include "kenning_inference_lib/core/kenning_protocol.h"
#include "kenning_inference_lib/core/utils.h"

/**
 * Runtime custom error codes
 */
#define INFERENCE_SERVER_STATUSES(STATUS)

GENERATE_MODULE_STATUSES(INFERENCE_SERVER);

#define CHECK_INIT_STATUS_RET(status, log_format, log_args...) \
    do                                                         \
    {                                                          \
        if (STATUS_OK != status)                               \
        {                                                      \
            LOG_ERR(log_format, ##log_args);                   \
            return status;                                     \
        }                                                      \
        LOG_DBG(log_format, ##log_args);                       \
    } while (0);

/**
 * Initialize runtime server
 *
 * @returns status of initialization
 */
status_t init_server();

/**
 * Wait for incoming message
 *
 * @param msg pointer to the message
 *
 * @returns STATUS_OK if message was received
 */
status_t wait_for_message(message_hdr_t *hdr);

/**
 * Handle received message
 *
 * @param msg pointer to the message
 *
 * @returns status of message handling
 */
status_t handle_message(message_hdr_t *hdr);

#endif // KENNING_INFERENCE_LIB_CORE_INFERENCE_SERVER_H_
