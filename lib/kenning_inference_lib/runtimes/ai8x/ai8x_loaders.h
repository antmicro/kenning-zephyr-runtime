/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_AI8X_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_AI8X_H_

#include <kenning_inference_lib/core/loaders.h>

/**
 * Enum with CNN accelerator weights loading states
 */
enum cnn_load_state
{
    CNN_LOAD_WEIGHTS_START,
    CNN_LOAD_WEIGHTS_LENGTH,
    CNN_LOAD_WEIGHTS,
    CNN_LOAD_BIASES_START,
    CNN_LOAD_BIASES_LENGTH,
    CNN_LOAD_BIASES,
    CNN_LOAD_END,
};

status_t buf_save_one_cnn(struct msg_loader *ldr, void *c);

status_t buf_save_cnn(struct msg_loader *ldr, const uint8_t *src, size_t n);

status_t buf_reset_cnn(struct msg_loader *ldr, size_t n);

status_t buf_save_cnn_input(struct msg_loader *ldr, const uint8_t *src, size_t n);

status_t buf_save_one_cnn_input(struct msg_loader *ldr, void *c);

status_t buf_reset_cnn_input(struct msg_loader *ldr, size_t n);

/**
 * Prepares loader table for ai8x rutime
 *
 * @returns error status
 */
status_t prepare_ai8x_ldr_table();

#endif // KENNING_INFERENCE_LIB_RUNTIMES_AI8X_H_
