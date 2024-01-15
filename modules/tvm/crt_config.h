/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MODULES_TVM_CRT_CONFIG_H_
#define MODULES_TVM_CRT_CONFIG_H_

/* Log level of the CRT runtime */
#define TVM_CRT_LOG_LEVEL TVM_CRT_LOG_LEVEL_DEBUG

/* Support low-level debugging in MISRA-C runtime */
#define TVM_CRT_DEBUG 0

/* Maximum supported dimension in NDArray */
#define TVM_CRT_MAX_NDIM 6

/* Maximum supported arguments in generated functions */
#define TVM_CRT_MAX_ARGS 10

/* Size of the global function registry, in bytes. */
#define TVM_CRT_GLOBAL_FUNC_REGISTRY_SIZE_BYTES 512

/* Maximum number of registered modules. */
#define TVM_CRT_MAX_REGISTERED_MODULES 2

/* Maximum packet size, in bytes, including the length header. */
#define TVM_CRT_MAX_PACKET_SIZE_BYTES 512

/* Maximum supported string length in dltype, e.g. "int8", "int16", "float32" */
#define TVM_CRT_MAX_STRLEN_DLTYPE 10

/* Maximum supported string length in function names */
#define TVM_CRT_MAX_STRLEN_FUNCTION_NAME 120

/* Maximum supported string length in parameter names */
#define TVM_CRT_MAX_STRLEN_PARAM_NAME 80

#endif // MODULES_TVM_CRT_CONFIG_H_
