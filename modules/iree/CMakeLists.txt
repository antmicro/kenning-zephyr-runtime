# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ZEPHYR_IREE_MODULE)
  set(IREE_DIR ${ZEPHYR_CURRENT_MODULE_DIR})
  set(IREE_SRC_DIR "${IREE_DIR}/runtime/src/")
  set(FLATCC_DIR "${IREE_DIR}/third_party/flatcc/")
  set(FLATCC_SRC_DIR "${FLATCC_DIR}/src/")

  # build flatcc
  add_subdirectory(flatcc)

  zephyr_library()

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
  add_compile_definitions(IREE_VM_EXECUTION_TRACING_ENABLE=0)
  add_compile_definitions(IREE_HAL_VERBOSE_TRACING_ENABLE=0)
  set(CMAKE_SYSTEM_NAME Generic)
  set(IREE_BUILD_BINDINGS_TFLITE OFF)
  set(IREE_BUILD_COMPILER OFF)
  set(IREE_BUILD_SAMPLES OFF)
  set(IREE_BUILD_TESTS OFF)
  set(IREE_ENABLE_THREADING OFF)
  set(IREE_ENABLE_MLIR OFF)
  set(IREE_HAL_DRIVER_DEFAULTS OFF CACHE BOOL "" FORCE)
  set(IREE_HAL_DRIVER_LOCAL_SYNC ON CACHE BOOL "" FORCE)
  set(IREE_HAL_EXECUTABLE_LOADER_DEFAULTS OFF CACHE BOOL "" FORCE)
  set(IREE_HAL_EXECUTABLE_LOADER_EMBEDDED_ELF ON CACHE BOOL "" FORCE)
  set(IREE_HAL_EXECUTABLE_LOADER_VMVX_MODULE ON CACHE BOOL "" FORCE)

  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-unused-function -Wno-double-promotion -Wno-implicit-function-declaration")
  set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-unused-function -Wno-double-promotion -Wno-implicit-function-declaration")

  build_flatcc(
    NAME
      "dummy"
    SRC_DIR
      "iree/base/internal/flatcc/"
    IREE_ROOT_DIR
      "${IREE_DIR}"
    FLATCC_ARGS
      "--reader"
      "--verifier"
  )

  build_flatcc(
    NAME
      "bytecode_module_def"
    SRC_DIR
      "iree/schemas/"
    IREE_ROOT_DIR
      "${IREE_DIR}"
    FLATCC_ARGS
      "--reader"
      "--builder"
      "--verifier"
      "--json"
  )

  zephyr_include_directories(
    ${IREE_SRC_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    "${IREE_DIR}/third_party/flatcc/include"
  )
  zephyr_library_add_dependencies(
    "flatcc_dummy"
    "flatcc_bytecode_module_def"
  )

  if("${CONFIG_64BIT}")
    set(IREE_UK_POINTER_SIZE 8)
  else("${CONFIG_64BIT}")
    set(IREE_UK_POINTER_SIZE 4)
  endif("${CONFIG_64BIT}")

  # disabled for now, because causes issues on some boards
  # if("${CONFIG_ARM64}")
  #   set(IREE_UK_ARCH_ARM_64 1)
  # endif("${CONFIG_ARM64}")

  set(iree_ukernel_config_headers)
  list(APPEND iree_ukernel_config_headers "iree/builtins/ukernel/arch/config.h")
  list(APPEND iree_ukernel_config_headers "iree/builtins/ukernel/arch/arm_64/config.h")

  foreach(config_header ${iree_ukernel_config_headers})
    configure_file(
      "${IREE_SRC_DIR}/${config_header}.in"
      "${config_header}"
    )
  endforeach()

  set(iree_src "")

  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/allocator.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/bitfield.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/internal/arena.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/internal/atomic_slist.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/internal/cpu.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/internal/flags.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/internal/fpu_state.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/internal/synchronization.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/loop.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/loop_inline.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/status.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/string_builder.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/string_view.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/wait_source.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/base/time.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/allocator.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/allocator_heap.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/buffer.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/buffer_heap.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/buffer_view.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/buffer_view_util.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/command_buffer.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/command_buffer_validation.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/channel.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/device.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/drivers/local_sync/sync_device.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/drivers/local_sync/sync_event.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/drivers/local_sync/sync_semaphore.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/event.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/executable.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/executable_cache.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/fence.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/elf/elf_module.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/elf/platform/generic.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/executable_environment.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/executable_loader.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/inline_command_buffer.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/local_executable.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/local_executable_cache.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/local_pipeline_layout.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/loaders/embedded_elf_loader.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/loaders/vmvx_module_loader.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/pipeline_layout.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/semaphore.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/string_util.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/utils/buffer_transfer.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/utils/deferred_command_buffer.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/utils/resource_set.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/utils/semaphore_base.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/modules/hal/module.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/modules/hal/types.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/modules/hal/utils/buffer_diagnostics.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/modules/vmvx/module.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/buffer.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/bytecode_disasm.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/bytecode_dispatch.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/bytecode_module.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/context.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/instance.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/invocation.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/list.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/module.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/native_module.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/ref.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/shims.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/vm/stack.c")
  list(APPEND iree_src "${FLATCC_SRC_DIR}/runtime/verifier.c")

  # ukernel sources
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/elementwise_generic.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/pack_tile.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/unpack_tile.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/mmt4d_tile.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/pack.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/unpack.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/mmt4d.c")
  list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/query_tile_sizes.c")
  if("${IREE_UK_ARCH_ARM_64}")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/arch/arm_64/pack_arm_64.c")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/arch/arm_64/unpack_arm_64.c")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/arch/arm_64/mmt4d_arm_64.c")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/arch/arm_64/mmt4d_arm_64.S")
    if("${IREE_UK_BUILD_ARM_64_DOTPROD}")
      list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/arch/arm_64/mmt4d_arm_64_dotprod.S")
    endif("${IREE_UK_BUILD_ARM_64_DOTPROD}")
    if("${IREE_UK_BUILD_ARM_64_I8MM}")
      list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/arch/arm_64/mmt4d_arm_64_i8mm.S")
    endif("${IREE_UK_BUILD_ARM_64_I8MM}")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/builtins/ukernel/arch/arm_64/query_tile_sizes_arm_64.c")
  endif("${IREE_UK_ARCH_ARM_64}")

  if("${CONFIG_ARM64}")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/elf/arch/arm_64.c")
  elseif("${CONFIG_ARM}")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/elf/arch/arm_32.c")
  elseif("${CONFIG_RISCV}")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/elf/arch/riscv.c")
  elseif("${CONFIG_X86}" AND "${CONFIG_64BIT}")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/elf/arch/x86_64.c")
  elseif("${CONFIG_X86}")
    list(APPEND iree_src "${IREE_SRC_DIR}/iree/hal/local/elf/arch/x86_32.c")
  else()
    message(FATAL_ERROR "unsupported architecture")
  endif()

  zephyr_library_sources(${iree_src})

endif(CONFIG_ZEPHYR_IREE_MODULE)
