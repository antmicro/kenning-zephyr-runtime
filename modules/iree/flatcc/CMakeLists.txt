# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

# - iree-flatcc module
# This module compiles iree-flatcc-cli tool and defines a function for
# generating flatcc headers with it

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS ${HOST_C_FLAGS})
set(CMAKE_EXECUTABLE_SUFFIX)

add_executable(iree-flatcc-cli
  "${FLATCC_DIR}/src/cli/flatcc_cli.c"
  "${FLATCC_DIR}/external/hash/cmetrohash64.c"
  "${FLATCC_DIR}/external/hash/str_set.c"
  "${FLATCC_DIR}/external/hash/ptr_set.c"
  "${FLATCC_DIR}/src/compiler/hash_tables/symbol_table.c"
  "${FLATCC_DIR}/src/compiler/hash_tables/scope_table.c"
  "${FLATCC_DIR}/src/compiler/hash_tables/name_table.c"
  "${FLATCC_DIR}/src/compiler/hash_tables/schema_table.c"
  "${FLATCC_DIR}/src/compiler/hash_tables/value_set.c"
  "${FLATCC_DIR}/src/compiler/fileio.c"
  "${FLATCC_DIR}/src/compiler/parser.c"
  "${FLATCC_DIR}/src/compiler/semantics.c"
  "${FLATCC_DIR}/src/compiler/coerce.c"
  "${FLATCC_DIR}/src/compiler/codegen_schema.c"
  "${FLATCC_DIR}/src/compiler/flatcc.c"
  "${FLATCC_DIR}/src/compiler/codegen_c.c"
  "${FLATCC_DIR}/src/compiler/codegen_c_reader.c"
  "${FLATCC_DIR}/src/compiler/codegen_c_sort.c"
  "${FLATCC_DIR}/src/compiler/codegen_c_builder.c"
  "${FLATCC_DIR}/src/compiler/codegen_c_verifier.c"
  "${FLATCC_DIR}/src/compiler/codegen_c_sorter.c"
  "${FLATCC_DIR}/src/compiler/codegen_c_json_parser.c"
  "${FLATCC_DIR}/src/compiler/codegen_c_json_printer.c"
  "${FLATCC_DIR}/src/runtime/builder.c"
  "${FLATCC_DIR}/src/runtime/emitter.c"
  "${FLATCC_DIR}/src/runtime/refmap.c"
)
target_include_directories(iree-flatcc-cli SYSTEM
  PUBLIC
    "${FLATCC_DIR}/external"
    "${FLATCC_DIR}/include"
    "${FLATCC_DIR}/config"
)
install(
  TARGETS iree-flatcc-cli
  COMPONENT IREETools-Runtime
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION bin
)

# Generates flatcc headers using flatcc-cli tool
# Sample usage:
#   build_flatcc(
#     NAME
#       "dummy"
#     SRC_DIR
#       "iree/base/internal/flatcc/"
#     IREE_ROOT_DIR
#       "${IREE_DIR}"
#     FLATCC_ARGS
#       "--reader"
#       "--verifier"
#   )
function(build_flatcc)
  cmake_parse_arguments(
    _ARGS
    ""
    "NAME;SRC_DIR;IREE_ROOT_DIR"
    "FLATCC_ARGS"
    ${ARGN}
  )

  # create list with outputs
  set(_OUTS "")
  foreach(_ARG ${_ARGS_FLATCC_ARGS})
    if(_ARG STREQUAL "--reader")
      list(APPEND _OUTS "${_ARGS_SRC_DIR}/${_ARGS_NAME}_reader.h")
    elseif(_ARG STREQUAL "--builder")
      list(APPEND _OUTS "${_ARGS_SRC_DIR}/${_ARGS_NAME}_builder.h")
    elseif(_ARG STREQUAL "--verifier")
      list(APPEND _OUTS "${_ARGS_SRC_DIR}/${_ARGS_NAME}_verifier.h")
    elseif(_ARG STREQUAL "--json")
      list(APPEND _OUTS "${_ARGS_SRC_DIR}/${_ARGS_NAME}_json_printer.h")
      list(APPEND _OUTS "${_ARGS_SRC_DIR}/${_ARGS_NAME}_json_parser.h")
    endif()
  endforeach()

  # define command to be run
  add_custom_command(
    OUTPUT
      ${_OUTS}
    COMMAND
      "${CMAKE_CURRENT_BINARY_DIR}/flatcc/iree-flatcc-cli"
        -o "${_ARGS_IREE_ROOT_DIR}/runtime/src/${_ARGS_SRC_DIR}"
        -I "${_ARGS_IREE_ROOT_DIR}"
        ${_ARGS_FLATCC_ARGS}
        "${_ARGS_IREE_ROOT_DIR}/runtime/src/${_ARGS_SRC_DIR}/${_ARGS_NAME}.fbs"
    WORKING_DIRECTORY
      "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS
      iree-flatcc-cli
    VERBATIM
  )

  add_custom_target(
    "flatcc_${_ARGS_NAME}_gen"
    DEPENDS
      ${_OUTS}
  )

  # add target to lib
  add_library("flatcc_${_ARGS_NAME}" INTERFACE)
  add_dependencies("flatcc_${_ARGS_NAME}" "flatcc_${_ARGS_NAME}_gen")
  target_include_directories("flatcc_${_ARGS_NAME}"
    INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )
  target_link_libraries("flatcc_${_ARGS_NAME}"
    INTERFACE
  )
  target_compile_options("flatcc_${_ARGS_NAME}"
    INTERFACE
      "-I${_ARGS_IREE_ROOT_DIR}/third_party/flatcc/include/"
      "-I${_ARGS_IREE_ROOT_DIR}/third_party/flatcc/include/flatcc/reflection/"
  )
  install(
    TARGETS "flatcc_${_ARGS_NAME}"
  )
endfunction()
