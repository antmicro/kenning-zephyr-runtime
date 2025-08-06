# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

# adds possibility to increase simulated board memory
include(../cmake/increase_board_memory.cmake)
kenning_increase_board_memory()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(demo_app LANGUAGES C)

# if model path starts with "./" make it absolute
if(CONFIG_KENNING_MODEL_PATH MATCHES "^\./.*")
  set(CONFIG_KENNING_MODEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../${CONFIG_KENNING_MODEL_PATH}")
endif(CONFIG_KENNING_MODEL_PATH MATCHES "^\./.*")

kenning_gen_model_header("${CMAKE_CURRENT_BINARY_DIR}/src/model_data.h")
kenning_add_board_repl_target()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/src")
target_sources(app PRIVATE src/main.c)
