/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#define LOG_MODULE_REGISTER(...)

#define LOG_ERR(fmt, ...) printf("[ERR] " fmt, ##__VA_ARGS__);
#define LOG_DBG(fmt, ...) printf("[DBG] " fmt, ##__VA_ARGS__);
#define LOG_INF(fmt, ...) printf("[INF] " fmt, ##__VA_ARGS__);
#define LOG_WRN(fmt, ...) printf("[WRN] " fmt, ##__VA_ARGS__);
