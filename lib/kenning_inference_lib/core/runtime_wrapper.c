/*
 * Copyright (c) 2025 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "kenning_inference_lib/core/runtime_wrapper.h"
#include "kenning_inference_lib/core/loaders.h"
#include <string.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(runtime_wrapper, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

/**
 * We are using macros defined in runtime_wrapper.h,
 * to generate definitions for the model_spec_input_length and model_spec_output_length functions.
 * We use macros since these functions are almost identical - more information in the runtime_wrapper.h file.
 */
GENERATE_MODEL_SPEC_LENGTH_FUNCTIONS(GENERATE_MODEL_SPEC_LENGTH_FUNCTION_DEFINITION);
