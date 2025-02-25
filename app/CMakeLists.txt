# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

kenning_add_board_repl_target()

add_dependencies(app board-repl)

target_sources(app PRIVATE src/main.c)
