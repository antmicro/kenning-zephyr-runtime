/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/protocol.h"
#include "uart_config.h"
#include <stdbool.h>
#include <string.h>

#ifndef __UNIT_TEST__
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#else // __UNIT_TEST__
#include "mocks/kernel.h"
#include "mocks/log.h"
#include "mocks/uart.h"
#endif

LOG_MODULE_REGISTER(uart, CONFIG_UART_LOG_LEVEL);

GENERATE_MODULE_STATUSES_STR(PROTOCOL);

ut_static const struct device *const G_UART_DEV = DEVICE_DT_GET(UART_DEVICE_NODE);

ut_static bool g_uart_initialized = false;

#ifdef __UNIT_TEST__
extern int64_t g_ticks;

static int64_t k_uptime_get() { return g_ticks++; }
#endif // __UNIT_TEST__

status_t protocol_init()
{
    if (g_uart_initialized)
    {
        return STATUS_OK;
    }

    if (!device_is_ready(G_UART_DEV))
    {
        return PROTOCOL_STATUS_ERROR;
    }

    g_uart_initialized = true;

    return STATUS_OK;
}

status_t protocol_write_data(const uint8_t *data, size_t data_length)
{
    if (!g_uart_initialized)
    {
        return PROTOCOL_STATUS_UNINIT;
    }

    RETURN_ERROR_IF_POINTER_INVALID(data, PROTOCOL_STATUS_INV_PTR);

    LOG_DBG("Writing %zu bytes to UART", data_length);

    status_t status = STATUS_OK;

    for (int i = 0; i < data_length; ++i)
    {
        uart_poll_out(G_UART_DEV, data[i]);
    }
    return status;
}

status_t protocol_read_data(uint8_t *data, size_t data_length)
{
    if (!g_uart_initialized)
    {
        return PROTOCOL_STATUS_UNINIT;
    }

    LOG_DBG("Reading %zu bytes from UART", data_length);

    size_t data_read = 0;
    int rx_status = 0;
    int64_t start_timer = k_uptime_get();

    while (data_read < data_length)
    {
        uint8_t c = 0;
        rx_status = uart_poll_in(G_UART_DEV, &c);
#ifdef CONFIG_NRF_UART_SLEEP_AFTER_POLL_WORKAROUND
        if (0 == data_read % 128)
        {
            k_sleep(K_TICKS(1));
        }
#endif // CONFIG_NRF_UART_SLEEP_AFTER_POLL_WORKAROUND
        if (0 == rx_status)
        {
            if (IS_VALID_POINTER(data))
            {
                data[data_read] = c;
            }
            data_read++;
            start_timer = k_uptime_get();
        }
        else if (k_uptime_get() - start_timer > UART_TIMEOUT_S * 1000)
        {
            return PROTOCOL_STATUS_TIMEOUT;
        }
        else if (-ENOSYS == rx_status)
        {
            return PROTOCOL_STATUS_RECV_ERROR_NOSYS;
        }
        else if (-EBUSY == rx_status)
        {
            return PROTOCOL_STATUS_RECV_ERROR_BUSY;
        }
    }
    return STATUS_OK;
}
