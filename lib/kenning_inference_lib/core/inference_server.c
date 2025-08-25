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
extern struct k_heap llext_heap;

LOADER_TYPE g_msg_ldr_map[NUM_MESSAGE_TYPES] = PREPARE_MSG_LDR_MAP;

#if defined(CONFIG_LLEXT)

// Size in bytes of the field at the beginning of the runtime transmission/request payload,
// that denotes size of the runtime itself.
#define RUNTIME_SIZE_FIELD_SIZE 4

int reset_runtime(struct msg_loader *ldr)
{
    if (ldr->addr != NULL)
    {
        k_heap_free(&llext_heap, ldr->addr);
        ldr->addr = NULL;
    }
    ldr->written = 0;
    ldr->max_size = 0;
    struct llext *ext = llext_by_name("runtime");
    if (NULL != ext)
    {
        int status;
        status = llext_teardown(ext);
        RETURN_ON_ERROR_LOG(status, status, "LLEXT runtime teardown error: %d", status);

        status = llext_unload(&ext);
        RETURN_ON_ERROR_LOG(status, status, "LLEXT runtime unload error: %d", status);
    }

    return STATUS_OK;
}

status_t save_runtime(struct msg_loader *ldr, const uint8_t *src, size_t n)
{

    if (ldr->addr != NULL)
    {
        return buf_save(ldr, src, n);
    }
    // First 4 bytes of the payload in the runtime transmission/request denotes size of the runtime.
    // It is needed to poperly allocate memory on the llext heap, so on the first call to the 'save_runtime'
    // function after reset the buffer needs to be at least 4 bytes long, to properly read size of the
    // memory to allocate.
    if (n < RUNTIME_SIZE_FIELD_SIZE)
    {
        RETURN_LOG_ERROR(LOADERS_STATUS_ERROR,
                         "First buffer saved to the runtime loader after reset needs to be at least %d bytes long, "
                         "but it is %d bytes long.",
                         RUNTIME_SIZE_FIELD_SIZE, n);
    }
    size_t full_runtime_size = ((uint32_t *)src)[0];
    ldr->addr = k_heap_aligned_alloc(&llext_heap, 64, full_runtime_size, K_NO_WAIT);
    ldr->written = 0;

    if (ldr->addr == NULL)
    {
        ldr->max_size = 0;
        RETURN_LOG_ERROR(LOADERS_STATUS_NOT_ENOUGH_MEMORY, "Couldn't allocate ELF buffer of size %d on LLEXT heap.",
                         full_runtime_size);
    }

    ldr->max_size = full_runtime_size;
    if (n > RUNTIME_SIZE_FIELD_SIZE)
    {
        return buf_save(ldr, src + RUNTIME_SIZE_FIELD_SIZE, n - RUNTIME_SIZE_FIELD_SIZE);
    }
    return STATUS_OK;
}

status_t save_one_runtime(struct msg_loader *ldr, void *c)
{
    RETURN_LOG_ERROR(LOADERS_STATUS_ERROR, "LLEXT loaders do not support 'save_one' function.");
}

status_t prepare_llext_loader()
{
    static struct msg_loader msg_loader_llext = {.save = save_runtime,
                                                 .save_one = save_one_runtime,
                                                 .reset = reset_runtime,
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
        LOG_WRN("Invalid event.");
        return INFERENCE_SERVER_STATUS_INV_PTR;
    }
    status = protocol_listen(event, loader_picker);
    if (KENNING_PROTOCOL_STATUS_TIMEOUT == status)
    {
        LOG_WRN("Listening timeout.");
        return INFERENCE_SERVER_STATUS_TIMEOUT;
    }
    if (STATUS_OK != status)
    {
        LOG_ERR("Error listening: %d (%s)", status, get_status_str(status));
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
