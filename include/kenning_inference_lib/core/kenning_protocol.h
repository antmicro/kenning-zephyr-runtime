/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_
#define KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_

#include <stdbool.h>

#include "kenning_inference_lib/core/loaders.h"
#include "kenning_inference_lib/core/protocol.h"

#define CHECK_PROTOCOL_STATUS(status)                                       \
    if (PROTOCOL_STATUS_TIMEOUT == (status))                                \
    {                                                                       \
        return KENNING_PROTOCOL_STATUS_TIMEOUT;                             \
    }                                                                       \
    if (STATUS_OK != (status))                                              \
    {                                                                       \
        LOG_ERR("Protocol error: %d (%s)", status, get_status_str(status)); \
        return KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR;                   \
    }

#define MESSAGE_SIZE_FULL(payload_size) (sizeof(message_hdr_t) + (payload_size))
#define RESP_MESSAGE_BUFFER_SIZE 128

/**
 * An enum that describes message type
 */
#define MESSAGE_TYPES(TYPE)              \
    TYPE(MESSAGE_TYPE_PING)              \
    TYPE(MESSAGE_TYPE_STATUS)            \
    TYPE(MESSAGE_TYPE_DATA)              \
    TYPE(MESSAGE_TYPE_MODEL)             \
    TYPE(MESSAGE_TYPE_PROCESS)           \
    TYPE(MESSAGE_TYPE_OUTPUT)            \
    TYPE(MESSAGE_TYPE_STATS)             \
    TYPE(MESSAGE_TYPE_IOSPEC)            \
    TYPE(MESSAGE_TYPE_OPTIMIZERS)        \
    TYPE(MESSAGE_TYPE_OPTIMIZE_MODEL)    \
    TYPE(MESSAGE_TYPE_RUNTIME)           \
    TYPE(MESSAGE_TYPE_UNOPTIMIZED_MODEL) \
    TYPE(NUM_MESSAGE_TYPES)

#define FLOW_CONTROL_VALUES(TYPE)         \
    TYPE(FLOW_CONTROL_REQUEST)            \
    TYPE(FLOW_CONTROL_REQUEST_RETRANSMIT) \
    TYPE(FLOW_CONTROL_ACKNOWLEDGE)        \
    TYPE(FLOW_CONTROL_TRANSMISSION)       \
    TYPE(NUM_FLOW_CONTROL_VALUES)

typedef enum
{
    MESSAGE_TYPES(GENERATE_ENUM)
} MESSAGE_TYPE;

typedef enum
{
    FLOW_CONTROL_VALUES(GENERATE_ENUM)
} FLOW_CONTROL_VALUE;

#define PREPARE_MSG_LDR_MAP                                         \
    {                                                               \
        LOADER_TYPE_NONE,        /*MESSAGE_TYPE_PING*/              \
            LOADER_TYPE_NONE,    /*MESSAGE_TYPE_STATUS*/            \
            LOADER_TYPE_DATA,    /*MESSAGE_TYPE_DATA*/              \
            LOADER_TYPE_MODEL,   /*MESSAGE_TYPE_MODEL*/             \
            LOADER_TYPE_NONE,    /*MESSAGE_TYPE_PROCESS*/           \
            LOADER_TYPE_NONE,    /*MESSAGE_TYPE_OUTPUT*/            \
            LOADER_TYPE_NONE,    /*MESSAGE_TYPE_STATS*/             \
            LOADER_TYPE_IOSPEC,  /*MESSAGE_TYPE_IOSPEC*/            \
            LOADER_TYPE_NONE,    /*MESSAGE_TYPE_OPTIMIZERS*/        \
            LOADER_TYPE_NONE,    /*MESSAGE_TYPE_OPTIMIZE_MODEL*/    \
            LOADER_TYPE_RUNTIME, /*MESSAGE_TYPE_RUNTIME*/           \
            LOADER_TYPE_NONE,    /*MESSAGE_TYPE_UNOPTIMIZED_MODEL*/ \
    }

extern LOADER_TYPE g_msg_ldr_map[NUM_MESSAGE_TYPES];

#define MSGT_TO_LDRT(msg) g_msg_ldr_map[(msg)]

/**
 * Protocol custom error codes
 */
#define KENNING_PROTOCOL_STATUSES(STATUS)                \
    STATUS(KENNING_PROTOCOL_STATUS_LOWER_LAYER_ERROR)    \
    STATUS(KENNING_PROTOCOL_STATUS_INVALID_MESSAGE_TYPE) \
    STATUS(KENNING_PROTOCOL_STATUS_EVENT_DENIED)         \
    STATUS(KENNING_PROTOCOL_STATUS_MSG_TOO_BIG)          \
    STATUS(KENNING_PROTOCOL_STATUS_FLOW_CONTROL_ERROR)

GENERATE_MODULE_STATUSES(KENNING_PROTOCOL);

typedef uint8_t message_type_t;
typedef uint8_t flow_control_flags_t;
typedef uint8_t checksum_t;
typedef uint32_t payload_size_t;

/**
 * Some message flags are common for all messages, while some are specific to message type.
 * We are using the flags_t union to reflect that.
 */
typedef union
{
    /**
     * Struct with general flags only
     */
    struct __attribute__((packed))
    {
        uint16_t success : 1;
        uint16_t fail : 1;
        uint16_t is_host_message : 1;
        uint16_t has_payload : 1;
        uint16_t first : 1;
        uint16_t last : 1;
        uint16_t is_kenning : 1;
        uint16_t is_zephyr : 1;
        uint16_t reserved : 4; // Reserved for future use.
        uint16_t _ : 4;        // Space for message-specific flags.
    } general_purpose_flags;
    /**
     * Struct with flags specific to message type IOSPEC
     */
    struct __attribute__((packed))
    {
        uint16_t _ : 12; // Space for general purpose flags
        uint16_t serialized : 1;
        uint16_t reserved : 3; // Reserved for future use.
    } flags_iospec;
    uint16_t raw_bytes;
} flags_t;

/**
 * A packed struct representing message header.
 */
typedef struct __attribute__((packed))
{
    message_type_t message_type : 6;
    flow_control_flags_t flow_control_flags : 2;
    checksum_t checksum;
    flags_t flags;
    payload_size_t payload_size;
} message_hdr_t;

/**
 * A struct describing a stream of bytes (either a pointer and size, or a loader)
 */
typedef struct
{
    union
    {
        uint8_t *raw_bytes;
        struct msg_loader *loader;
    };
    payload_size_t size;
} protocol_payload_t;

/**
 * A struct that contains all parameters describing single message.
 */
typedef struct
{
    message_hdr_t hdr;
    uint8_t *payload;
} outgoing_message_t;

/**
 * A struct that contains all values transported in a transmission or a request.
 */
typedef struct
{
    message_type_t message_type;
    flags_t flags;
    protocol_payload_t payload;
    bool is_request;
} protocol_event_t;

/**
 * Sends payload with flags (either as one message or as a series of messages).
 *
 * @param msg Pointer to the message, that will be be sent or split into multiple messages.
 *
 * @returns status of the protocol
 */
status_t protocol_transmit(const protocol_event_t *msg);

/**
 * Waits for a transmission or a request.
 *
 * @param event received transmission/request
 * @param Pointer to a function, matching a message type to a loader. Should return NULL if there is no loader.
 *
 * @returns status of the protocol
 */
status_t protocol_listen(protocol_event_t *event, struct msg_loader *(*loader_callback)(message_type_t));

#endif // KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_
