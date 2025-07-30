/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_INFERENCE_SERVER_H_
#define KENNING_INFERENCE_LIB_CORE_INFERENCE_SERVER_H_

#include "kenning_inference_lib/core/kenning_protocol.h"
#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/utils.h"

#define PREPARE_MSG_LDR_MAP                                     \
    {                                                           \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_PING*/              \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_STATUS*/            \
        LOADER_TYPE_DATA,    /*MESSAGE_TYPE_DATA*/              \
        LOADER_TYPE_MODEL,   /*MESSAGE_TYPE_MODEL*/             \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_PROCESS*/           \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_OUTPUT*/            \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_STATS*/             \
        LOADER_TYPE_IOSPEC,  /*MESSAGE_TYPE_IOSPEC*/            \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_OPTIMIZERS*/        \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_OPTIMIZE_MODEL*/    \
        LOADER_TYPE_RUNTIME, /*MESSAGE_TYPE_RUNTIME*/           \
        LOADER_TYPE_NONE,    /*MESSAGE_TYPE_UNOPTIMIZED_MODEL*/ \
    }

extern LOADER_TYPE g_msg_ldr_map[NUM_MESSAGE_TYPES];

#define MSGT_TO_LDRT(msg) g_msg_ldr_map[(msg)]

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
 * Wait for incoming transmission or request.
 *
 * @param msg pointer to the protocol event received
 *
 * @returns STATUS_OK if event was received
 */
status_t wait_for_protocol_event(protocol_event_t *event);

/**
 * Handle received transmission or request.
 *
 * @param msg pointer to the protocol event to handle
 *
 * @returns status of event handling
 */
status_t handle_protocol_event(protocol_event_t *event);

#endif // KENNING_INFERENCE_LIB_CORE_INFERENCE_SERVER_H_
