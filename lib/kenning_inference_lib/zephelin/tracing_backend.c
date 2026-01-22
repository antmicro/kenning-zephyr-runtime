/*
 * Copyright (c) 2026 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_ZPL_TRACE_BACKEND_KENNING_PROTOCOL

#include "kenning_inference_lib/core/kenning_protocol.h"
#include <tracing_backend.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zephelin_tracing_backend, CONFIG_ZEPHELIN_TRACING_BACKEND_LOG_LEVEL);

static uint8_t g_trace_buffer[CONFIG_ZPL_TRACE_BACKEND_KENNING_PROTOCOL_BUFFER_SIZE];
static int g_trace_buffer_size;

extern bool g_client_connected;

static void tracing_backend_kenning_protocol_init(void)
{
    g_trace_buffer_size = 0;
    LOG_DBG("Kenning Protocol Tracing Backend initialized");
}

static void tracing_backend_kenning_protocol_output(const struct tracing_backend *backend, uint8_t *data,
                                                    uint32_t length)
{
    if (g_trace_buffer_size + length > CONFIG_ZPL_TRACE_BACKEND_KENNING_PROTOCOL_BUFFER_SIZE / 2)
    {
        status_t status = STATUS_OK;
        if (g_client_connected)
        {
            protocol_event_t transmission;
            transmission.message_type = MESSAGE_TYPE_TRACE_DATA;
            transmission.payload.raw_bytes = g_trace_buffer;
            transmission.payload.size = g_trace_buffer_size;
            // We record the status, because the transmission might fail if the protocol is busy,
            // in such case we will attempt the transmission again.
            status = protocol_transmit(&transmission);
        }
        // Resetting buffer if:
        // - transmission was successfull
        // - or transmission was not at all attempted (becasue client has not connected yet)
        // - or a buffer overflow is imminent
        if (status == STATUS_OK || g_trace_buffer_size + length > CONFIG_ZPL_TRACE_BACKEND_KENNING_PROTOCOL_BUFFER_SIZE)
        {
            g_trace_buffer_size = 0;
        }
    }
    for (int i = 0; i < length; i++)
    {
        g_trace_buffer[i + g_trace_buffer_size] = data[i];
    }
    g_trace_buffer_size += length;
}

const struct tracing_backend_api tracing_backend_kenning_protocol_api = {
    .init = tracing_backend_kenning_protocol_init, .output = tracing_backend_kenning_protocol_output};

TRACING_BACKEND_DEFINE(tracing_backend_kenning_protocol, tracing_backend_kenning_protocol_api);

#endif
