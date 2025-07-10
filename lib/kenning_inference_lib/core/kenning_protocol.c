/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/kenning_protocol.h"
#include "kenning_inference_lib/core/loaders.h"
#include <zephyr/sys/util.h>

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(kenning_protocol, CONFIG_KENNING_PROTOCOL_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(KENNING_PROTOCOL);
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};
const char *const FLOW_CONTROL_STR[] = {FLOW_CONTROL_VALUES(GENERATE_STR)};

LOADER_TYPE g_msg_ldr_map[NUM_MESSAGE_TYPES] = PREPARE_MSG_LDR_MAP;

status_t protocol_transmit(const resp_message_t *msg)
{
    status_t status = STATUS_OK;
    unsigned int message_count =
        ((msg->hdr.payload_size - 1) / CONFIG_KENNING_PROTOCOL_MAX_OUTGOING_MESSAGE_SIZE) + 1; // Rounding
                                                                                               // up.
    payload_size_t bytes_sent = 0;
    for (int i = 0; i < message_count; i++)
    {
        payload_size_t message_payload_size =
            MIN(msg->hdr.payload_size - bytes_sent, CONFIG_KENNING_PROTOCOL_MAX_OUTGOING_MESSAGE_SIZE);
        resp_message_t message;

        message.hdr.flags = msg->hdr.flags;
        message.hdr.message_type = msg->hdr.message_type;
        message.hdr.flow_control_flags = FLOW_CONTROL_TRANSMISSION;
        message.hdr.payload_size = message_payload_size;
        message.hdr.flags.general_purpose_flags.first = (i == 0) ? 1 : 0;
        message.hdr.flags.general_purpose_flags.last = (i == (message_count - 1)) ? 1 : 0;
        message.hdr.flags.general_purpose_flags.has_payload = 1;
        message.hdr.flags.general_purpose_flags.is_host_message = 0;
        message.payload = msg->payload + bytes_sent;
        status = protocol_send_msg(&message);
        CHECK_PROTOCOL_STATUS(status);
        bytes_sent += message_payload_size;
    }
    return status;
}

status_t protocol_recv_msg(message_hdr_t *hdr)
{
    struct msg_loader *ldr = NULL;
    status_t status = STATUS_OK;

    status = protocol_recv_msg_hdr(hdr);

    RETURN_ON_ERROR(status, status);

    if (hdr->message_type >= NUM_MESSAGE_TYPES)
    {
        protocol_read_data(NULL, hdr->payload_size);
        LOG_ERR("Invalid message type: %u", hdr->message_type);
    }

    if (hdr->payload_size == 0)
    {
        return status;
    }

    for (int i = 0; i < LDR_TABLE_COUNT; i++)
    {
        struct msg_loader *n_ldr = g_ldr_tables[i][MSGT_TO_LDRT(hdr->message_type)];
        if (n_ldr != NULL)
        {
            ldr = n_ldr;
        }
    }

    if (ldr == NULL)
    {
        protocol_read_data(NULL, hdr->payload_size);
        LOG_ERR("Couldn't find loader for %u", hdr->message_type);
        return KENNING_PROTOCOL_STATUS_DATA_INV;
    }

    status = protocol_recv_msg_content(ldr, hdr->payload_size);
    return status;
}

status_t protocol_recv_msg_hdr(message_hdr_t *hdr)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(hdr, KENNING_PROTOCOL_STATUS_INV_PTR);

    status = protocol_read_data((uint8_t *)hdr, sizeof(message_hdr_t));
    CHECK_PROTOCOL_STATUS(status);
    return status;
}

status_t protocol_recv_msg_content(struct msg_loader *ldr, size_t n)
{
    static uint8_t __attribute__((aligned(4))) msg_recv_buffer[CONFIG_KENNING_MESSAGE_RECV_BUFFER_SIZE];
    status_t status = STATUS_OK;
    const int buffer_size = sizeof(msg_recv_buffer);

    RETURN_ERROR_IF_POINTER_INVALID(ldr, KENNING_PROTOCOL_STATUS_INV_PTR);

    status = ldr->reset(ldr, n);
    RETURN_ON_ERROR(status, KENNING_PROTOCOL_STATUS_INTERNAL_ERROR);
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
    return status;
}
status_t protocol_send_msg(const resp_message_t *msg)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(msg, KENNING_PROTOCOL_STATUS_INV_PTR);

    status = protocol_write_data((uint8_t *)(&msg->hdr), sizeof(message_hdr_t));
    CHECK_PROTOCOL_STATUS(status);

    status = protocol_write_data(msg->payload, msg->hdr.payload_size);
    CHECK_PROTOCOL_STATUS(status);

    return status;
}

status_t protocol_prepare_success_resp(resp_message_t *response)
{
    RETURN_ERROR_IF_POINTER_INVALID(response, KENNING_PROTOCOL_STATUS_INV_PTR);
    response->hdr.payload_size = 0;
    response->hdr.message_type = MESSAGE_TYPE_OK;
    return STATUS_OK;
}

status_t protocol_prepare_fail_resp(resp_message_t *response)
{
    RETURN_ERROR_IF_POINTER_INVALID(response, KENNING_PROTOCOL_STATUS_INV_PTR);
    response->hdr.payload_size = 0;
    response->hdr.message_type = MESSAGE_TYPE_ERROR;
    return STATUS_OK;
}
