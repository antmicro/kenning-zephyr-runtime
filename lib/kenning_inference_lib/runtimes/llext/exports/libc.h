/*
 * Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
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
extern void(__assert_func)(const char *, int, const char *, const char *);
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

EXPORT_SYMBOL(abort);
EXPORT_SYMBOL(bsearch);
EXPORT_SYMBOL(cos);
EXPORT_SYMBOL(cosf);
EXPORT_SYMBOL(exp);
EXPORT_SYMBOL(expf);
EXPORT_SYMBOL(expm1);
EXPORT_SYMBOL(fclose);
EXPORT_SYMBOL(floor);
EXPORT_SYMBOL(fmodf);
EXPORT_SYMBOL(fprintf);
EXPORT_SYMBOL(fputs);
EXPORT_SYMBOL(free);
EXPORT_SYMBOL(frexp);
EXPORT_SYMBOL(fwrite);
EXPORT_SYMBOL(logf);
EXPORT_SYMBOL(malloc);
EXPORT_SYMBOL(memcmp);
EXPORT_SYMBOL(memcpy);
EXPORT_SYMBOL(memmove);
EXPORT_SYMBOL(memset);
EXPORT_SYMBOL(pow);
EXPORT_SYMBOL(puts);
EXPORT_SYMBOL(realloc);
EXPORT_SYMBOL(round);
EXPORT_SYMBOL(sin);
EXPORT_SYMBOL(sinf);
EXPORT_SYMBOL(snprintf);
EXPORT_SYMBOL(sqrt);
EXPORT_SYMBOL(sqrtf);
EXPORT_SYMBOL(strcmp);
EXPORT_SYMBOL(strlen);
EXPORT_SYMBOL(strncmp);
EXPORT_SYMBOL(strtol);
EXPORT_SYMBOL(strtoll);
EXPORT_SYMBOL(strtoul);
EXPORT_SYMBOL(strtoull);
EXPORT_SYMBOL(tanhf);
EXPORT_SYMBOL(vfprintf);
EXPORT_SYMBOL(vsnprintf);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_LIBC_H_
