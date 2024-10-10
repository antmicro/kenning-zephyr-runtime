/*
 * Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_KENNING_INFERENCE_LIB_MOCKS_LLEXT_H_
#define TESTS_KENNING_INFERENCE_LIB_MOCKS_LLEXT_H_

#include <stddef.h>

#define LLEXT_BUF_LOADER(...) \
    {                         \
    }
#define LLEXT_LOAD_PARAM_DEFAULT \
    {                            \
    }

struct llext_loader
{
};

struct llext_buf_loader
{
    struct llext_loader loader;
};

struct llext_load_param
{
};

struct llext_symbol
{
    const char *name;
    void *addr;
};

struct llext_symtable
{
    size_t sym_cnt;
    struct llext_symbol *syms;
};

struct llext
{
    struct llext_symtable sym_tab, exp_tab;
};

struct llext *llext_by_name(char *);
int llext_unload(struct llext **);
int llext_load(struct llext_loader *, const char *, struct llext **, struct llext_load_param *);
int llext_teardown(struct llext *);
int llext_bringup(struct llext *);

#endif // TESTS_KENNING_INFERENCE_LIB_MOCKS_LLEXT_H_
