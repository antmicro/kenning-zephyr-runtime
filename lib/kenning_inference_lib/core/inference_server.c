/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/inference_server.h"
#include "kenning_inference_lib/core/callbacks.h"
#include "kenning_inference_lib/core/protocol.h"

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(inference_server, CONFIG_INFERENCE_SERVER_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(INFERENCE_SERVER);

extern const char *const MESSAGE_TYPE_STR[];
extern const callback_ptr_t g_msg_callback[];

status_t init_server()
{
    status_t status = STATUS_OK;

    // initialize protocol
    status = protocol_init();
    CHECK_INIT_STATUS_RET(status, "protocol_init returned 0x%x (%s)", status, get_status_str(status));

    LOG_INF("Inference server started");
    return STATUS_OK;
}

status_t wait_for_message(message_t **msg)
{
    status_t status = STATUS_OK;

    if (!IS_VALID_POINTER(msg))
    {
        return INFERENCE_SERVER_STATUS_INV_PTR;
    }
    LOG_DBG("Waiting for message");

    status = protocol_recv_msg(msg);
    if (PROTOCOL_STATUS_TIMEOUT == status)
    {
        LOG_WRN("Receive message timeout");
        return INFERENCE_SERVER_STATUS_TIMEOUT;
    }
    if (KENNING_PROTOCOL_STATUS_DATA_READY != status)
    {
        LOG_ERR("Error receiving message: %d (%s)", status, get_status_str(status));
        return INFERENCE_SERVER_STATUS_ERROR;
    }
    const char *message_type_str =
        (*msg)->message_type < NUM_MESSAGE_TYPES ? MESSAGE_TYPE_STR[(*msg)->message_type] : "UNKNOWN";

    LOG_DBG("Received message. Size: %d, type: %d (%s)", (*msg)->message_size, (*msg)->message_type, message_type_str);

    return STATUS_OK;
}

status_t handle_message(message_t *msg)
{
    status_t status = STATUS_OK;

    if (!IS_VALID_POINTER(msg))
    {
        LOG_WRN("Invalid message.");
        return INFERENCE_SERVER_STATUS_INV_PTR;
    }

    if (msg->message_type >= NUM_MESSAGE_TYPES)
    {
        LOG_WRN("Unknown message type. Ignoring.");
        return INFERENCE_SERVER_STATUS_INV_ARG;
    }

    status = g_msg_callback[msg->message_type](&msg);
    if (STATUS_OK != status)
    {
        LOG_ERR("Runtime error: 0x%x (%s)", status, get_status_str(status));
    }
    if (NULL != msg)
    {
        LOG_DBG("Sending reponse. Size: %d, type: %d (%s)", msg->message_size, msg->message_type,
                MESSAGE_TYPE_STR[msg->message_type]);
        status = protocol_send_msg(msg);
        if (STATUS_OK != status)
        {
            LOG_ERR("Error sending message: 0x%x (%s)", status, get_status_str(status));
        }
        LOG_DBG("Response sent");
    }
    return status;
}
