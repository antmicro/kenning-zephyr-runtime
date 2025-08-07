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

/*
 Flag informing us whether the protocol is just now in progress of sending a message.
 If it is, we cannot send any other messages, because they could end up being sent
 between header and payload of the message that is already being sent.
*/
static bool protocol_busy = false;

/**
 * Receives a single message header.
 *
 * @param header pointer to which the received header will be saved.
 *
 * @returns status of the protocol
 */
status_t receive_message_header(message_hdr_t *header)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(header, KENNING_PROTOCOL_STATUS_INV_PTR);

    status = protocol_read_data((uint8_t *)header, sizeof(message_hdr_t));
    CHECK_PROTOCOL_STATUS(status);
    return status;
}

/**
 * Receives a single message payload of a given size,
 *
 * @param ldr loader for the payload.
 * @param n size of the payload in bytes.
 *
 * @returns status of the protocol
 */
status_t receive_message_payload(struct msg_loader *ldr, size_t n)
{
    static uint8_t __attribute__((aligned(4))) msg_recv_buffer[CONFIG_KENNING_MESSAGE_RECV_BUFFER_SIZE];
    status_t status = STATUS_OK;
    const int buffer_size = sizeof(msg_recv_buffer);

    RETURN_ERROR_IF_POINTER_INVALID(ldr, KENNING_PROTOCOL_STATUS_INV_PTR);

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

/**
 * Receives a series of messages, assuming that the header of the first message
 * has already been received and passed as argument.
 *
 * @param ldr loader for message payload.
 * @param header received header of the first message.
 *
 * @returns status of the protocol
 */
status_t receive_messages(struct msg_loader *ldr, message_hdr_t *header)
{
    status_t status = STATUS_OK;
    message_type_t message_type = header->message_type;
    flow_control_flags_t flow_control_flags = header->flow_control_flags;
    for (int i = 0;; i++)
    {
        // Header of the first message should already be received before this function is called.
        if (i != 0)
        {
            status = receive_message_header(header);
            RETURN_ON_ERROR(status, status);
            if (header->message_type != message_type)
            {
                protocol_read_data(NULL, header->payload_size);
                return KENNING_PROTOCOL_STATUS_INVALID_MESSAGE_TYPE;
            }
            if (header->flow_control_flags != flow_control_flags)
            {
                protocol_read_data(NULL, header->payload_size);
                return KENNING_PROTOCOL_STATUS_FLOW_CONTROL_ERROR;
            }
        }
        if (header->flags.general_purpose_flags.has_payload)
        {
            status = receive_message_payload(ldr, header->payload_size);
            RETURN_ON_ERROR(status, status);
        }
        if (header->flags.general_purpose_flags.last)
        {
            return status;
        }
    }
}

/**
 * Sends given message
 *
 * @param msg message to be sent
 *
 * @returns status of the protocol
 */
status_t send_message(const outgoing_message_t *msg)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(msg, KENNING_PROTOCOL_STATUS_INV_PTR);

    status = protocol_write_data((uint8_t *)(&msg->hdr), sizeof(message_hdr_t));
    CHECK_PROTOCOL_STATUS(status);

    if (msg->hdr.flags.general_purpose_flags.has_payload)
    {
        status = protocol_write_data(msg->payload, msg->hdr.payload_size);
        CHECK_PROTOCOL_STATUS(status);
    }
    return status;
}

status_t protocol_transmit(const protocol_event_t *event)
{
    if (protocol_busy)
    {
        LOG_DBG("Attempted to start a transmission, while a message was being sent.");
        return KENNING_PROTOCOL_STATUS_BUSY;
    }
    status_t status = STATUS_OK;
    RETURN_ERROR_IF_POINTER_INVALID(event, KENNING_PROTOCOL_STATUS_INV_PTR);
    bool has_payload = event->payload.size > 0;
    unsigned int message_count =
        has_payload ? ((event->payload.size - 1) / CONFIG_KENNING_PROTOCOL_MAX_OUTGOING_MESSAGE_SIZE) + 1
                    : 1; // Rounding
                         // up.
    payload_size_t bytes_sent = 0;
    for (int i = 0; i < message_count; i++)
    {
        payload_size_t message_payload_size =
            MIN(event->payload.size - bytes_sent, CONFIG_KENNING_PROTOCOL_MAX_OUTGOING_MESSAGE_SIZE);
        outgoing_message_t message;
        message.hdr.flags = event->flags;
        message.hdr.message_type = event->message_type;
        message.hdr.flow_control_flags = FLOW_CONTROL_TRANSMISSION;
        message.hdr.payload_size = message_payload_size;
        message.hdr.flags.general_purpose_flags.first = (i == 0) ? 1 : 0;
        message.hdr.flags.general_purpose_flags.last = (i == (message_count - 1)) ? 1 : 0;
        message.hdr.flags.general_purpose_flags.has_payload = has_payload;
        message.hdr.flags.general_purpose_flags.is_host_message = 0;
        message.payload = has_payload ? event->payload.raw_bytes + bytes_sent : NULL;
        protocol_busy = true;
        status = send_message(&message);
        protocol_busy = false;
        RETURN_ON_ERROR(status, status);
        bytes_sent += message_payload_size;
    }
    return status;
}

status_t protocol_listen(protocol_event_t *event, struct msg_loader *(*loader_callback)(message_type_t))
{
    status_t status = STATUS_OK;
    RETURN_ERROR_IF_POINTER_INVALID(event, KENNING_PROTOCOL_STATUS_INV_PTR);
    RETURN_ERROR_IF_POINTER_INVALID(loader_callback, KENNING_PROTOCOL_STATUS_INV_PTR);
    message_hdr_t header;
    status = receive_message_header(&header);
    RETURN_ON_ERROR(status, status);

    if (header.flow_control_flags != FLOW_CONTROL_TRANSMISSION && header.flow_control_flags != FLOW_CONTROL_REQUEST)
    {
        const char *flow_control_str = header.flow_control_flags < NUM_FLOW_CONTROL_VALUES
                                           ? FLOW_CONTROL_STR[header.flow_control_flags]
                                           : "UNKNOWN";
        LOG_ERR("Invalid message at this time: %d (%s)", header.flow_control_flags, flow_control_str);
        protocol_read_data(NULL, header.payload_size);
        return KENNING_PROTOCOL_STATUS_FLOW_CONTROL_ERROR;
    }
    if (header.flags.general_purpose_flags.first == 0)
    {
        LOG_ERR("First message received did not have the 'first' flag set");
        protocol_read_data(NULL, header.payload_size);
        return KENNING_PROTOCOL_STATUS_FLOW_CONTROL_ERROR;
    }

    event->message_type = header.message_type;
    event->flags = header.flags;
    event->is_request = (header.flow_control_flags == FLOW_CONTROL_REQUEST);

    if (header.message_type >= NUM_MESSAGE_TYPES)
    {
        LOG_ERR("Invalid message type: %u", header.message_type);
        protocol_read_data(NULL, header.payload_size);
        return KENNING_PROTOCOL_STATUS_INVALID_MESSAGE_TYPE;
    }

    struct msg_loader *ldr = loader_callback(header.message_type);

    event->payload.loader = ldr;
    if (header.flags.general_purpose_flags.has_payload)
    {
        // Passing first message size, because we don't know size of the whole transmission.
        status_t loader_status = ldr->reset(ldr, header.payload_size);
        if (loader_status)
        {
            LOG_ERR("Loader reset failure, status: %d", loader_status);
            protocol_read_data(NULL, header.payload_size);
            return loader_status;
        }
        status = receive_messages(ldr, &header);
    }
    event->payload.size = ldr->written;
    return status;
}
