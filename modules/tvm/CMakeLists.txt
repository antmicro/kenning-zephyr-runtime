# Copyright (c) 2023 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ZEPHYR_TVM_MODULE)
  set(TVM_DIR ${ZEPHYR_CURRENT_MODULE_DIR})

  zephyr_library()

  zephyr_include_directories(
    ${TVM_DIR}/include/
    ${TVM_DIR}/src/runtime/crt/include/
  )

  zephyr_library_sources(
    ${TVM_DIR}/src/runtime/crt/common/crt_backend_api.c
    ${TVM_DIR}/src/runtime/crt/common/crt_runtime_api.c
    ${TVM_DIR}/src/runtime/crt/common/func_registry.c
    ${TVM_DIR}/src/runtime/crt/common/ndarray.c
    ${TVM_DIR}/src/runtime/crt/common/packed_func.c
    ${TVM_DIR}/src/runtime/crt/graph_executor/graph_executor.c
    ${TVM_DIR}/src/runtime/crt/graph_executor/load_json.c
    ${TVM_DIR}/src/runtime/crt/memory/page_allocator.c
    ${TVM_DIR}/src/runtime/crt/memory/stack_allocator.c
  )
endif(CONFIG_ZEPHYR_TVM_MODULE)
