/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ai8x_loaders.h"
#include "cnn_model.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(ai8x_runtime);

status_t buf_save_one_cnn(struct msg_loader *ldr, void *c)
{
    uint32_t val = *(uint32_t *)c;

    switch ((enum cnn_load_state)ldr->state)
    {
    case CNN_LOAD_WEIGHTS_START:
        ldr->addr = (void *)val;
        if (val == 0)
        {
            ldr->state = (void *)CNN_LOAD_BIASES_START;
        }
        else
        {
            *((volatile uint8_t *)((uint32_t)val | 1)) = 0x01;
            ldr->state = (void *)CNN_LOAD_WEIGHTS_LENGTH;
        }
        break;

    case CNN_LOAD_WEIGHTS_LENGTH:
        ldr->max_size = val;
        ldr->written = 0;
        ldr->state = (void *)CNN_LOAD_WEIGHTS;
        break;

    case CNN_LOAD_WEIGHTS:
        *(((volatile uint32_t *)(ldr->addr)) + ldr->written++) = val;
        if (ldr->written == ldr->max_size)
        {
            ldr->state = (void *)CNN_LOAD_WEIGHTS_START;
        }
        break;

    case CNN_LOAD_BIASES_START:
        ldr->addr = (void *)val;
        if (val == 0)
        {
            ldr->state = (void *)CNN_LOAD_END;
        }
        else
        {
            ldr->state = (void *)CNN_LOAD_BIASES_LENGTH;
        }
        break;

    case CNN_LOAD_BIASES_LENGTH:
        ldr->max_size = val;
        ldr->written = 0;
        ldr->state = (void *)CNN_LOAD_BIASES;
        break;

    case CNN_LOAD_BIASES:
        *(((volatile uint32_t *)(ldr->addr)) + ldr->written++) = val;
        if (ldr->written == ldr->max_size)
        {
            ldr->state = (void *)CNN_LOAD_BIASES_START;
        }
        break;

    case CNN_LOAD_END:
        break;
    }
    return STATUS_OK;
}

status_t buf_save_cnn(struct msg_loader *ldr, const uint8_t *src, size_t n)
{
    status_t status = STATUS_OK;
    for (int i = 0; i < n / 4; i++)
    {
        status = buf_save_one_cnn(ldr, (uint32_t *)src + i);
        RETURN_ON_ERROR(status, status);
    }
    return status;
}

status_t buf_reset_cnn(struct msg_loader *ldr, size_t n)
{
    ldr->state = (void *)CNN_LOAD_WEIGHTS_START;
    ldr->addr = NULL;
    ldr->written = 0;
    ldr->max_size = 0;
    return STATUS_OK;
}

status_t buf_save_cnn_input(struct msg_loader *ldr, const uint8_t *src, size_t n)
{
    int s = cnn_load_input((uint32_t *)src, ldr->written, n / 4);

    if (s)
    {
        return LOADERS_STATUS_ERROR;
    }
    ldr->written += n;
    return STATUS_OK;
}

status_t buf_save_one_cnn_input(struct msg_loader *ldr, void *c) { return buf_save_cnn_input(ldr, c, 1); }

status_t buf_reset_cnn_input(struct msg_loader *ldr, size_t n)
{
    ldr->written = 0;
    return STATUS_OK;
}

status_t prepare_ai8x_ldr_table()
{
    static struct msg_loader msg_loader_model = {
        .save = buf_save_cnn,
        .save_one = buf_save_one_cnn,
        .reset = buf_reset_cnn,
    };
    buf_reset_cnn(&msg_loader_model, 0);

    static struct msg_loader msg_loader_input = {
        .save = buf_save_cnn_input,
        .save_one = buf_save_one_cnn_input,
        .reset = buf_reset_cnn_input,
    };
    buf_reset_cnn_input(&msg_loader_input, 0);

    memset(&g_ldr_tables[1], 0, NUM_LOADER_TYPES * sizeof(struct msg_loader *));
    g_ldr_tables[1][LOADER_TYPE_MODEL] = &msg_loader_model;
    g_ldr_tables[1][LOADER_TYPE_DATA] = &msg_loader_input;

    return STATUS_OK;
}
