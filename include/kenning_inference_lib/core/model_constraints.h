/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_MODEL_CONSTRAINTS_H_
#define KENNING_INFERENCE_LIB_CORE_MODEL_CONSTRAINTS_H_

/**
 * Model input and output metadata constraints (used in the model_spec_t struct in runtime_wrapper.h).
 *
 * For the loaded model, full information about multiple tensors (for both input and output) can be stored.
 * For each tensor, its full shape and data type is stored.
 */

/**
 * How many input/output tensors can the model have at most
 */
#define MAX_MODEL_INPUT_NUM 2
#define MAX_MODEL_OUTPUT_NUM 12

/**
 * No input/output tensor can have more dimensions than specified below
 */
#define MAX_MODEL_INPUT_DIM 4
#define MAX_MODEL_OUTPUT_DIM 4

/**
 * Maximum lengths of strings storing the model's name and entry function (parameters required by some runtimes)
 */
#define MAX_LENGTH_ENTRY_FUNC_NAME 20
#define MAX_LENGTH_MODEL_NAME 20

#endif // KENNING_INFERENCE_LIB_CORE_MODEL_CONSTRAINTS_H_
