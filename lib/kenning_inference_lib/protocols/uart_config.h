/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_RUNTIME_PROTOCOLS_UART_CONFIG_H_
#define KENNING_RUNTIME_PROTOCOLS_UART_CONFIG_H_

#define UART_DEVICE_NODE DT_NODELABEL(uart1)
#define UART_TIMEOUT_S (0.1F) /* UART read timeout (100 ms) */

#endif // KENNING_RUNTIME_PROTOCOLS_UART_CONFIG_H_
