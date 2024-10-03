/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/kenning_protocol.h"

#ifndef __UNIT_TEST__
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/log.h"
#endif

LOG_MODULE_REGISTER(kenning_protocol, CONFIG_KENNING_PROTOCOL_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(KENNING_PROTOCOL);
const char *const MESSAGE_TYPE_STR[] = {MESSAGE_TYPES(GENERATE_STR)};

static uint8_t __attribute__((aligned(4))) g_message_buffer[MAX_MESSAGE_SIZE_BYTES + 2];

/**
 * Returns pointer to a message buffer with payload aligned to 4 bytes
 *
 * @returns pointer to a message buffer
 */
static message_t *get_message_buffer()
{
    // payload should be aligned to 4 bytes and as header (msg size and msg type)
    // are total 6 bytes, we need to shift msg buffer pointer 2 bytes
    return (message_t *)(g_message_buffer + 2);
}

status_t protocol_recv_msg(message_t **msg)
{
    status_t status = STATUS_OK;
    message_size_t msg_size = 0;
    MESSAGE_TYPE msg_type = MESSAGE_TYPE_OK;
    uint8_t data[4];

    RETURN_ERROR_IF_POINTER_INVALID(msg, KENNING_PROTOCOL_STATUS_INV_PTR);

    // read size of the message
    status = protocol_read_data(data, sizeof(message_size_t));
    CHECK_PROTOCOL_STATUS(status);

    msg_size = *((message_size_t *)data);

    if (msg_size > MAX_MESSAGE_SIZE_BYTES)
    {
        LOG_ERR("message too big: %u", msg_size);
        // read the rest of the data
        status = protocol_read_data(NULL, msg_size);

        return KENNING_PROTOCOL_STATUS_MSG_TOO_BIG;
    }
    if (msg_size < sizeof(message_type_t))
    {
        LOG_ERR("invalid message size: %u", msg_size);
        if (msg_size > 0)
        {
            // read the rest of the message
            status = protocol_read_data(NULL, msg_size);
        }
        return KENNING_PROTOCOL_STATUS_INV_MSG_SIZE;
    }

    // read type of the message
    status = protocol_read_data(data, sizeof(message_type_t));
    CHECK_PROTOCOL_STATUS(status);
    msg_type = *((message_type_t *)data);

    // get pointer to the message buffer
    *msg = get_message_buffer();
    RETURN_ERROR_IF_POINTER_INVALID(*msg, KENNING_PROTOCOL_STATUS_INV_PTR);

    (*msg)->message_size = msg_size;
    (*msg)->message_type = msg_type;

    // read the payload
    status = protocol_read_data((*msg)->payload, MESSAGE_SIZE_PAYLOAD(msg_size));
    if (STATUS_OK != status)
    {
        *msg = NULL;
    }
    CHECK_PROTOCOL_STATUS(status);

    return KENNING_PROTOCOL_STATUS_DATA_READY;
}

status_t protocol_send_msg(const message_t *msg)
{
    status_t status = STATUS_OK;

    RETURN_ERROR_IF_POINTER_INVALID(msg, KENNING_PROTOCOL_STATUS_INV_PTR);

    status = protocol_write_data((uint8_t *)msg, MESSAGE_SIZE_FULL(msg->message_size));

    CHECK_PROTOCOL_STATUS(status);

    return status;
}

status_t protocol_prepare_success_resp(message_t **response)
{
    RETURN_ERROR_IF_POINTER_INVALID(response, KENNING_PROTOCOL_STATUS_INV_PTR);

    *response = get_message_buffer();
    if (!IS_VALID_POINTER(*response))
    {
        return KENNING_PROTOCOL_STATUS_INTERNAL_ERROR;
    }
    (*response)->message_size = sizeof(message_type_t);
    (*response)->message_type = MESSAGE_TYPE_OK;
    return STATUS_OK;
}

status_t protocol_prepare_fail_resp(message_t **response)
{
    RETURN_ERROR_IF_POINTER_INVALID(response, KENNING_PROTOCOL_STATUS_INV_PTR);

    *response = get_message_buffer();
    if (!IS_VALID_POINTER(*response))
    {
        return KENNING_PROTOCOL_STATUS_INTERNAL_ERROR;
    }
    (*response)->message_size = sizeof(message_type_t);
    (*response)->message_type = MESSAGE_TYPE_ERROR;
    return STATUS_OK;
}
