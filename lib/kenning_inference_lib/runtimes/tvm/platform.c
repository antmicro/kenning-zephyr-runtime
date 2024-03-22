/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlpack/dlpack.h>
#include <stdint.h>
#include <tvm/runtime/crt/error_codes.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/timing/timing.h>

#include "tvm.h"

LOG_MODULE_REGISTER(tvm_platform, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

K_HEAP_DEFINE(tvm_heap, 1024 * CONFIG_KENNING_TVM_HEAP_SIZE);

volatile timing_t g_microtvm_start_time, g_microtvm_end_time;
int g_microtvm_timer_running = 0;

static uint64_t total_allocated = 0;
static uint64_t total_freed = 0;
static uint64_t peak_allocated = 0;

void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *esf)
{
    for (;;)
        ;
}

void TVMPlatformAbort(tvm_crt_error_t error)
{
    LOG_ERR("TVM error: 0x%x", error);
#ifdef CONFIG_REBOOT
    sys_reboot(SYS_REBOOT_COLD);
#endif // CONFIG_REBOOT
    for (;;)
        ;
}

size_t TVMPlatformFormatMessage(char *out_buf, size_t out_buf_size_bytes, const char *fmt, va_list args)
{
    return vsnprintk(out_buf, out_buf_size_bytes, fmt, args);
}

tvm_crt_error_t TVMPlatformMemoryAllocate(size_t num_bytes, DLDevice dev, void **out_ptr)
{
    LOG_DBG("TVM alloc: %zu", num_bytes);
    if (0 == num_bytes)
    {
        *out_ptr = NULL;
        return kTvmErrorNoError;
    }
    *out_ptr = k_heap_alloc(&tvm_heap, num_bytes, K_NO_WAIT);
    if (*out_ptr != NULL)
    {
        total_allocated += sys_heap_usable_size(&(tvm_heap.heap), *out_ptr);
        if (total_allocated - total_freed > peak_allocated)
        {
            peak_allocated = total_allocated - total_freed;
        }
    }
    return (*out_ptr == NULL) ? kTvmErrorPlatformNoMemory : kTvmErrorNoError;
}

tvm_crt_error_t TVMPlatformMemoryFree(void *ptr, DLDevice dev)
{
    if (NULL == ptr)
    {
        return kTvmErrorNoError;
    }
    total_freed += sys_heap_usable_size(&(tvm_heap.heap), ptr);
    k_heap_free(&tvm_heap, ptr);
    return kTvmErrorNoError;
}

tvm_crt_error_t TVMPlatformTimerStart()
{
    if (g_microtvm_timer_running)
    {
        LOG_WRN("TVM timer already running");
        return kTvmErrorPlatformTimerBadState;
    }

    g_microtvm_start_time = timing_counter_get();
    g_microtvm_timer_running = 1;
    return kTvmErrorNoError;
}

tvm_crt_error_t TVMPlatformTimerStop(double *elapsed_time_seconds)
{
    if (!g_microtvm_timer_running)
    {
        LOG_WRN("TVM timer not running");
        return kTvmErrorSystemErrorMask | 2;
    }

    g_microtvm_end_time = timing_counter_get();
    uint64_t cycles = timing_cycles_get(&g_microtvm_start_time, &g_microtvm_end_time);
    uint64_t ns_spent = timing_cycles_to_ns(cycles);
    *elapsed_time_seconds = ns_spent / (double)1e9;
    g_microtvm_timer_running = 0;
    return kTvmErrorNoError;
}

tvm_crt_error_t TVMPlatformGenerateRandom(uint8_t *buffer, size_t num_bytes)
{
    uint32_t random;

    // Fill parts of `buffer` which are as large as `random`.
    size_t num_full_blocks = num_bytes / sizeof(random);
    for (int i = 0; i < num_full_blocks; ++i)
    {
        random = sys_rand32_get();
        memcpy(&buffer[i * sizeof(random)], &random, sizeof(random));
    }

    // Fill any leftover tail which is smaller than `random`.
    size_t num_tail_bytes = num_bytes % sizeof(random);
    if (num_tail_bytes > 0)
    {
        random = sys_rand32_get();
        memcpy(&buffer[num_bytes - num_tail_bytes], &random, num_tail_bytes);
    }
    return kTvmErrorNoError;
}

tvm_crt_error_t TVMPlatformInitialize()
{
    timing_init();
    timing_start();

    return kTvmErrorNoError;
}

void tvm_get_allocation_stats(tvm_alloc_stats_t *tvm_alloc_stats)
{
    tvm_alloc_stats->total_allocated = total_allocated;
    tvm_alloc_stats->total_freed = total_freed;
    tvm_alloc_stats->peak_allocated = peak_allocated;
}
