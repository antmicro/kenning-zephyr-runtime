/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>

#include "kenning_inference_lib/core/kenning_protocol.h"
#include "kenning_inference_lib/core/logger.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(logger, CONFIG_LOGGER_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(LOGGER);

/*
 Array with bytes, ready to send to the client (Kenning).
 Format:
 <message 1 size (1 byte)><message 1 string><message 2 size (1 byte)><message 2 string>...<message n size (1
 byte)><message n string>.
*/
static uint8_t msg_buffer[CONFIG_KENNING_LOG_BUFFER_SIZE];
// Number of bytes already in the buffer (only counting messages, that are fully processed).
static int msg_buffer_len = 0;
// Number of characters placed in the buffer from the last/current message (not counting the size field).
static int curr_msg_len = 0;
// Since the size field is 1 byte, the message cannot be longer, than 256 bytes..
#define MAX_SINGLE_MESSAGE_LENGTH 0xFF
/*
 Flag, that needs to be set to 1, whenever this module is currently processing logs. All new logs are ignored when
 this flag is set to 1. This is supposed to prevent recursive log generation (so when processing logs creates more
 logs).
*/
static bool sending_logs = false;

int render_character(uint8_t *data, size_t length, void *ctx)
{
    for (int i = 0; i < length; i++)
    {
        int index = i + curr_msg_len + msg_buffer_len + 1; // We always shift the
                                                           // log message by 1
                                                           // byte, to leave space
                                                           // for the size field.
        if (index >= CONFIG_KENNING_LOG_BUFFER_SIZE || curr_msg_len >= MAX_SINGLE_MESSAGE_LENGTH)
        {
            return i;
        }
        msg_buffer[index] = data[i];
        curr_msg_len += 1;
    }
    return length;
}

LOG_OUTPUT_DEFINE(out, render_character, NULL, 0);

void send_all_messages()
{
    protocol_event_t transmission;
    transmission.message_type = MESSAGE_TYPE_LOGS;
    transmission.flags.raw_bytes = 0;
    transmission.flags.general_purpose_flags.is_zephyr = 1;
    transmission.payload.raw_bytes = msg_buffer;
    transmission.payload.size = msg_buffer_len;
    status_t status = protocol_transmit(&transmission);

    // Erase the buffer, if the transmission succeeded.
    if (status == STATUS_OK)
    {
        msg_buffer_len = 0;
    }
}

void process(const struct log_backend *const backend, union log_msg_generic *msg)
{
    // We are not processing a log message, generated while processing another log message.
    if (sending_logs)
    {
        return;
    }
    sending_logs = true;
    // Adding the current log message to the buffer.
    curr_msg_len = 0;
    log_output_msg_process(&out, &msg->log, log_backend_std_get_flags());
    // We insert message size in front of the message in the buffer.
    msg_buffer[msg_buffer_len] = curr_msg_len - 1; // We are subtracting 1 from 'curr_msg_len', because we are
                                                   // ignoring the last character of the message, since it's always a
                                                   // Line Feed, which we don't need.
    msg_buffer_len += curr_msg_len; // Normally we should add here 'curr_msg_len + 1' (to account for that 1 byte size
                                    // field). However in this case we want to deliberately ignore the last character
                                    // of the message (Line Feed)
    send_all_messages();
    sending_logs = false;
}

void panic(const struct log_backend *const backend)
{
    sending_logs = true;
    send_all_messages();
    sending_logs = false;
}

void init(const struct log_backend *const backend) { LOG_INF("Sending logs to Kenning..."); }

void notify(const struct log_backend *const backend, enum log_backend_evt event, union log_backend_evt_arg *arg)
{
    // This backend is designed for use in LOG_MODE_IMMEDIATE, so we do not need the notification system.
    return;
}

int format_set(const struct log_backend *const backend, uint32_t log_type)
{
    return -ENOTSUP; // This value denotes, that the backend does not support changing formats.
}

void dropped(const struct log_backend *const backend, uint32_t cnt) { LOG_WRN("Log messages dropped: %d", cnt); }

struct log_backend_api api = {
    .process = process, .dropped = dropped, .panic = panic, .init = init, .format_set = format_set, .notify = notify};

LOG_BACKEND_DEFINE(kenning_protocol_logging_backend, api, 0);

status_t logger_start()
{
    msg_buffer_len = 0;
    log_backend_enable(&kenning_protocol_logging_backend, NULL, 0);
    return STATUS_OK;
}

status_t logger_stop()
{
    log_backend_disable(&kenning_protocol_logging_backend);
    return STATUS_OK;
}
