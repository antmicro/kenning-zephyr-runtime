/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "kenning_inference_lib/core/runtime_wrapper.h"
#include "kenning_inference_lib/core/utils.h"
#include <iree/base/allocator.h>
#include <iree/base/api.h>
#include <iree/base/string_view.h>
#include <zephyr/kernel.h>

K_HEAP_DEFINE(iree_heap, 1024 * CONFIG_KENNING_IREE_HEAP_SIZE);

// ===============================================================
// Mechanism for collecting statistics from the custom IREE heap
// ===============================================================

// Current allocation statistics (total allocated bytes, total freed bytes, peak allocation).
runtime_statistics_allocation_t iree_total_alloc_stats;

// Number of bytes that are currently allocated.
static uint64_t currently_allocated;

/**
 * Note an allocation of N bytes in the statistics.
 *
 * @param size Number of allocated bytes.
 *
 * @returns None
 */
static void report_allocation(uint64_t size)
{
    currently_allocated += size;
    iree_total_alloc_stats.peak_allocated = MAX(iree_total_alloc_stats.peak_allocated, currently_allocated);
    iree_total_alloc_stats.total_allocated += size;
}

/**
 * Note a deallocation of N bytes in the statistics
 *
 * @param size Number of freed bytes.
 *
 * @returns None
 */
static void report_deallocation(uint64_t size)
{
    currently_allocated -= size;
    iree_total_alloc_stats.total_freed += size;
}

/**
 * Note a deallocation of bytes from a given pointer in the statistics.
 *
 * @param ptr Pointer to the memory area being freed.
 *
 * @returns None
 */
static void report_deallocation_ptr(void *ptr) { report_deallocation(sys_heap_usable_size(&(iree_heap.heap), ptr)); }

/**
 * Resets all allocation statistics to 0.
 *
 * @returns STATUS_OK
 */
status_t iree_allocator_reset_stats()
{
    iree_total_alloc_stats.peak_allocated = 0;
    iree_total_alloc_stats.total_allocated = 0;
    iree_total_alloc_stats.total_freed = 0;
    currently_allocated = 0;
    return STATUS_OK;
}

/**
 * Downloads collected allocation statistic (without resetting them, so it can be called multiple times).
 *
 * @param ptr Pointer to the struct, where statistics will be placed.
 *
 * @returns RUNTIME_WRAPPER_STATUS_INV_PTR if the given pointer is invalid, STATUS_OK otherwise.
 */
status_t iree_allocator_get_stats(runtime_statistics_allocation_t *allocation_stats)
{
    RETURN_ERROR_IF_POINTER_INVALID(allocation_stats, RUNTIME_WRAPPER_STATUS_INV_PTR);
    *allocation_stats = iree_total_alloc_stats;
    return STATUS_OK;
}

// ===============================================================
// Implementations of allocation functions for the custom IREE heap
// ===============================================================

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
    report_allocation(byte_length);
    switch (command)
    {
    case IREE_ALLOCATOR_COMMAND_REALLOC:
    {
        if (IS_VALID_POINTER(*inout_ptr))
        {
            report_deallocation_ptr(existing_ptr);
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
        report_deallocation_ptr(ptr);
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
