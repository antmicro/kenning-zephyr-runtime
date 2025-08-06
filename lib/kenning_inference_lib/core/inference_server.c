/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
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
extern const char *const FLOW_CONTROL_STR[];
extern const callback_ptr_t g_msg_callback[];

LOADER_TYPE g_msg_ldr_map[NUM_MESSAGE_TYPES] = PREPARE_MSG_LDR_MAP;

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

struct msg_loader *loader_picker(message_type_t message_type)
{
    struct msg_loader *ldr = NULL;
    for (int i = 0; i < LDR_TABLE_COUNT; i++)
    {
        struct msg_loader *n_ldr = g_ldr_tables[i][MSGT_TO_LDRT(message_type)];
        if (n_ldr != NULL)
        {
            ldr = n_ldr;
        }
    }
    return ldr;
}

status_t wait_for_protocol_event(protocol_event_t *event)
{
    status_t status = STATUS_OK;
    if (!IS_VALID_POINTER(event))
    {
        LOG_WRN("Invalid message.");
        return INFERENCE_SERVER_STATUS_INV_PTR;
    }
    status = protocol_listen(event, loader_picker);
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
        event->message_type < NUM_MESSAGE_TYPES ? MESSAGE_TYPE_STR[event->message_type] : "UNKNOWN";

    LOG_DBG("Received event. Size: %d, type: %d (%s), flags: 0x%04x", event->payload.size, event->message_type,
            message_type_str, event->flags.raw_bytes);

    return STATUS_OK;
}

status_t handle_protocol_event(protocol_event_t *event)
{
    static uint8_t __attribute__((aligned(4))) resp_payload[CONFIG_KENNING_RESPONSE_PAYLOAD_SIZE];
    status_t status = STATUS_OK;
    if (!IS_VALID_POINTER(event))
    {
        LOG_WRN("Invalid event.");
        return INFERENCE_SERVER_STATUS_INV_PTR;
    }
    protocol_event_t resp = {.payload.size = 0, .payload.raw_bytes = resp_payload, .message_type = event->message_type};
    resp.flags.general_purpose_flags.is_zephyr = 1;

    status = g_msg_callback[event->message_type](event, &resp.payload);
    if (STATUS_OK != status)
    {
        LOG_ERR("Runtime error: 0x%x (%s)", status, get_status_str(status));
        resp.flags.general_purpose_flags.fail = 1;
    }
    else
    {
        resp.flags.general_purpose_flags.success = 1;
    }
    if (event->is_request)
    {
        const char *message_type_str =
            resp.message_type < NUM_MESSAGE_TYPES ? MESSAGE_TYPE_STR[resp.message_type] : "UNKNOWN";
        LOG_DBG("Sending response. Size: %d, type: %d (%s), flags: 0x%04x", resp.payload.size, resp.message_type,
                message_type_str, resp.flags.raw_bytes);

        status = protocol_transmit(&resp);

        if (STATUS_OK != status)
        {
            LOG_ERR("Error sending message: 0x%x (%s)", status, get_status_str(status));
        }
        else
        {
            LOG_DBG("Response sent");
        }
    }
    return status;
}
