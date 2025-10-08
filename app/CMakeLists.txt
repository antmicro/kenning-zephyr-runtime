# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

# adds possibility to increase simulated board memory
include(../cmake/increase_board_memory.cmake)
kenning_increase_board_memory()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

kenning_add_board_repl_target()

target_sources(app PRIVATE src/main.c)
