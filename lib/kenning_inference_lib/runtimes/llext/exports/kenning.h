/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_KENNING_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_KENNING_H_

#include <zephyr/llext/symbol.h>

/* Kenning Zephyr Runtime exports */
#include <kenning_inference_lib/core/runtime_wrapper.h>

extern MlModel g_model_struct;

EXPORT_SYMBOL(g_model_struct);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_KENNING_H_