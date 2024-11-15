/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/kenning_protocol.h"
#include "kenning_inference_lib/core/inference_server.h"
#include "kenning_inference_lib/core/loaders.h"

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(kenning_protocol, CONFIG_KENNING_PROTOCOL_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(KENNING_PROTOCOL);
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};

status_t protocol_recv_msg(message_hdr_t *hdr)
{
    struct msg_loader *ldr = NULL;
    status_t status = STATUS_OK;

    status = protocol_recv_msg_hdr(hdr);

    RETURN_ON_ERROR(status, status);

    if (hdr->message_type >= NUM_MESSAGE_TYPES)
    {
        protocol_read_data(NULL, MESSAGE_SIZE_PAYLOAD(hdr->message_size));
        LOG_ERR("Invalid message type: %u", hdr->message_type);
        return KENNING_PROTOCOL_STATUS_DATA_INV;
    }

    if (MESSAGE_SIZE_PAYLOAD(hdr->message_size) == 0)
    {
        return status;
    }

    for (int i = 0; i < LDR_TABLE_COUNT; i++)
    {
        struct msg_loader *n_ldr = g_ldr_tables[i][hdr->message_type];
        if (n_ldr != NULL)
        {
            ldr = n_ldr;
        }
    }

    if (ldr == NULL)
    {
        protocol_read_data(NULL, MESSAGE_SIZE_PAYLOAD(hdr->message_size));
        LOG_ERR("Couldn't find loader for %u", hdr->message_type);
        return KENNING_PROTOCOL_STATUS_DATA_INV;
    }

    status = protocol_recv_msg_content(ldr, MESSAGE_SIZE_PAYLOAD(hdr->message_size));
    return status;
}

status_t protocol_recv_msg_hdr(message_hdr_t *hdr)
{
    status_t status = STATUS_OK;

    status = protocol_read_data((uint8_t *)hdr, sizeof(message_hdr_t));
    CHECK_PROTOCOL_STATUS(status);
    return status;
}

status_t protocol_recv_msg_content(struct msg_loader *ldr, size_t n)
{
    static uint8_t __attribute__((aligned(4))) msg_recv_buffer[CONFIG_KENNING_MESSAGE_RECV_BUFFER_SIZE];
    status_t status = STATUS_OK;
    const int buffer_size = sizeof(msg_recv_buffer);

    ldr->reset(ldr, n);

    while (n)
    {
        int to_read = (buffer_size > n) ? n : buffer_size;
        status = protocol_read_data(msg_recv_buffer, to_read);
        CHECK_PROTOCOL_STATUS(status);
        if (ldr->save(ldr, msg_recv_buffer, to_read))
        {
            status = KENNING_PROTOCOL_STATUS_MSG_TOO_BIG;
        }
        n -= to_read;
    }
#undef BUFF_SIZE
    return status;
}
status_t protocol_send_msg(const resp_message_t *msg)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(msg, KENNING_PROTOCOL_STATUS_INV_PTR);

    status = protocol_write_data((uint8_t *)(&msg->hdr), sizeof(message_hdr_t));
    CHECK_PROTOCOL_STATUS(status);

    status = protocol_write_data(msg->payload, MESSAGE_SIZE_PAYLOAD(msg->hdr.message_size));
    CHECK_PROTOCOL_STATUS(status);

    return status;
}

status_t protocol_prepare_success_resp(resp_message_t *response)
{
    RETURN_ERROR_IF_POINTER_INVALID(response, KENNING_PROTOCOL_STATUS_INV_PTR);
    response->hdr.message_size = sizeof(message_type_t);
    response->hdr.message_type = MESSAGE_TYPE_OK;
    return STATUS_OK;
}

status_t protocol_prepare_fail_resp(resp_message_t *response)
{
    RETURN_ERROR_IF_POINTER_INVALID(response, KENNING_PROTOCOL_STATUS_INV_PTR);
    response->hdr.message_size = sizeof(message_type_t);
    response->hdr.message_type = MESSAGE_TYPE_ERROR;
    return STATUS_OK;
}
