/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_IREE_IREE_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_IREE_IREE_H_

#define BREAK_ON_IREE_ERROR(status) \
    if (!iree_status_is_ok(status)) \
    {                               \
        break;                      \
    }

#define CHECK_IREE_STATUS(status)            \
    if (!iree_status_is_ok(status))          \
    {                                        \
        return RUNTIME_WRAPPER_STATUS_ERROR; \
    }

#endif // KENNING_INFERENCE_LIB_RUNTIMES_IREE_IREE_H_
