# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(kenning_inference_lib)

macro(appendTVMModel runtime_src)
  if("${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND}")
    list(APPEND ${runtime_src} "runtimes/tvm/generated/magic_wand.c")
    set_source_files_properties("runtimes/tvm/generated/magic_wand.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
    add_compile_definitions(TVM_MODEL_MAGIC_WAND)
  elseif("${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND_INT8}")
    list(APPEND ${runtime_src} "runtimes/tvm/generated/magic_wand_int8.c")
    set_source_files_properties("runtimes/tvm/generated/magic_wand_int8.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")
  elseif("${CONFIG_KENNING_TVM_MODEL_PRE_GEN}")
    get_filename_component(BASE_MODEL_PATH_DIR "${CONFIG_KENNING_MODEL_PATH}" DIRECTORY)
    get_filename_component(BASE_MODEL_PATH_NAME "${CONFIG_KENNING_MODEL_PATH}" NAME_WE)
    set(BASE_MODEL_PATH "${BASE_MODEL_PATH_DIR}/${BASE_MODEL_PATH_NAME}")
    add_custom_command(
      OUTPUT
        runtimes/tvm/generated/model_impl.c
        runtimes/tvm/generated/model_impl.h
      DEPENDS
        "${CONFIG_KENNING_MODEL_PATH}"
      COMMAND
        cp "${BASE_MODEL_PATH}.c" runtimes/tvm/generated/model_impl.c &&
        cp "${BASE_MODEL_PATH}.h" runtimes/tvm/generated/model_impl.h
    )
    list(APPEND ${runtime_src} "runtimes/tvm/generated/model_impl.c")
    set_source_files_properties("runtimes/tvm/generated/model_impl.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")

    include_directories(${CMAKE_CURRENT_BINARY_DIR}/runtimes/tvm/)
  elseif("${CONFIG_KENNING_TVM_MODEL_GEN}")
    if("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
      message(FATAL_ERROR "Provide model path via CONFIG_KENNING_MODEL_PATH")
    endif("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")

    add_custom_command(
      OUTPUT
        runtimes/tvm/generated/model_impl.c
        runtimes/tvm/generated/model_impl.h
      COMMAND
        python3 ${CMAKE_SOURCE_DIR}/../scripts/build_tvm.py
          --input-path ${CONFIG_KENNING_MODEL_PATH}
          --output-path runtimes/tvm/generated/model_impl.graph_data
          --header-template-path ${CMAKE_CURRENT_SOURCE_DIR}/runtimes/tvm/generated/model_impl.h.template
          --board ${BOARD}
          --conf '${CONFIG_KENNING_TVM_TARGET_ATTRS}'
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
    list(APPEND ${runtime_src} "runtimes/tvm/generated/model_impl_no_round.c")
    set_source_files_properties("runtimes/tvm/generated/model_impl_no_round.c" PROPERTIES COMPILE_FLAGS "-Wno-unused-variable")

    include_directories(${CMAKE_CURRENT_BINARY_DIR}/runtimes/tvm/)
  else()
    message(FATAL_ERROR "Invalid TVM model selected")
  endif()
endmacro()

macro(generateTFLiteOpsResolver runtime_src)
  if(NOT "${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    add_custom_command(
      OUTPUT
        runtimes/tflite/generated/model.tflite
        runtimes/tflite/generated/ops_resolver.h
      COMMAND
        python3 ${CMAKE_SOURCE_DIR}/../scripts/build_tflite.py
          --model-path ${CONFIG_KENNING_MODEL_PATH}
          --ops-list "${CONFIG_KENNING_TFLITE_OPS}"
          --output-resolver-path runtimes/tflite/generated/ops_resolver.h
          --output-model-path runtimes/tflite/generated/model.tflite
    )
  elseif(NOT "${CONFIG_KENNING_TFLITE_OPS}" STREQUAL "")
    add_custom_command(
      OUTPUT
        runtimes/tflite/generated/model.tflite
        runtimes/tflite/generated/ops_resolver.h
      COMMAND
        python3 ${CMAKE_SOURCE_DIR}/../scripts/build_tflite.py
          --ops-list "${CONFIG_KENNING_TFLITE_OPS}"
          --output-resolver-path runtimes/tflite/generated/ops_resolver.h
          --output-model-path runtimes/tflite/generated/model.tflite
    )
  else()
    add_custom_command(
      OUTPUT
        runtimes/tflite/generated/model.tflite
        runtimes/tflite/generated/ops_resolver.h
      COMMAND
        python3 ${CMAKE_SOURCE_DIR}/../scripts/build_tflite.py
          --ops-list "Conv2D,FullyConnected,MaxPool2D,Reshape,Softmax"
          --output-resolver-path runtimes/tflite/generated/ops_resolver.h
          --output-model-path runtimes/tflite/generated/model.tflite
    )
  endif()
  list(APPEND ${runtime_src} "runtimes/tflite/generated/ops_resolver.h")
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/runtimes/tflite/)
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
list(APPEND core_src "core/loaders.c")
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
  generateTFLiteOpsResolver(runtime_src)
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

  if(CONFIG_TENSORFLOW_LITE_MICRO)
    set(tflite_runtime_src "")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/python/tflite_micro/python_ops_resolver.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/micro/kernels/fft_flexbuffers_generated_data.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/micro/kernels/rfft.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/micro/kernels/window.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/micro/kernels/window_flexbuffers_generated_data.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/rfft_float.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/rfft_int16.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/rfft_int32.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/window.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/kiss_fft_wrappers/kiss_fft_float.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/kiss_fft_wrappers/kiss_fft_int16.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/kiss_fft_wrappers/kiss_fft_int32.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/compiler/mlir/lite/core/api/error_reporter.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/compiler/mlir/lite/schema/schema_utils.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/array.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/core/c/common.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/debug_log.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/hexdump.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/fake_micro_context.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/memory_helpers.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_allocation_info.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/test_helpers.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/test_helper_custom_ops.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/recording_micro_allocator.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_time.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_profiler.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_utils.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/flatbuffer_utils.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/mock_micro_graph.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_interpreter.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_interpreter_context.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_interpreter_graph.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_allocator.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_context.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_log.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_op_resolver.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/micro_resource_variable.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/system_setup.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/tflite_bridge/micro_error_reporter.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/internal/common.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/internal/quantization_util.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/internal/portable_tensor_utils.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/internal/tensor_ctypes.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/internal/tensor_utils.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/internal/reference/comparisons.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/kernels/kernel_util.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/core/api/flatbuffer_conversions.cc")
    list(APPEND tflite_runtime_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/core/api/tensor_utils.cc")

    file(GLOB tflite_kernels_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/tensorflow/lite/micro/kernels/*.cc")
    list(FILTER tflite_kernels_src EXCLUDE REGEX ".*test.*")
    list(APPEND tflite_runtime_src ${tflite_kernels_src})

    file(GLOB signal_kernels_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/micro/kernels/*.cc")
    list(FILTER signal_kernels_src EXCLUDE REGEX ".*test.*")
    list(APPEND tflite_runtime_src ${signal_kernels_src})

    file(GLOB signal_src "${ZEPHYR_TFLITE_MICRO_MODULE_DIR}/signal/src/*.cc")
    list(FILTER signal_src EXCLUDE REGEX ".*test.*")
    list(APPEND tflite_runtime_src ${signal_src})


    list(APPEND tflite_runtime_src "runtimes/tflite/tflite.cpp")
    list(APPEND tflite_runtime_src "runtimes/tflite/tflite.c")

    generateTFLiteOpsResolver(tflite_runtime_src)

    add_compile_definitions(EXTENSION_TVM)

    add_llext_target(
      "llext-tflite"
      OUTPUT "${CMAKE_BINARY_DIR}/llext/tflite.llext"
      SOURCES "${tflite_runtime_src}"
    )

    llext_compile_options("llext-tflite" -ffunction-sections)
    llext_link_options("llext-tflite" -Wl,--gc-sections -Wl,-e tflite_opt_entrypoint -Wl,${CMAKE_CURRENT_SOURCE_DIR}/runtimes/llext/keep_ext.ld)

    llext_compile_options("llext-tflite" -O3 -Os)
    llext_link_options("llext-tflite" -O3 -Os)
    add_dependencies(app llext-tflite)
  endif(CONFIG_TENSORFLOW_LITE_MICRO)
endif(CONFIG_LLEXT)
