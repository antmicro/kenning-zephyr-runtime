# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

file(GLOB core_src core/*.c)
if("${CONFIG_PROTOCOL}" STREQUAL "uart")
  set(protocol_src protocols/uart.c)
else()
  error("Protocol ${CONFIG_PROTOCOL} not supported")
endif()

if("${CONFIG_ML_RUNTIME}" STREQUAL "stub")
  FILE(GLOB runtime_src runtimes/stub/*.c)
elseif("${CONFIG_ML_RUNTIME}" STREQUAL "tvm")
  set(runtime_src "")
  list(APPEND runtime_src "runtimes/tvm/tvm.c")
  list(APPEND runtime_src "runtimes/tvm/platform.c")

  if("${CONFIG_TVM_MODEL}" STREQUAL "magic_wand")
    list(APPEND runtime_src "runtimes/tvm/generated/magic_wand.c")
    add_compile_definitions(TVM_MODEL_MAGIC_WAND)
  else()
    error("Invalid TVM model selected")
  endif()
else()
  message(FATAL_ERROR "Invalid ML runtime selected.")
endif()

if("${BOARD}" STREQUAL "stm32f746g_disco")
  add_compile_definitions(COMM_UART=usart6)
elseif("${BOARD}" STREQUAL "nrf52840dk_nrf52840")
  add_compile_definitions(COMM_UART=uart1)
else()
  message(FATAL_ERROR "Board ${BOARD} not supported")
endif()

zephyr_library_sources(${core_src} ${protocol_src} ${runtime_src})
