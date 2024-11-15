/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_LIBCPP_H_
#define KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_LIBCPP_H_

/* C++ exports */
#if defined(CONFIG_GLIBCXX_LIBCPP)

#define CPP_EXPORT_SYMBOL(x) \
    extern void(x)(void);    \
    EXPORT_SYMBOL(x);

CPP_EXPORT_SYMBOL(__cxa_guard_abort)
CPP_EXPORT_SYMBOL(__cxa_guard_acquire)
CPP_EXPORT_SYMBOL(__cxa_guard_release)
CPP_EXPORT_SYMBOL(_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE6insertEjPKc)
CPP_EXPORT_SYMBOL(_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_disposeEv)
CPP_EXPORT_SYMBOL(_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE7_S_copyEPcPKcj)
CPP_EXPORT_SYMBOL(_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE12_M_constructEjc)
CPP_EXPORT_SYMBOL(_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcj)
CPP_EXPORT_SYMBOL(_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE10_M_replaceEjjPKcj)
CPP_EXPORT_SYMBOL(_ZSt20__throw_length_errorPKc)
CPP_EXPORT_SYMBOL(_ZdlPvj)
CPP_EXPORT_SYMBOL(__clrsbsi2)
CPP_EXPORT_SYMBOL(__dso_handle)

#undef CPP_EXPORT_SYMBOL

#endif // CONFIG_GLIBCXX_LIBCPP

#endif // KENNING_INFERENCE_LIB_RUNTIMES_LLEXT_EXPORTS_LIBCPP_H_