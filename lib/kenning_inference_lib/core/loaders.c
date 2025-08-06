/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kenning_inference_lib/core/loaders.h>

GENERATE_MODULE_STATUSES_STR(LOADERS);

status_t buf_save(struct msg_loader *ldr, const uint8_t *src, size_t n)
{
    if (ldr->written + n > ldr->max_size)
    {
        return LOADERS_STATUS_NOT_ENOUGH_MEMORY;
    }

    memcpy((uint8_t *)(ldr->addr) + ldr->written, src, n);
    ldr->written += n;

    return STATUS_OK;
}

status_t buf_save_one(struct msg_loader *ldr, void *c)
{
    if (ldr->written + 1 > ldr->max_size)
    {
        return LOADERS_STATUS_NOT_ENOUGH_MEMORY;
    }

    ((uint8_t *)(ldr->addr))[ldr->written] = *(uint8_t *)c;
    ldr->written++;

    return STATUS_OK;
}

status_t buf_reset(struct msg_loader *ldr, size_t n)
{
    ldr->written = 0;
    return STATUS_OK;
}

struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_LOADER_TYPES];
