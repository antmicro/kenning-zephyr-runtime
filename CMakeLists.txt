# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(include)

set(KENNING_MODULE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "KENNING_MODULE_BINARY_DIR")

include(cmake/board_repl.cmake)
include(cmake/gen_model_header.cmake)

if (CONFIG_ZTEST)
  add_compile_definitions(__UNIT_TEST__=1)
endif (CONFIG_ZTEST)

add_subdirectory(lib)
