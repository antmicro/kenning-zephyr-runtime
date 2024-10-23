# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(kenning_inference_lib)

macro(appendTVMModel runtime_src)
  if("${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND}")
    list(APPEND ${runtime_src} "runtimes/tvm/generated/magic_wand.c")
    set_source_files_properties("runtimes/tvm/generated/magic_wand.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
    add_compile_definitions(TVM_MODEL_MAGIC_WAND)
  elseif("${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND_INT8}")
    list(APPEND runtime_src "runtimes/tvm/generated/magic_wand_int8.c")
    set_source_files_properties("runtimes/tvm/generated/magic_wand_int8.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
  elseif("${CONFIG_KENNING_TVM_MODEL_GEN}")
    if("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
      message(FATAL_ERROR "Provide model path via CONFIG_KENNING_MODEL_PATH")
    endif("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")

    add_custom_command(
      OUTPUT
        runtimes/tvm/generated/model_impl.c
        runtimes/tvm/generated/model_impl.h
      COMMAND
        python3 ${CMAKE_SOURCE_DIR}/../scripts/gen_tvm_ops.py
          --input-path ${CONFIG_KENNING_MODEL_PATH}
          --output-path runtimes/tvm/generated/model_impl.graph_data
          --header-template-path ${CMAKE_CURRENT_SOURCE_DIR}/runtimes/tvm/generated/model_impl.h.template
          --board ${BOARD}
          ${CONFIG_KENNING_MODEL_COMPILER_ARGS}
    )
    # temporary fix for failing vrinta instruction in Renode simulation
    # it removes roundf that is applied after softmax (the output is cast
    # into uint after that, so this should not break anything)
    add_custom_command(
      OUTPUT
        "runtimes/tvm/generated/model_impl_no_round.c"
      DEPENDS
        "runtimes/tvm/generated/model_impl.c"
      COMMAND
        sed -e 's/roundf\(\(/\(\(/g' "runtimes/tvm/generated/model_impl.c" > "runtimes/tvm/generated/model_impl_no_round.c"
    )
    list(APPEND runtime_src "runtimes/tvm/generated/model_impl_no_round.c")
    set_source_files_properties("runtimes/tvm/generated/model_impl_no_round.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")

    add_compile_definitions(TVM_MODEL_MODEL_IMPL)
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/runtimes/tvm/)
  else()
    message(FATAL_ERROR "Invalid TVM model selected")
  endif()
endmacro()

# if model path starts with "./" make it absolute
if (CONFIG_KENNING_MODEL_PATH MATCHES "^\./.*")
  set(CONFIG_KENNING_MODEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../${CONFIG_KENNING_MODEL_PATH}")
endif (CONFIG_KENNING_MODEL_PATH MATCHES "^\./.*")

# sometimes generated ARM instructions in C source are uppercase while compiler expects lowercase
add_compile_definitions(__SEL=__sel)
add_compile_definitions(__SSUB8=__ssub8)
add_compile_definitions(__SMLAD=__smlad)

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
  appendTVMModel(runtime_src)
elseif(${CONFIG_KENNING_ML_RUNTIME_TFLITE})
  list(APPEND runtime_src "runtimes/tflite/tflite.cpp")
  list(APPEND runtime_src "runtimes/tflite/tflite.c")
  list(APPEND runtime_src "runtimes/tflite/resolver.cpp")
elseif(${CONFIG_KENNING_ML_RUNTIME_IREE})
  set(IREE_DIR ${ZEPHYR_CURRENT_MODULE_DIR}/../iree)
  set(ZEPHYR_CONFIG_HEADER "${IREE_DIR}/zephyr/zephyr_config.h")

  if("${CONFIG_ARM64}")
    add_compile_definitions(IREE_ARCH_ARM_64)
  elseif("${CONFIG_ARM}")
    add_compile_definitions(IREE_ARCH_ARM_32)
  elseif("${CONFIG_RISCV}" AND "${CONFIG_64BIT}")
    add_compile_definitions(IREE_ARCH_RISCV_64)
  elseif("${CONFIG_RISCV}")
    add_compile_definitions(IREE_ARCH_RISCV_32)
  elseif("${CONFIG_X86}" AND "${CONFIG_64BIT}")
    add_compile_definitions(IREE_ARCH_X86_64)
  elseif("${CONFIG_X86}")
    add_compile_definitions(IREE_ARCH_X86_32)
  else()
    message(FATAL_ERROR "unsupported architecture")
  endif()

  add_compile_definitions(FLATCC_USE_GENERIC_ALIGNED_ALLOC)
  add_compile_definitions(IREE_FILE_IO_ENABLE=0)
  add_compile_definitions(IREE_PLATFORM_GENERIC=1)
  add_compile_definitions(IREE_SYNCHRONIZATION_DISABLE_UNSAFE=1)
  add_compile_definitions(IREE_USER_CONFIG_H="${ZEPHYR_CONFIG_HEADER}")
  add_compile_definitions(IREE_VM_EXECUTION_TRACING_ENABLE=1)

  list(APPEND runtime_src "runtimes/iree/iree.c")
  list(APPEND runtime_src "runtimes/iree/platform.c")

  if("${CONFIG_KENNING_IREE_LOADER_VMVX}")
    list(APPEND runtime_src "runtimes/iree/device_vmvx.c")
  elseif("${CONFIG_KENNING_IREE_LOADER_EMBEDDED_ELF}")
    list(APPEND runtime_src "runtimes/iree/device_embedded_elf.c")
  elseif()
    message(FATAL_ERROR "Invalid IREE loader")
  endif()
elseif(${CONFIG_KENNING_ML_RUNTIME_LLEXT})
  list(APPEND runtime_src "runtimes/llext/llext.c")
endif()

zephyr_library_sources(${core_src} ${protocol_src} ${runtime_src})

if(CONFIG_LLEXT)
  if(CONFIG_ZEPHYR_TVM_MODULE)
    set(tvm_runtime_src "")
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/common/crt_backend_api.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/common/crt_runtime_api.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/common/func_registry.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/common/ndarray.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/common/packed_func.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/graph_executor/graph_executor.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/graph_executor/load_json.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/memory/page_allocator.c)
    list(APPEND tvm_runtime_src ${ZEPHYR_TVM_MODULE_DIR}/src/runtime/crt/memory/stack_allocator.c)

    list(APPEND tvm_runtime_src "runtimes/tvm/tvm.c")
    list(APPEND tvm_runtime_src "runtimes/tvm/platform.c")

    appendTVMModel(tvm_runtime_src)

    add_compile_definitions(EXTENSION_TVM)

    add_llext_target(
      "llext-tvm"
      OUTPUT "${CMAKE_BINARY_DIR}/llext/tvm.llext"
      SOURCES "${tvm_runtime_src}"
    )

    llext_compile_options("llext-tvm" -mlong-calls)
    llext_compile_options("llext-tvm" -Os)

    add_dependencies(app llext-tvm)
  endif(CONFIG_ZEPHYR_TVM_MODULE)
endif(CONFIG_LLEXT)
