/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_TFLITE_RESOLVER_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_TFLITE_RESOLVER_H_

#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>

#define TFLITE_RESOLVER_SIZE 128

void tflite_initialize_resolver();

#endif // KENNING_INFERENCE_LIB_RUNTIMES_TFLITE_RESOLVER_H_
