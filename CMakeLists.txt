# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set_property(GLOBAL PROPERTY CSTD gnu11)

zephyr_include_directories(include)

if (CONFIG_ZTEST)
  add_compile_definitions(__UNIT_TEST__=1)
endif (CONFIG_ZTEST)

add_subdirectory(lib)
