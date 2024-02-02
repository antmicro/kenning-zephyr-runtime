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
elseif(${CONFIG_KENNING_ML_RUNTIME_IREE})
  set(IREE_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../iree)
  set(ZEPHYR_CONFIG_HEADER "${IREE_DIR}/zephyr/zephyr_config.h")

  if("${CONFIG_ARM64}")
    add_compile_definitions(-DIREE_ARCH_ARM_64)
  elseif("${CONFIG_ARM}")
    add_compile_definitions(-DIREE_ARCH_ARM_32)
  elseif("${CONFIG_RISCV}" AND "${CONFIG_64BIT}")
    add_compile_definitions(-DIREE_ARCH_RISCV_64)
  elseif("${CONFIG_RISCV}")
    add_compile_definitions(-DIREE_ARCH_RISCV_32)
  elseif("${CONFIG_X86}" AND "${CONFIG_64BIT}")
    add_compile_definitions(-DIREE_ARCH_X86_64)
  elseif("${CONFIG_X86}")
    add_compile_definitions(-DIREE_ARCH_X86_32)
  else()
    message(FATAL_ERROR "unsupported architecture")
  endif()

  add_compile_definitions(-DFLATCC_USE_GENERIC_ALIGNED_ALLOC)
  add_compile_definitions(-DIREE_FILE_IO_ENABLE=0)
  add_compile_definitions(-DIREE_PLATFORM_GENERIC=1)
  add_compile_definitions(-DIREE_SYNCHRONIZATION_DISABLE_UNSAFE=1)
  add_compile_definitions(-DIREE_USER_CONFIG_H="${ZEPHYR_CONFIG_HEADER}")
  add_compile_definitions(-DIREE_VM_EXECUTION_TRACING_ENABLE=1)

  list(APPEND runtime_src "runtimes/iree/iree.c")
  list(APPEND runtime_src "runtimes/iree/platform.c")

  if("${CONFIG_KENNING_IREE_LOADER_VMVX}")
    list(APPEND runtime_src "runtimes/iree/device_vmvx.c")
  elseif("${CONFIG_KENNING_IREE_LOADER_EMBEDDED_ELF}")
    list(APPEND runtime_src "runtimes/iree/device_embedded_elf.c")
  elseif()
    message(FATAL_ERROR "Invalid IREE loader")
  endif()
endif()

zephyr_library_sources(${core_src} ${protocol_src} ${runtime_src})
