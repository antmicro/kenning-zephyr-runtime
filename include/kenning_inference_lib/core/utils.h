/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_CORE_UTILS_H_
#define KENNING_INFERENCE_LIB_CORE_UTILS_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __UNIT_TEST__
#define ut_static static
#else // __UNIT_TEST__
#define ut_static
#endif // __UNIT_TEST__

/* checks if pointer is valid */
#define IS_VALID_POINTER(ptr) (NULL != (ptr))

/* frees a pointer if not NULL and sets it to NULL */
#define FREE_POINTER(ptr) \
    if (NULL != (ptr))    \
    {                     \
        free(ptr);        \
        ptr = NULL;       \
    }

/* checks if pointer is NULL and returns error when it is true */
#define RETURN_ERROR_IF_POINTER_INVALID(ptr, error_status) \
    if (!IS_VALID_POINTER(ptr))                            \
    {                                                      \
        return error_status;                               \
    }

/* returns on error */
#define RETURN_ON_ERROR(status, err_code) \
    if (STATUS_OK != (status))            \
    {                                     \
        return err_code;                  \
    }

/* breaks loop on error */
#define BREAK_ON_ERROR(status) \
    if (STATUS_OK != (status)) \
    {                          \
        break;                 \
    }

/* breaks loop on error and logs provided message */
#define BREAK_ON_ERROR_LOG(status, log_format, ...) \
    if (STATUS_OK != (status))                      \
    {                                               \
        LOG_ERR(log_format, __VA_ARGS__);           \
        break;                                      \
    }

/* breaks loop when `cond` is true and logs provided message */
#define BREAK_ON_TRUE_LOG(cond, log_format, ...) \
    if ((cond))                                  \
    {                                            \
        LOG_ERR(log_format, __VA_ARGS__);        \
        break;                                   \
    }

/* breaks loop when `cond` is true, logs provided message  and sets `status` to `error_code` */
#define BREAK_ON_TRUE_LOG_SET_STATUS(status, error_code, cond, log_format, ...) \
    if ((cond))                                                                 \
    {                                                                           \
        status = error_code;                                                    \
        LOG_ERR(log_format, __VA_ARGS__);                                       \
        break;                                                                  \
    }

/* CSRs addresses */
#define CSR_CYCLE (0xC00)
#define CSR_TIME (0xC01)

#ifndef __UNIT_TEST__
#define CSR_READ(v, csr) __asm__ __volatile__("csrr %0, %1" : "=r"(v) : "n"(csr) : /* clobbers: none */);
#else // __UNIT_TEST__
#define CSR_READ(v, csr)                      \
    do                                        \
    {                                         \
        extern uint32_t g_mock_csr;           \
        extern void mock_csr_read_callback(); \
        (v) = g_mock_csr;                     \
        mock_csr_read_callback();             \
    } while (0);
#endif // __UNIT_TEST__

#define TIMER_CLOCK_FREQ (24000000u) /* 24 MHz */

#define GENERATE_ENUM(enum, ...) enum,
#define GENERATE_STR(str, ...) #str,

#define INT_TO_BOOL(x) (!!(x))
/* extracts least significant one */
#define LS_ONE(x) ((x) & ((x) ^ ((x)-1)))
/* counts tailing zeros */
#define TR_ZEROS(x)                                                                        \
    (INT_TO_BOOL(LS_ONE(x) & 0xFFFF0000) << 4 | INT_TO_BOOL(LS_ONE(x) & 0xFF00FF00) << 3 | \
     INT_TO_BOOL(LS_ONE(x) & 0xF0F0F0F0) << 2 | INT_TO_BOOL(LS_ONE(x) & 0xCCCCCCCC) << 1 | \
     INT_TO_BOOL(LS_ONE(x) & 0xAAAAAAAA))
/* performs masked or operation */
#define MASKED_OR_32(a, b, mask) (((a) & (0xFFFFFFFF ^ (mask))) | ((b) & (mask)))

/* retrieves field offset from register field mask */
#define GET_OFFSET(field) TR_ZEROS(field)
/* extracts register field value */
#define GET_REG_FIELD(var, field) (((var) & (field)) >> GET_OFFSET(field))
/* sets register field value */
#define SET_REG_FIELD(var, field, value) (MASKED_OR_32((var), (value) << GET_OFFSET(field), (field)))

#define STATUS_MASK_MODULE 0xFF00
#define STATUS_MASK_CODE 0xFF
#define GENERATE_ERROR(module, status)                                   \
    ((((module) << TR_ZEROS(STATUS_MASK_MODULE)) & STATUS_MASK_MODULE) | \
     ((((status_t)(status)) << TR_ZEROS(STATUS_MASK_CODE)) & STATUS_MASK_CODE))
#define GET_STATUS_MODULE(status) GET_REG_FIELD(status, STATUS_MASK_MODULE)
#define GET_STATUS_CODE(status) GET_REG_FIELD(status, STATUS_MASK_CODE)
/* generates enum with module statuses. First status module##_STATUS_OK is added
   only to set enum counter value and should not be used. The generated enum
   contains generic statuses from GENERIC_STATUSES defined below and custom
   statuses module##_STATUSES that are defined in each modules header. The
   module##_LAST_STATUS is only to get number of statuses and should not be used
 */
#define GENERATE_MODULE_STATUSES(module)                                                              \
    typedef enum                                                                                      \
    {                                                                                                 \
        module##_STATUS_OK = GENERATE_ERROR(module, STATUS_OK),                                       \
        GENERIC_STATUSES(GENERATE_ENUM, module) module##_STATUSES(GENERATE_ENUM) module##_LAST_STATUS \
    } module##_STATUS;

/* generates array of strings with modules statuses names */
#define GENERATE_MODULE_STATUSES_STR(module)                                                                \
    const char *const module##_STATUS_STR[] = {#module "_STATUS_OK", GENERIC_STATUSES(GENERATE_STR, module) \
                                                                         module##_STATUSES(GENERATE_STR)};  \
    const size_t module##_STATUS_COUNT = GET_STATUS_COUNT(module);

/* returns status count for given module */
#define GET_STATUS_COUNT(module) GET_STATUS_CODE(module##_LAST_STATUS)

/**
 * Modules
 */
#ifdef NO_KENNING_COMM
#define MODULES(MODULE) \
    MODULE(MODEL)       \
    MODULE(RUNTIME_WRAPPER)
#else // NO_KENNING_COMM
#define MODULES(MODULE)      \
    MODULE(CALLBACKS)        \
    MODULE(INFERENCE_SERVER) \
    MODULE(KENNING_PROTOCOL) \
    MODULE(MODEL)            \
    MODULE(PROTOCOL)         \
    MODULE(RUNTIME_WRAPPER)
#endif // NO_KENNING_COMM

/**
 * Enum with all modules
 */
enum
{
    SKIP_ZERO,
    MODULES(GENERATE_ENUM)
};

#define STATUS_OK 0 /* success */

/**
 * Generic error status
 */
#define GENERIC_STATUSES(STATUS, module)                       \
    STATUS(module##_STATUS_ERROR)   /* generic error */        \
    STATUS(module##_STATUS_INV_PTR) /* invalid pointer */      \
    STATUS(module##_STATUS_INV_ARG) /* invalid argument */     \
    STATUS(module##_STATUS_UNINIT)  /* module uninitialized */ \
    STATUS(module##_STATUS_TIMEOUT) /* timeout */

typedef int32_t status_t;

const char *get_status_str(status_t status);

#endif // KENNING_INFERENCE_LIB_CORE_UTILS_H_
