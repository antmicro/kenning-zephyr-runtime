/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/utils.h"
#include <iree/base/allocator.h>
#include <iree/base/api.h>
#include <iree/base/string_view.h>
#include <zephyr/kernel.h>

K_HEAP_DEFINE(iree_heap, 1024 * CONFIG_KENNING_IREE_HEAP_SIZE);

iree_status_t iree_allocator_system_alloc(iree_allocator_command_t command, const iree_allocator_alloc_params_t *params,
                                          void **inout_ptr)
{
    IREE_ASSERT_ARGUMENT(params);
    IREE_ASSERT_ARGUMENT(inout_ptr);
    iree_host_size_t byte_length = params->byte_length;
    if (IREE_UNLIKELY(byte_length == 0))
    {
        return iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "allocations must be >0 bytes");
    }

    void *new_ptr = NULL;
    switch (command)
    {
    case IREE_ALLOCATOR_COMMAND_REALLOC:
    {
        if (IS_VALID_POINTER(*inout_ptr))
        {
            new_ptr = k_heap_realloc(&iree_heap, *inout_ptr, byte_length, K_NO_WAIT);
        }
        else
        {
            new_ptr = k_heap_alloc(&iree_heap, byte_length, K_NO_WAIT);
        }
    };
    break;
    case IREE_ALLOCATOR_COMMAND_CALLOC:
    {
        new_ptr = k_heap_calloc(&iree_heap, byte_length, 1, K_NO_WAIT);
    };
    break;
    case IREE_ALLOCATOR_COMMAND_MALLOC:
    {
        new_ptr = k_heap_alloc(&iree_heap, byte_length, K_NO_WAIT);
    };
    break;
    default:
    {
        return iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "system allocator did not recognize the request");
    }
    }
    if (!new_ptr)
    {
        return iree_make_status(IREE_STATUS_RESOURCE_EXHAUSTED, "system allocator failed the request");
    }

    *inout_ptr = new_ptr;
    return iree_ok_status();
}

iree_status_t iree_allocator_system_free(void **inout_ptr)
{
    IREE_ASSERT_ARGUMENT(inout_ptr);
    void *ptr = *inout_ptr;
    if (IREE_LIKELY(ptr != NULL))
    {

        k_heap_free(&iree_heap, ptr);
        *inout_ptr = NULL;
    }
    return iree_ok_status();
}

iree_status_t iree_allocator_ctl(void *self, iree_allocator_command_t command, const void *params, void **inout_ptr)
{
    switch (command)
    {
    case IREE_ALLOCATOR_COMMAND_MALLOC:
    case IREE_ALLOCATOR_COMMAND_CALLOC:
    case IREE_ALLOCATOR_COMMAND_REALLOC:
        return iree_allocator_system_alloc(command, (const iree_allocator_alloc_params_t *)params, inout_ptr);
    case IREE_ALLOCATOR_COMMAND_FREE:
        return iree_allocator_system_free(inout_ptr);
    default:
        return iree_make_status(IREE_STATUS_UNIMPLEMENTED, "unsupported system allocator command");
    }
}

iree_allocator_t iree_allocator_zephyr(void)
{
    iree_allocator_t v = {NULL, iree_allocator_ctl};
    return v;
}
