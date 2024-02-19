# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

set(core_src "")
list(APPEND core_src "core/model.c")
list(APPEND core_src "core/utils.c")
if(${CONFIG_KENNING_COMMUNICATION_PROTOCOL_NONE})
  message(WARNING "Communication with Kenning disabled")
  add_compile_definitions(NO_KENNING_COMM)
else()
  list(APPEND core_src "core/callbacks.c")
  list(APPEND core_src "core/inference_server.c")
  list(APPEND core_src "core/kenning_protocol.c")
endif()

set(protocol_src "")
if(${CONFIG_KENNING_COMMUNICATION_PROTOCOL_UART})
  list(APPEND protocol_src "protocols/uart.c")
endif()

set(runtime_src "")
if(${CONFIG_KENNING_ML_RUNTIME_STUB})
  list(APPEND runtime_src "runtimes/stub/stub.c")
elseif(${CONFIG_KENNING_ML_RUNTIME_TVM})
  list(APPEND runtime_src "runtimes/tvm/tvm.c")
  list(APPEND runtime_src "runtimes/tvm/platform.c")

  if("${CONFIG_KENNING_TVM_MODEL}" STREQUAL "magic_wand")
    list(APPEND runtime_src "runtimes/tvm/generated/magic_wand.c")
    set_source_files_properties("runtimes/tvm/generated/magic_wand.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
    add_compile_definitions(TVM_MODEL_MAGIC_WAND)
  elseif("${CONFIG_KENNING_TVM_MODEL}" STREQUAL "gen_model")
    add_custom_command(
      OUTPUT
        runtimes/tvm/generated/model_impl.c
        runtimes/tvm/generated/model_impl.h
      COMMAND
        python3 ${CMAKE_SOURCE_DIR}/../scripts/gen_tvm_ops.py
          --input-path ${CONFIG_KENNING_MODEL_PATH}
          --output-path runtimes/tvm/generated/model_impl.graph_data
          --template-header-path ${CMAKE_CURRENT_SOURCE_DIR}/runtimes/tvm/generated/model_impl.h.template
          --board ${BOARD}
          ${CONFIG_KENNING_MODEL_COMPILER_ARGS}
    )
    list(APPEND runtime_src "runtimes/tvm/generated/model_impl.c")
    set_source_files_properties("runtimes/tvm/generated/model_impl.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
    add_compile_definitions(TVM_MODEL_MODEL_IMPL)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/runtimes/tvm/)
  else()
    message(FATAL_ERROR "Invalid TVM model selected: ${CONFIG_KENNING_TVM_MODEL}.")
  endif()
elseif(${CONFIG_KENNING_ML_RUNTIME_TFLITE})
  list(APPEND runtime_src "runtimes/tflite/tflite.cpp")
  list(APPEND runtime_src "runtimes/tflite/tflite.c")
  list(APPEND runtime_src "runtimes/tflite/resolver.cpp")
endif()

zephyr_library_sources(${core_src} ${protocol_src} ${runtime_src})
