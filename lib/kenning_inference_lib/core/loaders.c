/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kenning_inference_lib/core/loaders.h>

int buf_save(struct msg_loader *ldr, uint8_t *src, size_t n)
{
    if (ldr->written + n > ldr->max_size)
        return -1;

    memcpy((uint8_t *)(ldr->addr) + ldr->written, src, n);
    ldr->written += n;

    return 0;
}

int buf_save_one(struct msg_loader *ldr, uint8_t c, size_t n)
{
    if (ldr->written + 1 > ldr->max_size)
        return -1;

    ((uint8_t *)(ldr->addr))[ldr->written] = c;
    ldr->written++;

    return 0;
}

int buf_reset(struct msg_loader *ldr, size_t n)
{
    ldr->written = 0;
    return 0;
}

struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_LOADER_TYPES];