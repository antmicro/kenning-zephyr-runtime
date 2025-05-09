/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_AI8X_CNN_MODEL_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_AI8X_CNN_MODEL_H_

#include <stdint.h>

/**
 * Initializes CNN parameters
 *
 * @returns CNN_OK if initialization was successful, CNN_FAIL otherwise
 */
int cnn_init(void);

/**
 * Configures CNN layers
 *
 * @returns CNN_OK if initialization was successful, CNN_FAIL otherwise
 */
int cnn_configure(void);

/**
 * Unloads CNN output
 *
 * @returns CNN_OK if initialization was successful, CNN_FAIL otherwise
 */
int cnn_unload(uint32_t *out_buf);

/**
 * Loads model input
 *
 * @returns CNN_OK if initialization was successful, CNN_FAIL otherwise
 */
int cnn_load_input(uint32_t *src, uint32_t off, uint32_t n);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_AI8X_CNN_MODEL_H_
