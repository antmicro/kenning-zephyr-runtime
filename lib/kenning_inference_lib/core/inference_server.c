/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/inference_server.h"
#include "kenning_inference_lib/core/callbacks.h"
#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/model.h"
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

struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_MESSAGE_TYPES];
status_t init_server()
{
    status_t status = STATUS_OK;

    // initialize protocol
    status = protocol_init();
    CHECK_INIT_STATUS_RET(status, "protocol_init returned 0x%x (%s)", status, get_status_str(status));

// initialize model if LLEXT is not used
#if !defined(CONFIG_LLEXT)
    status = model_init();
    CHECK_INIT_STATUS_RET(status, "model_init returned 0x%x (%s)", status, get_status_str(status));
#endif

    prepare_main_ldr_table();

    LOG_INF("Inference server started");
    return STATUS_OK;
}

status_t prepare_main_ldr_table()
{
    status_t status = STATUS_OK;

    static struct msg_loader msg_loader_iospec = MSG_LOADER_BUF((uint8_t *)(&g_model_struct), sizeof(MlModel));
    g_ldr_tables[0][MESSAGE_TYPE_IOSPEC] = &msg_loader_iospec;
    return STATUS_OK;
}

status_t wait_for_message(message_hdr_t *hdr)
{
    status_t status = STATUS_OK;
    LOG_DBG("Waiting for message");

    status = protocol_recv_msg(hdr);
    if (PROTOCOL_STATUS_TIMEOUT == status)
    {
        LOG_WRN("Receive message timeout");
        return INFERENCE_SERVER_STATUS_TIMEOUT;
    }
    if (STATUS_OK != status)
    {
        LOG_ERR("Error receiving message: %d (%s)", status, get_status_str(status));
        return INFERENCE_SERVER_STATUS_ERROR;
    }
    const char *message_type_str =
        hdr->message_type < NUM_MESSAGE_TYPES ? MESSAGE_TYPE_STR[hdr->message_type] : "UNKNOWN";

    LOG_DBG("Received message. Size: %d, type: %d (%s)", hdr->message_size, hdr->message_type, message_type_str);

    return STATUS_OK;
}

status_t handle_message(message_hdr_t *hdr)
{
    static uint8_t __attribute__((aligned(4))) resp_payload[CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE];
    status_t status = STATUS_OK;
    resp_message_t resp = {.payload = resp_payload};

    if (!IS_VALID_POINTER(hdr))
    {
        LOG_WRN("Invalid message.");
        return INFERENCE_SERVER_STATUS_INV_PTR;
    }

    if (hdr->message_type >= NUM_MESSAGE_TYPES)
    {
        LOG_WRN("Unknown message type. Ignoring.");
        return INFERENCE_SERVER_STATUS_INV_ARG;
    }

    status = g_msg_callback[hdr->message_type](hdr, &resp);
    if (STATUS_OK != status)
    {
        LOG_ERR("Runtime error: 0x%x (%s)", status, get_status_str(status));
        return status;
    }

    LOG_DBG("Sending response. Size: %d, type: %d (%s)", resp.hdr.message_size, resp.hdr.message_type,
            MESSAGE_TYPE_STR[resp.hdr.message_type]);
    status = protocol_send_msg(&resp);
    if (STATUS_OK != status)
    {
        LOG_ERR("Error sending message: 0x%x (%s)", status, get_status_str(status));
    }
    else
    {
        LOG_DBG("Response sent");
    }

    return status;
}
