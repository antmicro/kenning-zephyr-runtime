/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_ZEPHYR_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_ZEPHYR_H_

#include <zephyr/llext/symbol.h>

/* Various configuration dependant exports */
#if defined(CONFIG_TIMING_FUNCTIONS)
#include <zephyr/timing/timing.h>
EXPORT_SYMBOL(arch_timing_counter_get);
EXPORT_SYMBOL(arch_timing_cycles_get);
EXPORT_SYMBOL(arch_timing_cycles_to_ns);
EXPORT_SYMBOL(timing_init);
EXPORT_SYMBOL(timing_start);
EXPORT_SYMBOL(timing_stop);
#endif

#if defined(CONFIG_TEST_RANDOM_GENERATOR)
#include <zephyr/random/random.h>
EXPORT_SYMBOL(z_impl_sys_rand_get);
#endif // CONFIG_TIMER_RANDOM_GENERATOR

#if defined(CONFIG_REBOOT)
#include <zephyr/sys/reboot.h>
EXPORT_SYMBOL(sys_reboot);
#endif // CONFIG_REBOOT

/* Other Zephyr-related exports */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

EXPORT_SYMBOL(k_heap_alloc);
EXPORT_SYMBOL(k_heap_free);
EXPORT_SYMBOL(k_heap_init);
EXPORT_SYMBOL(sys_heap_init);
EXPORT_SYMBOL(sys_heap_usable_size);

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_ZEPHYR_H_