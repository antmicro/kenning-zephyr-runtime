/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_ARM_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_ARM_H_

#include <zephyr/llext/symbol.h>

/* ARM architecture-specific exports */
#if defined(CONFIG_ARM)
extern void(__aeabi_d2lz)(void);
extern void(__aeabi_dadd)(void);
extern void(__aeabi_dcmpeq)(void);
extern void(__aeabi_dcmpge)(void);
extern void(__aeabi_dcmpgt)(void);
extern void(__aeabi_dcmple)(void);
extern void(__aeabi_dcmpun)(void);
extern void(__aeabi_ddiv)(void);
extern void(__aeabi_dmul)(void);
extern void(__aeabi_i2d)(void);
extern void(__aeabi_ldivmod)(void);
extern void(__aeabi_ul2d)(void);
extern void(__assert_func)(void);

EXPORT_SYMBOL(__aeabi_d2lz);
EXPORT_SYMBOL(__aeabi_dadd);
EXPORT_SYMBOL(__aeabi_dcmpeq);
EXPORT_SYMBOL(__aeabi_dcmpge);
EXPORT_SYMBOL(__aeabi_dcmpgt);
EXPORT_SYMBOL(__aeabi_dcmple);
EXPORT_SYMBOL(__aeabi_dcmpun);
EXPORT_SYMBOL(__aeabi_ddiv);
EXPORT_SYMBOL(__aeabi_dmul);
EXPORT_SYMBOL(__aeabi_i2d);
EXPORT_SYMBOL(__aeabi_ldivmod);
EXPORT_SYMBOL(__aeabi_ul2d);
#endif // CONFIG_ARM

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_ARM_H_