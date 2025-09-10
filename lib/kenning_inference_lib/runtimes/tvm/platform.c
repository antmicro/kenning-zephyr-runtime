/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "kenning_inference_lib/core/runtime_wrapper.h"
#include "tvm.h"

#include <dlpack/dlpack.h>
#include <stdint.h>
#include <tvm/runtime/crt/error_codes.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/timing/timing.h>

LOG_MODULE_REGISTER(tvm_platform, CONFIG_RUNTIME_WRAPPER_LOG_LEVEL);

#if defined(EXTENSION_TVM)
struct k_heap tvm_heap;

__attribute__((aligned(8))) uint8_t tvm_heap_data[1024 * CONFIG_KENNING_TVM_HEAP_SIZE];

void local_heap_init() { k_heap_init(&tvm_heap, tvm_heap_data, 1024 * CONFIG_KENNING_TVM_HEAP_SIZE); }

__attribute__((section(".init_array"))) void *init_array[] = {local_heap_init};
#else
K_HEAP_DEFINE(tvm_heap, 1024 * CONFIG_KENNING_TVM_HEAP_SIZE);
#endif

volatile timing_t g_microtvm_start_time, g_microtvm_end_time;
int g_microtvm_timer_running = 0;

static uint64_t g_total_allocated = 0;
static uint64_t g_total_freed = 0;
static uint64_t g_peak_allocated = 0;

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
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
        g_total_allocated += sys_heap_usable_size(&(tvm_heap.heap), *out_ptr);
        if (g_total_allocated - g_total_freed > g_peak_allocated)
        {
            g_peak_allocated = g_total_allocated - g_total_freed;
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
    g_total_freed += sys_heap_usable_size(&(tvm_heap.heap), ptr);
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

const uint8_t *tvm_graph_json_ptr(const tvm_graph_t *tvm_graph) { return tvm_graph->graph_data; }

const uint8_t *tvm_graph_params_ptr(const tvm_graph_t *tvm_graph)
{
    return &(tvm_graph->graph_data[tvm_graph->graph_json_size]);
}

void tvm_get_allocation_stats(runtime_statistics_allocation_t *tvm_alloc_stats)
{
    tvm_alloc_stats->total_allocated = g_total_allocated;
    tvm_alloc_stats->total_freed = g_total_freed;
    tvm_alloc_stats->peak_allocated = g_peak_allocated;
}
