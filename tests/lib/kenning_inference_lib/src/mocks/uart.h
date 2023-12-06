/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_TESTS_MOCKS_UART_H_
#define KENNING_INFERENCE_LIB_TESTS_MOCKS_UART_H_

#define DT_NODELABEL(node) node
#define DEVICE_DT_GET(node) NULL

struct device
{
    const char *name;
};

bool device_is_ready(const struct device *dev);

int uart_poll_in(const struct device *dev, unsigned char *p_char);

void uart_poll_out(const struct device *dev, unsigned char out_char);

#endif // KENNING_INFERENCE_LIB_TESTS_MOCKS_UART_H_