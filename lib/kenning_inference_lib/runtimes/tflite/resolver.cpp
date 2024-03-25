/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>

#include "resolver.h"

tflite::MicroMutableOpResolver<TFLITE_RESOLVER_SIZE> g_tflite_resolver;

void tflite_initialize_resolver()
{
    g_tflite_resolver.AddConv2D();
    g_tflite_resolver.AddFullyConnected();
    g_tflite_resolver.AddMaxPool2D();
    g_tflite_resolver.AddReshape();
    g_tflite_resolver.AddSoftmax();
}
