/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_PROTOCOL_H_
#define KENNING_INFERENCE_LIB_CORE_PROTOCOL_H_

#include "kenning_inference_lib/core/utils.h"
#include <stdbool.h>

/**
 * UART custom error codes
 */
#define PROTOCOL_STATUSES(STATUS)             \
    STATUS(PROTOCOL_STATUS_INV_ARG_BAUDRATE)  \
    STATUS(PROTOCOL_STATUS_INV_ARG_WORDSIZE)  \
    STATUS(PROTOCOL_STATUS_INV_ARG_STOP_BITS) \
    STATUS(PROTOCOL_STATUS_RECV_ERROR)        \
    STATUS(PROTOCOL_STATUS_RECV_ERROR_NOSYS)  \
    STATUS(PROTOCOL_STATUS_RECV_ERROR_BUSY)   \
    STATUS(PROTOCOL_STATUS_NO_DATA)

GENERATE_MODULE_STATUSES(PROTOCOL);

/**
 * Initialize protocol
 *
 * @returns status of initialization
 */
status_t protocol_init();

/**
 * Write buffer of bytes
 *
 * @param data buffer to be written
 * @param data_length length of the buffer
 *
 * @returns error status of write
 */
status_t protocol_write_data(const uint8_t *data, size_t data_length);

/**
 * Reads bytes into given buffer
 *
 * @param data buffer for results
 * @param data_length length of the buffer
 *
 * @returns status of read action
 */
status_t protocol_read_data(uint8_t *data, size_t data_length);

#endif // KENNING_INFERENCE_LIB_CORE_PROTOCOL_H_
