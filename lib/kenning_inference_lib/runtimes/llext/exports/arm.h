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

#define AEABI_EXPORT_SYMBOL(x) \
    extern void(x)(void);      \
    EXPORT_SYMBOL(x);

AEABI_EXPORT_SYMBOL(__aeabi_atexit)
AEABI_EXPORT_SYMBOL(__aeabi_d2f)
AEABI_EXPORT_SYMBOL(__aeabi_d2iz)
AEABI_EXPORT_SYMBOL(__aeabi_d2lz)
AEABI_EXPORT_SYMBOL(__aeabi_d2ulz)
AEABI_EXPORT_SYMBOL(__aeabi_dadd)
AEABI_EXPORT_SYMBOL(__aeabi_dcmpeq)
AEABI_EXPORT_SYMBOL(__aeabi_dcmpge)
AEABI_EXPORT_SYMBOL(__aeabi_dcmpgt)
AEABI_EXPORT_SYMBOL(__aeabi_dcmple)
AEABI_EXPORT_SYMBOL(__aeabi_dcmplt)
AEABI_EXPORT_SYMBOL(__aeabi_dcmpun)
AEABI_EXPORT_SYMBOL(__aeabi_ddiv)
AEABI_EXPORT_SYMBOL(__aeabi_dmul)
AEABI_EXPORT_SYMBOL(__aeabi_dsub)
AEABI_EXPORT_SYMBOL(__aeabi_f2d)
AEABI_EXPORT_SYMBOL(__aeabi_i2d)
AEABI_EXPORT_SYMBOL(__aeabi_l2d)
AEABI_EXPORT_SYMBOL(__aeabi_ldivmod)
AEABI_EXPORT_SYMBOL(__aeabi_ui2d)
AEABI_EXPORT_SYMBOL(__aeabi_ul2d)
AEABI_EXPORT_SYMBOL(__aeabi_uldivmod)

#undef AEABI_EXPORT_SYMBOL

#endif // CONFIG_ARM

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_ARM_H_