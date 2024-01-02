/*
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_KENNING_INFERENCE_LIB_UTILS_H_
#define TESTS_KENNING_INFERENCE_LIB_UTILS_H_

#define DECLARE_VOID_MOCK(...) FAKE_VOID_FUNC(__VA_ARGS__);
#define DECLARE_MOCK(...) FAKE_VALUE_FUNC(__VA_ARGS__);
#define RESET_VOID_MOCK(name, ...) RESET_FAKE(name);
#define RESET_MOCK(ret_type, name, ...) RESET_FAKE(name);

#endif // TESTS_KENNING_INFERENCE_LIB_UTILS_H_
