/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_
#define KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_

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
        return KENNING_PROTOCOL_STATUS_CLIENT_DISCONNECTED;                 \
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

typedef struct __attribute__((packed))
{
    message_type_t message_type : 6;
    flow_control_flags_t flow_control_flags : 2;
    checksum_t checksum;
    flags_t flags;
    payload_size_t payload_size;
} message_hdr_t;

/**
 * A struct that contains all parameters describing single message
 */
typedef struct
{
    message_hdr_t hdr;
    uint8_t *payload;
} resp_message_t;

/* Max length of LLEXT name */
#define LLEXT_NAME_MAX_LEN 32

/**
 * Sends payload with flags (either as one message or as a series of messages).
 *
 * @param msg Pointer to the message, that will be be sent or split into multiple messages.
 *
 * @returns status of the protocol
 */
status_t protocol_transmit(const resp_message_t *msg);
/**
 * Waits for a message to be received
 *
 * @param msg received message
 *
 * @returns status of the protocol
 */
status_t protocol_recv_msg(message_hdr_t *hdr);
/**
 * Sends given message
 *
 * @param msg message to be sent
 *
 * @returns status of the protocol
 */
status_t protocol_send_msg(const resp_message_t *msg);
/**
 * Create a message that indicates an successful action. The message type is OK
 * and the paylaod is empty
 *
 * @param response created message
 *
 * @returns status of the protocol
 */
status_t protocol_prepare_success_resp(resp_message_t *response);
/**
 * Create a message that indicates an error. The message type is ERROR and the
 * payload is empty
 *
 * @param response created message
 *
 * @returns status of the protocol
 */
status_t protocol_prepare_fail_resp(resp_message_t *response);

status_t protocol_recv_msg_content(struct msg_loader *ldr, size_t n);

status_t protocol_recv_msg_hdr(message_hdr_t *hdr);

#endif // KENNING_INFERENCE_LIB_CORE_KENNING_PROTOCOL_H_
