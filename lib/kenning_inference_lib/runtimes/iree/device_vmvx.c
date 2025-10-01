/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "iree.h"

#include <iree/hal/drivers/local_sync/sync_device.h>
#include <iree/hal/local/loaders/vmvx_module_loader.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(iree_runtime_vmvx, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

iree_status_t create_device(iree_vm_instance_t *instance, iree_allocator_t host_allocator,
                            iree_hal_device_t **out_device)
{
    iree_status_t iree_status = iree_ok_status();
    iree_hal_executable_loader_t *loader = NULL;
    iree_hal_allocator_t *device_allocator = NULL;

    do
    {
        iree_allocator_t host_allocator = iree_allocator_system();
        // prepare params
        iree_hal_sync_device_params_t params;
        iree_hal_sync_device_params_initialize(&params);

        // create loader
        iree_status = iree_hal_vmvx_module_loader_create(instance, 0, NULL, host_allocator, &loader);
        BREAK_ON_IREE_ERROR(iree_status);

        // allocate buffers
        iree_string_view_t identifier = iree_make_cstring_view("local");
        iree_status = iree_hal_allocator_create_heap(identifier, host_allocator, host_allocator, &device_allocator);
        BREAK_ON_IREE_ERROR(iree_status);

        // create device
        iree_status = iree_hal_sync_device_create(identifier, &params, /*loader_count=*/1, &loader, device_allocator,
                                                  host_allocator, out_device);
    } while (0);

    // cleanup
    if (NULL != loader)
    {
        iree_hal_executable_loader_release(loader);
    }
    if (NULL != device_allocator)
    {
        iree_hal_allocator_release(device_allocator);
    }

    return iree_status;
}
