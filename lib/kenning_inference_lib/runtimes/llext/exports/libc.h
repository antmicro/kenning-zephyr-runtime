/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_NEWLIB_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_NEWLIB_H_

#include <zephyr/llext/symbol.h>

/* Newlib implementation-specific exports */
#if defined(CONFIG_NEWLIB_LIBC)
#include <zephyr/sys/printk.h>
extern const char *_ctype_;
extern void(__assert_func)(void);
extern int(vsnprintk)(char *, unsigned int, const char *, va_list);

EXPORT_SYMBOL(vsnprintk);
EXPORT_SYMBOL(__assert_func);
EXPORT_SYMBOL(_ctype_);
EXPORT_SYMBOL(_impure_ptr);
#endif // CONFIG_NEWLIB_LIBC

/* Standard library exports */
#include <math.h>
#include <stdio.h>
#include <string.h>

EXPORT_SYMBOL(puts);
EXPORT_SYMBOL(snprintf);
EXPORT_SYMBOL(strtol);
EXPORT_SYMBOL(strtoul);
EXPORT_SYMBOL(vfprintf);
EXPORT_SYMBOL(vsnprintf);
EXPORT_SYMBOL(fputs);
EXPORT_SYMBOL(expf);
EXPORT_SYMBOL(fclose);
EXPORT_SYMBOL(fprintf);
EXPORT_SYMBOL(fwrite);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_LIBC_H_