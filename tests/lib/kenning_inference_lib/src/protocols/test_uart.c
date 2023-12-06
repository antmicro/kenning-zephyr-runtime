/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/fff.h>
#include <zephyr/ztest.h>

#include <kenning_inference_lib/core/protocol.h>

#include "utils.h"

#define UART_BUFFER_SIZE 256

extern bool g_uart_initialized;

uint64_t g_ticks;
uint8_t g_uart_buffer_out[UART_BUFFER_SIZE];
size_t g_uart_buffer_out_idx;
uint8_t g_uart_buffer_in[UART_BUFFER_SIZE];
size_t g_uart_buffer_in_idx;

// ========================================================
// mocks
// ========================================================
DEFINE_FFF_GLOBALS;

#define VOID_MOCKS(MOCK) MOCK(uart_poll_out, const struct device *, unsigned char)

#define MOCKS(MOCK)                                                 \
    MOCK(bool, device_is_ready, const struct device *)              \
    MOCK(int, uart_poll_in, const struct device *, unsigned char *) \
    MOCK(int32_t, k_sleep, k_timeout_t)

VOID_MOCKS(DECLARE_VOID_MOCK);
MOCKS(DECLARE_MOCK);

void uart_poll_out_mock(const struct device *dev, unsigned char out_char);

int uart_poll_in_mock(const struct device *dev, unsigned char *p_char);

int32_t k_sleep_mock(k_timeout_t timeout);

// ========================================================
// setup
// ========================================================

static void uart_tests_setup_f()
{
    VOID_MOCKS(RESET_VOID_MOCK);
    MOCKS(RESET_MOCK);

    k_sleep_fake.custom_fake = k_sleep_mock;

    g_uart_initialized = false;
    g_ticks = 0;
    g_uart_buffer_out_idx = 0;
    g_uart_buffer_in_idx = 0;
}

ZTEST_SUITE(kenning_inference_lib_test_uart, NULL, NULL, uart_tests_setup_f, NULL, NULL);

// ========================================================
// protocol_init
// ========================================================

/**
 * Tests UART initialization when device is ready
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_init)
{
    status_t status = STATUS_OK;

    device_is_ready_fake.return_val = true;
    status = protocol_init();

    zassert_equal(STATUS_OK, status);
    zassert_true(g_uart_initialized);
}

/**
 * Tests UART initialization when UART is already initialized
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_init_already_initialized)
{
    status_t status = STATUS_OK;

    device_is_ready_fake.return_val = true;
    g_uart_initialized = true;

    status = protocol_init();

    zassert_equal(STATUS_OK, status);
    zassert_true(g_uart_initialized);
}

/**
 * Tests UART initialization when device is not ready
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_init_device_not_ready)
{
    status_t status = STATUS_OK;

    device_is_ready_fake.return_val = false;
    status = protocol_init();

    zassert_equal(PROTOCOL_STATUS_ERROR, status);
    zassert_false(g_uart_initialized);
}

// ========================================================
// protocol_write_data
// ========================================================

/**
 * Tests writing data to UART
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_write_data)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    g_uart_initialized = true;
    uart_poll_out_fake.custom_fake = uart_poll_out_mock;

    status = protocol_write_data(data, sizeof(data));

    zassert_equal(STATUS_OK, status);
    zassert_mem_equal(data, g_uart_buffer_out, sizeof(data));
    zassert_equal(sizeof(data), g_uart_buffer_out_idx);
}

/**
 * Tests writing data to UART when UART not initialized
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_write_data_not_initialized)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    uart_poll_out_fake.custom_fake = uart_poll_out_mock;

    status = protocol_write_data(data, sizeof(data));

    zassert_equal(PROTOCOL_STATUS_UNINIT, status);
    zassert_equal(0, g_uart_buffer_out_idx);
}

/**
 * Tests writing data to UART when data is invalid
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_write_data_invalid_pointer)
{
    status_t status = STATUS_OK;

    g_uart_initialized = true;
    uart_poll_out_fake.custom_fake = uart_poll_out_mock;

    status = protocol_write_data(NULL, 1);

    zassert_equal(PROTOCOL_STATUS_INV_PTR, status);
    zassert_equal(0, g_uart_buffer_out_idx);
}

/**
 * Tests writing data to UART when data size is 0
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_write_data_no_data)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";

    g_uart_initialized = true;
    uart_poll_out_fake.custom_fake = uart_poll_out_mock;

    status = protocol_write_data(data, 0);

    zassert_equal(STATUS_OK, status);
    zassert_equal(0, g_uart_buffer_out_idx);
}

// ========================================================
// protocol_read_data
// ========================================================

/**
 * Tests reading data from UART
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_read_data)
{
    status_t status = STATUS_OK;
    uint8_t buffer[256];
    uint8_t data[] = "some data";
    memcpy(g_uart_buffer_in, data, sizeof(data));

    g_uart_initialized = true;
    uart_poll_in_fake.custom_fake = uart_poll_in_mock;

    status = protocol_read_data(buffer, sizeof(data));

    zassert_equal(STATUS_OK, status);
    zassert_mem_equal(data, buffer, sizeof(data));
    zassert_equal(sizeof(data), g_uart_buffer_in_idx);
}

/**
 * Tests reading data from UART when UART is not initialized
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_read_data_not_initialized)
{
    status_t status = STATUS_OK;
    uint8_t buffer[256];
    uint8_t data[] = "some data";
    memcpy(g_uart_buffer_in, data, sizeof(data));

    uart_poll_in_fake.custom_fake = uart_poll_in_mock;

    status = protocol_read_data(buffer, sizeof(data));

    zassert_equal(PROTOCOL_STATUS_UNINIT, status);
    zassert_equal(0, g_uart_buffer_in_idx);
}

/**
 * Tests reading data from UART when buffer is invalid
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_read_data_invalid_pointer)
{
    status_t status = STATUS_OK;
    uint8_t data[] = "some data";
    memcpy(g_uart_buffer_in, data, sizeof(data));

    g_uart_initialized = true;
    uart_poll_in_fake.custom_fake = uart_poll_in_mock;

    status = protocol_read_data(NULL, sizeof(data));

    zassert_equal(PROTOCOL_STATUS_INV_PTR, status);
    zassert_equal(0, g_uart_buffer_in_idx);
}

/**
 * Tests reading data from UART when data size is 0
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_read_data_no_data)
{
    status_t status = STATUS_OK;
    uint8_t buffer[256];
    uint8_t data[] = "some data";
    memcpy(g_uart_buffer_in, data, sizeof(data));

    g_uart_initialized = true;
    uart_poll_in_fake.custom_fake = uart_poll_in_mock;

    status = protocol_read_data(buffer, 0);

    zassert_equal(STATUS_OK, status);
    zassert_equal(0, g_uart_buffer_in_idx);
}

/**
 * Tests reading data from UART when data size is 0
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_read_data_timeout)
{
    status_t status = STATUS_OK;
    uint8_t buffer[256];
    uint8_t data[] = "some data";
    memcpy(g_uart_buffer_in, data, sizeof(data));

    g_uart_initialized = true;
    uart_poll_in_fake.return_val = -1;

    status = protocol_read_data(buffer, sizeof(data));

    zassert_equal(PROTOCOL_STATUS_TIMEOUT, status);
}

/**
 * Tests reading data from UART when operation not implenented
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_read_data_nosys)
{
    status_t status = STATUS_OK;
    uint8_t buffer[256];
    uint8_t data[] = "some data";
    memcpy(g_uart_buffer_in, data, sizeof(data));

    g_uart_initialized = true;
    uart_poll_in_fake.return_val = -ENOSYS;

    status = protocol_read_data(buffer, sizeof(data));

    zassert_equal(PROTOCOL_STATUS_RECV_ERROR_NOSYS, status);
}

/**
 * Tests reading data from UART when async reception enavbled
 */
ZTEST(kenning_inference_lib_test_uart, test_uart_read_data_busy)
{
    status_t status = STATUS_OK;
    uint8_t buffer[256];
    uint8_t data[] = "some data";
    memcpy(g_uart_buffer_in, data, sizeof(data));

    g_uart_initialized = true;
    uart_poll_in_fake.return_val = -EBUSY;

    status = protocol_read_data(buffer, sizeof(data));

    zassert_equal(PROTOCOL_STATUS_RECV_ERROR_BUSY, status);
}

// ========================================================
// mocks
// ========================================================

void uart_poll_out_mock(const struct device *dev, unsigned char out_char)
{
    if (g_uart_buffer_out_idx < UART_BUFFER_SIZE)
    {
        g_uart_buffer_out[g_uart_buffer_out_idx++] = out_char;
    }
}

int uart_poll_in_mock(const struct device *dev, unsigned char *p_char)
{
    if (g_uart_buffer_in_idx < UART_BUFFER_SIZE)
    {
        *p_char = g_uart_buffer_in[g_uart_buffer_in_idx++];
        return 0;
    }
    return -1;
}

int32_t k_sleep_mock(k_timeout_t timeout)
{
    g_ticks += timeout.ticks;
    return 0;
}
