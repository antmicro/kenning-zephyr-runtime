# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

FILE(GLOB core_src core/*.c)
SET(protocol_src protocols/uart.c)
FILE(GLOB runtime_src runtimes/stub/*.c)

zephyr_library_sources(${core_src} ${protocol_src} ${runtime_src})
