/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_KENNING_INFERENCE_LIB_MOCKS_LOG_H_
#define TESTS_KENNING_INFERENCE_LIB_MOCKS_LOG_H_

#include <stdio.h>

#define LOG_MODULE_REGISTER(...)

#define LOG_ERR(fmt, ...) printf("[ERR] " fmt "\n", ##__VA_ARGS__);
#define LOG_DBG(fmt, ...) printf("[DBG] " fmt "\n", ##__VA_ARGS__);
#define LOG_INF(fmt, ...) printf("[INF] " fmt "\n", ##__VA_ARGS__);
#define LOG_WRN(fmt, ...) printf("[WRN] " fmt "\n", ##__VA_ARGS__);

#endif // TESTS_KENNING_INFERENCE_LIB_MOCKS_LOG_H_
