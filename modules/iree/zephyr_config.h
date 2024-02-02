/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_CONFIG_H_
#define ZEPHYR_CONFIG_H_

// IREE_TIME_NOW_FN is required and used to fetch the current RTC time and to be
// used for wait handling. A thread-less system can just return 0.
#define IREE_TIME_NOW_FN \
    {                    \
        return 0;        \
    }

// IREE_DEVICE_SIZE_T for status print out.
#define IREE_DEVICE_SIZE_T uint32_t
#define PRIdsz PRIu32

#endif // ZEPHYR_CONFIG_H_
