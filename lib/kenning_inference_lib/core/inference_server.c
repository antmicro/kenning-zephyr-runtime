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
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

#if defined(CONFIG_LLEXT)
#include "zephyr/llext/llext.h"
#endif // defined(CONFIG_LLEXT)

LOG_MODULE_REGISTER(inference_server, CONFIG_INFERENCE_SERVER_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(INFERENCE_SERVER);

extern const char *const MESSAGE_TYPE_STR[];
extern const callback_ptr_t g_msg_callback[];

#if defined(CONFIG_LLEXT)

int reset_runtime_alloc(struct msg_loader *ldr, size_t n)
{
    int status;
    extern struct k_heap llext_heap;
    struct llext *ext = llext_by_name("runtime");
    if (NULL != ext)
    {
        status = llext_teardown(ext);
        RETURN_ON_ERROR_LOG(status, status, "LLEXT runtime teardown error: %d", status);

        status = llext_unload(&ext);
        RETURN_ON_ERROR_LOG(status, status, "LLEXT runtime unload error: %d", status);
    }
    k_heap_free(&llext_heap, ldr->addr);
    ldr->addr = NULL;
    ldr->addr = k_heap_aligned_alloc(&llext_heap, 64, n, K_NO_WAIT);
    ldr->written = 0;

    if (ldr->addr == NULL)
    {
        ldr->max_size = 0;
        LOG_ERR("Couldn't allocate ELF buffer on LLEXT heap");
        return -1;
    }

    ldr->max_size = n;
    return 0;
}

status_t prepare_llext_loader()
{
    static struct msg_loader msg_loader_llext = {.save = buf_save,
                                                 .save_one = buf_save_one,
                                                 .reset = reset_runtime_alloc,
                                                 .written = 0,
                                                 .max_size = 0,
                                                 .addr = NULL};
    g_ldr_tables[0][LOADER_TYPE_RUNTIME] = &msg_loader_llext;

    return STATUS_OK;
}

#endif // defined(CONFIG_LLEXT)

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
#else
    prepare_llext_loader();
#endif
    LOG_INF("Inference server started");
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
