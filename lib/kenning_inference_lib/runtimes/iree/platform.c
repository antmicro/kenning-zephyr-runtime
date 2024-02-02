/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    *memptr = k_aligned_alloc(alignment, size);
    return (NULL == *memptr) ? 1 : 0;
}
