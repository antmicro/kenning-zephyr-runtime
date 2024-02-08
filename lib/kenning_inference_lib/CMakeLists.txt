# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

file(GLOB core_src core/*.c)
if("${CONFIG_PROTOCOL}" STREQUAL "uart")
  set(protocol_src protocols/uart.c)
elseif("${CONFIG_PROTOCOL}" STREQUAL "none")
  message(WARNING "Communication with Kenning disabled")
  list(FILTER core_src EXCLUDE REGEX ".*/core/callbacks\.c")
  list(FILTER core_src EXCLUDE REGEX ".*/core/inference_server\.c")
  list(FILTER core_src EXCLUDE REGEX ".*/core/kenning_protocol\.c")
  add_compile_definitions(NO_KENNING_COMM)
else()
  message(FATAL_ERROR "Protocol ${CONFIG_PROTOCOL} not supported")
endif()

if("${CONFIG_ML_RUNTIME}" STREQUAL "stub")
  file(GLOB runtime_src runtimes/stub/*.c)
elseif("${CONFIG_ML_RUNTIME}" STREQUAL "tvm")
  set(runtime_src "")
  list(APPEND runtime_src "runtimes/tvm/tvm.c")
  list(APPEND runtime_src "runtimes/tvm/platform.c")

  if("${CONFIG_TVM_MODEL}" STREQUAL "magic_wand")
    list(APPEND runtime_src "runtimes/tvm/generated/magic_wand.c")
    add_compile_definitions(TVM_MODEL_MAGIC_WAND)
  else()
    message(FATAL_ERROR "Invalid TVM model selected: ${CONFIG_TVM_MODEL}.")
  endif()
elseif("${CONFIG_ML_RUNTIME}" STREQUAL "tflite")
  set(runtime_src "")
  list(APPEND runtime_src "runtimes/tflite/tflite.cpp")
  list(APPEND runtime_src "runtimes/tflite/tflite.c")
  list(APPEND runtime_src "runtimes/tflite/resolver.cpp")
else()
  message(FATAL_ERROR "Invalid ML runtime selected: ${CONFIG_ML_RUNTIME}.")
endif()

zephyr_library_sources(${core_src} ${protocol_src} ${runtime_src})
