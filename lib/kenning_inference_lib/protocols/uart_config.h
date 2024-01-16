/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_PROTOCOLS_UART_CONFIG_H_
#define KENNING_INFERENCE_LIB_PROTOCOLS_UART_CONFIG_H_

#define UART_DEVICE_NODE DT_ALIAS(kcomms)
#define UART_TIMEOUT_S (0.5F) /* UART read timeout (500 ms) */

#endif // KENNING_INFERENCE_LIB_PROTOCOLS_UART_CONFIG_H_
