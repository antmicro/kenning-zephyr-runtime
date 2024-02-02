/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_
#define KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_

#include "kenning_inference_lib/core/protocol.h"

#define CHECK_PROTOCOL_STATUS(status)                                       \
    if (PROTOCOL_STATUS_TIMEOUT == (status))                                \
    {                                                                       \
        return KENNING_PROTOCOL_STATUS_TIMEOUT;                             \
    }                                                                       \
    if (STATUS_OK != (status))                                              \
    {                                                                       \
        LOG_ERR("Protocol error: %d (%s)", status, get_status_str(status)); \
        return KENNING_PROTOCOL_STATUS_CLIENT_DISCONNECTED;                 \
    }

#define MAX_MESSAGE_SIZE_BYTES (64UL * 1024UL) // 64 KB

#define MESSAGE_SIZE_PAYLOAD(msg_size) ((msg_size) - sizeof(message_type_t))
#define MESSAGE_SIZE_FULL(msg_size) (sizeof(message_t) + MESSAGE_SIZE_PAYLOAD(msg_size))

/**
 * An enum that describes message type
 */
#define MESSAGE_TYPES(TYPE)    \
    TYPE(MESSAGE_TYPE_OK)      \
    TYPE(MESSAGE_TYPE_ERROR)   \
    TYPE(MESSAGE_TYPE_DATA)    \
    TYPE(MESSAGE_TYPE_MODEL)   \
    TYPE(MESSAGE_TYPE_PROCESS) \
    TYPE(MESSAGE_TYPE_OUTPUT)  \
    TYPE(MESSAGE_TYPE_STATS)   \
    TYPE(MESSAGE_TYPE_IOSPEC)  \
    TYPE(NUM_MESSAGE_TYPES)

typedef enum
{
    MESSAGE_TYPES(GENERATE_ENUM)
} MESSAGE_TYPE;

/**
 * Protocol custom error codes
 */
#define KENNING_PROTOCOL_STATUSES(STATUS)               \
    STATUS(KENNING_PROTOCOL_STATUS_CLIENT_CONNECTED)    \
    STATUS(KENNING_PROTOCOL_STATUS_CLIENT_DISCONNECTED) \
    STATUS(KENNING_PROTOCOL_STATUS_CLIENT_IGNORED)      \
    STATUS(KENNING_PROTOCOL_STATUS_DATA_READY)          \
    STATUS(KENNING_PROTOCOL_STATUS_DATA_INV)            \
    STATUS(KENNING_PROTOCOL_STATUS_INTERNAL_ERROR)      \
    STATUS(KENNING_PROTOCOL_STATUS_MSG_TOO_BIG)         \
    STATUS(KENNING_PROTOCOL_STATUS_INV_MSG_SIZE)

GENERATE_MODULE_STATUSES(KENNING_PROTOCOL);

typedef uint32_t message_size_t;
typedef uint16_t message_type_t;

/**
 * A struct that contains all parameters describing single message
 */
typedef struct __attribute__((packed))
{
    message_size_t message_size;
    message_type_t message_type;
    uint8_t payload[0];
} message_t;

/**
 * Waits for a message to be received
 *
 * @param msg received message
 *
 * @returns status of the protocol
 */
status_t protocol_recv_msg(message_t **msg);
/**
 * Sends given message
 *
 * @param msg message to be sent
 *
 * @returns status of the protocol
 */
status_t protocol_send_msg(const message_t *msg);
/**
 * Create a message that indicates an successful action. The message type is OK
 * and the paylaod is empty
 *
 * @param response created message
 *
 * @returns status of the protocol
 */
status_t protocol_prepare_success_resp(message_t **response);
/**
 * Create a message that indicates an error. The message type is ERROR and the
 * payload is empty
 *
 * @param response created message
 *
 * @returns status of the protocol
 */
status_t protocol_prepare_fail_resp(message_t **response);

#endif // KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_
