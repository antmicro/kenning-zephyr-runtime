/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_KENNING_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_KENNING_H_

#include <zephyr/llext/symbol.h>

/* Kenning Zephyr Runtime exports */
#include <kenning_inference_lib/core/inference_server.h>
#include <kenning_inference_lib/core/loaders.h>
#include <kenning_inference_lib/core/runtime_wrapper.h>

extern model_spec_t g_model_spec;

EXPORT_SYMBOL(g_model_spec);

EXPORT_SYMBOL(g_ldr_tables);

EXPORT_SYMBOL(buf_save);
EXPORT_SYMBOL(buf_save_one);
EXPORT_SYMBOL(buf_reset);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_KENNING_H_