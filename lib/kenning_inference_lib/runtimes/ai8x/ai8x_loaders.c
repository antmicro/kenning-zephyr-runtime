/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ai8x_loaders.h"
#include "cnn_model.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(ai8x_runtime);

static status_t buf_save_one_cnn(struct msg_loader *ldr, void *c)
{
    // This loader uses a state machine to load the model.
    // `max_size` and `addr` are determined dynamically.

    // Model contains an array of weights and an array of biases.
    // Each entry in the array of weights/biases has target address (4B),
    // length (4B) and content (length * 4B). If the target address is 0
    // the state machine starts loading biases (if found during weight loading)
    // or stops (if found during bias loading).

    uint32_t val = *(uint32_t *)c;
    switch ((enum cnn_load_state)ldr->state)
    {
    case CNN_LOAD_WEIGHTS_START:
        ldr->addr = (void *)val;
        if (val == 0)
        {
            // Start loading biases if the address is 0
            ldr->state = (void *)CNN_LOAD_BIASES_START;
        }
        else
        {
            // Probably necessary for MRAM writes. Taken from
            // https://github.com/analogdevicesinc/msdk/blob/89609813e1bac31cd7d93e1bb60b0aecb5a90bc6/Examples/MAX78000/CNN/mnist/cnn.c#L91
            *((volatile uint8_t *)((uint32_t)val | 1)) = 0x01;

            // Load the rest of the entry
            ldr->state = (void *)CNN_LOAD_WEIGHTS_LENGTH;
        }
        break;

    case CNN_LOAD_WEIGHTS_LENGTH:
        ldr->max_size = val;
        ldr->written = 0;
        ldr->state = (void *)CNN_LOAD_WEIGHTS;
        break;

    case CNN_LOAD_WEIGHTS:
        // Append to the target address
        *(((volatile uint32_t *)(ldr->addr)) + ldr->written++) = val;
        if (ldr->written == ldr->max_size)
        {
            // Start loading another entry
            ldr->state = (void *)CNN_LOAD_WEIGHTS_START;
        }
        break;

    case CNN_LOAD_BIASES_START:
        ldr->addr = (void *)val;
        if (val == 0)
        {
            // Stop loading if the address is 0
            ldr->state = (void *)CNN_LOAD_END;
        }
        else
        {
            // Load the rest of the entry
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
            // Start loading another entry
            ldr->state = (void *)CNN_LOAD_BIASES_START;
        }
        break;

    case CNN_LOAD_END:
        // TODO: Ensure that this case is entered once. This is not critical
        // but it may help with error detection.
        break;
    }
    return STATUS_OK;
}

static status_t buf_save_cnn(struct msg_loader *ldr, const uint8_t *src, size_t n)
{
    status_t status = STATUS_OK;

    // `buf_save_one_cnn` expects `uint32_t` instead of `uint8_t`
    for (int i = 0; i < n / sizeof(uint32_t); i++)
    {
        status = buf_save_one_cnn(ldr, (uint32_t *)src + i);
        RETURN_ON_ERROR(status, status);
    }
    return status;
}

static status_t buf_reset_cnn(struct msg_loader *ldr, size_t n)
{
    ldr->state = (void *)CNN_LOAD_WEIGHTS_START;
    ldr->addr = NULL;
    ldr->written = 0;
    ldr->max_size = 0;
    return STATUS_OK;
}

static status_t buf_save_cnn_input(struct msg_loader *ldr, const uint8_t *src, size_t n)
{
    int s = cnn_load_input((uint32_t *)src, ldr->written, n / sizeof(uint32_t));

    if (s)
    {
        return LOADERS_STATUS_ERROR;
    }
    ldr->written += n;
    return STATUS_OK;
}

static status_t buf_save_one_cnn_input(struct msg_loader *ldr, void *c)
{
    return buf_save_cnn_input(ldr, c, sizeof(uint32_t));
}

static status_t buf_reset_cnn_input(struct msg_loader *ldr, size_t n)
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
