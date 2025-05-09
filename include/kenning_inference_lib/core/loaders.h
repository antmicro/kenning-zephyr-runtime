/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_LOADERS_H_
#define KENNING_INFERENCE_LIB_CORE_LOADERS_H_

#include "kenning_inference_lib/core/utils.h"

#define LOADERS_STATUSES(STATUS) STATUS(LOADERS_STATUS_NOT_ENOUGH_MEMORY)

GENERATE_MODULE_STATUSES(LOADERS);

#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct msg_loader
{
    int (*save)(struct msg_loader *, const uint8_t *, size_t);
    int (*save_one)(struct msg_loader *, void *);
    int (*reset)(struct msg_loader *, size_t);
    size_t written;
    size_t max_size;
    void *addr;
    void *state;
};

int buf_save(struct msg_loader *ldr, const uint8_t *src, size_t n);

int buf_save_one(struct msg_loader *ldr, void *c);

int buf_reset(struct msg_loader *ldr, size_t n);

#define MSG_LOADER_BUF(_addr, _max_size)                                                                       \
    {                                                                                                          \
        .save = buf_save, .save_one = buf_save_one, .reset = buf_reset, .written = 0, .max_size = (_max_size), \
        .addr = (_addr)                                                                                        \
    }

#define MSG_LOADER_BUF_RESET(_addr, _max_size, _reset)                                                        \
    {                                                                                                         \
        .save = buf_save, .save_one = buf_save_one, .reset = (_reset), .written = 0, .max_size = (_max_size), \
        .addr = (_addr)                                                                                       \
    }

#define LOADER_TYPES(TYPE)    \
    TYPE(LOADER_TYPE_NONE)    \
    TYPE(LOADER_TYPE_DATA)    \
    TYPE(LOADER_TYPE_MODEL)   \
    TYPE(LOADER_TYPE_IOSPEC)  \
    TYPE(LOADER_TYPE_RUNTIME) \
    TYPE(NUM_LOADER_TYPES)

typedef enum
{
    LOADER_TYPES(GENERATE_ENUM)
} LOADER_TYPE;

#define LDR_TABLE_COUNT 2
extern struct msg_loader *g_ldr_tables[LDR_TABLE_COUNT][NUM_LOADER_TYPES];

#endif // KENNING_INFERENCE_LIB_CORE_LOADERS_H_
