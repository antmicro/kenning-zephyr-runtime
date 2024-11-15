/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_LOADERS_H_
#define KENNING_INFERENCE_LIB_CORE_LOADERS_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct msg_loader
{
    int (*save)(struct msg_loader *, uint8_t *, size_t);
    int (*save_one)(struct msg_loader *, uint8_t, size_t);
    int (*reset)(struct msg_loader *, size_t);
    size_t written;
    size_t max_size;
    void *addr;
};

int buf_save(struct msg_loader *ldr, uint8_t *src, size_t n);

int buf_save_one(struct msg_loader *ldr, uint8_t c, size_t n);

int buf_reset(struct msg_loader *ldr, size_t n);

#define MSG_LOADER_BUF(_addr, _max_size)                                                                       \
    {                                                                                                          \
        .save = buf_save, .save_one = buf_save_one, .reset = buf_reset, .written = 0, .max_size = (_max_size), \
        .addr = (_addr)                                                                                        \
    }

#endif // KENNING_INFERENCE_LIB_CORE_LOADERS_H_
