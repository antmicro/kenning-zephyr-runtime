# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(demo_app LANGUAGES C)

if(${CONFIG_KENNING_ML_RUNTIME_TVM})
  set(model_graph_path ${CMAKE_CURRENT_SOURCE_DIR}/model/tvm/model_graph.json)
  set(model_params_path ${CMAKE_CURRENT_SOURCE_DIR}/model/tvm/model_params.bin)

  add_custom_command(
    OUTPUT
      src/model_data.h
    COMMAND
      stat --printf="%s" ${model_graph_path} |
        xargs printf "0: %08x" |
        xxd -r -g0 1>model_data
    COMMAND
      stat --printf="%s" ${model_params_path} |
        xargs printf "0: %08x" |
        xxd -r -g0 1>>model_data
    COMMAND
      objcopy
        -I binary
        -O binary
        --reverse-bytes=4
        model_data
        model_data
    COMMAND
      cat
        ${model_graph_path} ${model_params_path}
        1>>model_data
    COMMAND
      xxd
        -i
        model_data
        src/model_data.h
    COMMAND
      rm
        model_data
  )
  add_custom_target(
    generate_model_data_h
    DEPENDS
    src/model_data.h
  )
  add_dependencies(app generate_model_data_h)

  unset(model_graph_path)
  unset(model_params_path)
elseif(${CONFIG_KENNING_ML_RUNTIME_TFLITE})

  set(model_path ${CMAKE_CURRENT_SOURCE_DIR}/model/tflite/model.tflite)

  add_custom_command(
    OUTPUT
      src/model_data.h
    COMMAND
      cp
        ${model_path}
        model_data
    COMMAND
      xxd
        -i
        model_data
        src/model_data.h
    COMMAND
      rm
        model_data
  )
  add_custom_target(
    generate_model_data_h
    DEPENDS
    src/model_data.h
  )
  add_dependencies(app generate_model_data_h)
endif()

add_custom_command(
  OUTPUT
    ${BOARD}.repl
  COMMAND
    gcc
      -H -E -P -x assembler-with-cpp
      -I ${ZEPHYR_BASE}/include/
      -I ${ZEPHYR_BASE}/dts/
      -I ${ZEPHYR_BASE}/dts/arm/
      -I ${ZEPHYR_BASE}/dts/riscv/
      -I ${ZEPHYR_BASE}/dts/common/
      -I ${WEST_TOPDIR}/modules/hal/stm32/dts/
      ${dts_files}
      1>${BOARD}_flat.dts
      2>includes.txt
  COMMAND
    dts2repl
      --output ${BOARD}.repl
      ${BOARD}_flat.dts
)

add_custom_target(
  board-repl
  DEPENDS
    ${BOARD}.repl
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/src")
target_sources(app PRIVATE src/main.c)
