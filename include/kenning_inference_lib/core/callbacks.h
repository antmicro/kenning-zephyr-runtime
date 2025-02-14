/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_CALLBACKS_H_
#define KENNING_INFERENCE_LIB_CORE_CALLBACKS_H_

#include "kenning_inference_lib/core/kenning_protocol.h"
#include "kenning_inference_lib/core/utils.h"

/**
 * Runtime custom error codes
 */
#define CALLBACKS_STATUSES(STATUS) STATUS(CALLBACKS_STATUS_INV_MSG_TYPE)

GENERATE_MODULE_STATUSES(CALLBACKS);

#define VALIDATE_HEADER(callback_message_type, hdr)                     \
    do                                                                  \
    {                                                                   \
        RETURN_ERROR_IF_POINTER_INVALID(hdr, CALLBACKS_STATUS_INV_PTR); \
        if ((callback_message_type) != (hdr)->message_type)             \
        {                                                               \
            return CALLBACKS_STATUS_INV_MSG_TYPE;                       \
        }                                                               \
    } while (0);

#define CHECK_STATUS_LOG(status, response, log_format, log_args...) \
    do                                                              \
    {                                                               \
        if (STATUS_OK != status)                                    \
        {                                                           \
            LOG_ERR(log_format, ##log_args);                        \
            return protocol_prepare_fail_resp(response);            \
        }                                                           \
        LOG_DBG(log_format, ##log_args);                            \
    } while (0);

#define PREPARE_RESPONSE(status)                               \
    do                                                         \
    {                                                          \
        status_t resp_status = STATUS_OK;                      \
        if (STATUS_OK == status)                               \
        {                                                      \
            resp_status = protocol_prepare_success_resp(resp); \
            RETURN_ON_ERROR(resp_status, resp_status);         \
        }                                                      \
        else                                                   \
        {                                                      \
            resp_status = protocol_prepare_fail_resp(resp);    \
            RETURN_ON_ERROR(resp_status, resp_status);         \
        }                                                      \
    } while (0);
/**
 * Type of callback function
 */
typedef status_t (*callback_ptr_t)(message_hdr_t *, resp_message_t *);

#if !defined(CONFIG_LLEXT) && !defined(CONFIG_ZTEST)
#define runtime_callback unsupported_callback
#endif // !defined(CONFIG_LLEXT) && !defined(CONFIG_ZTEST)

/**
 * List of callbacks for each message type
 */
#define CALLBACKS_TABLE(ENTRY)                               \
    /*    MessageType      Callback_function */              \
    ENTRY(MESSAGE_TYPE_OK, ok_callback)                      \
    ENTRY(MESSAGE_TYPE_ERROR, error_callback)                \
    ENTRY(MESSAGE_TYPE_DATA, data_callback)                  \
    ENTRY(MESSAGE_TYPE_MODEL, model_callback)                \
    ENTRY(MESSAGE_TYPE_PROCESS, process_callback)            \
    ENTRY(MESSAGE_TYPE_OUTPUT, output_callback)              \
    ENTRY(MESSAGE_TYPE_STATS, stats_callback)                \
    ENTRY(MESSAGE_TYPE_IOSPEC, iospec_callback)              \
    ENTRY(MESSAGE_TYPE_OPTIMIZERS, unsupported_callback)     \
    ENTRY(MESSAGE_TYPE_OPTIMIZE_MODEL, unsupported_callback) \
    ENTRY(MESSAGE_TYPE_RUNTIME, runtime_callback)

#define ENTRY(msg_type, callback_func) status_t callback_func(message_hdr_t *, resp_message_t *);
CALLBACKS_TABLE(ENTRY)
#undef ENTRY

#endif // KENNING_INFERENCE_LIB_CORE_CALLBACKS_H_
