/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_IREE_IREE_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_IREE_IREE_H_

#include <iree/base/status.h>
#include <iree/hal/allocator.h>
#include <iree/hal/device.h>
#include <iree/vm/instance.h>

#include <zephyr/logging/log.h>

#define BREAK_ON_IREE_ERROR(status)                                                                 \
    if (!iree_status_is_ok(status))                                                                 \
    {                                                                                               \
        LOG_ERR("IREE failed " __FILE__ ":%d with status: %d", __LINE__, iree_status_code(status)); \
        break;                                                                                      \
    }

#define CHECK_IREE_STATUS(status)                                                                   \
    if (!iree_status_is_ok(status))                                                                 \
    {                                                                                               \
        LOG_ERR("IREE failed " __FILE__ ":%d with status: %d", __LINE__, iree_status_code(status)); \
        return RUNTIME_WRAPPER_STATUS_ERROR;                                                        \
    }

#endif // KENNING_INFERENCE_LIB_RUNTIMES_IREE_IREE_H_

/**
 * Creates IREE device
 *
 * @param instance IREE vm instance
 * @param host_allocator allocator
 * @param out_device created device
 *
 * @returns error status
 */
iree_status_t create_device(iree_vm_instance_t *instance, iree_allocator_t host_allocator,
                            iree_hal_device_t **out_device);

iree_allocator_t iree_allocator_zephyr(void);
