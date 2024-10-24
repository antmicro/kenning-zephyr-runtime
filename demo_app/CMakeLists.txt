# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(demo_app LANGUAGES C)

if(${CONFIG_KENNING_ML_RUNTIME_TVM})
  if("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    set(model_graph_path ${CMAKE_CURRENT_SOURCE_DIR}/model/tvm/magic_wand_graph.json)
    set(model_params_path ${CMAKE_CURRENT_SOURCE_DIR}/model/tvm/magic_wand_params.bin)

    add_custom_command(
      OUTPUT
        model_graph_min.json
      COMMAND
        cat ${model_graph_path} |
          tr -d \"\\n\\t \" 1>model_graph_min.json
    )

    add_custom_command(
      DEPENDS
        model_graph_min.json
      OUTPUT
        model_data
      COMMAND
        stat --printf="%s" model_graph_min.json |
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
          model_graph_min.json ${model_params_path}
          1>>model_data
    )
    unset(model_graph_path)
    unset(model_params_path)
  else("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    set(CONFIG_KENNING_TVM_MODEL "gen_model")
    add_custom_command(
      OUTPUT
        model_data
      COMMAND
        python3 ${CMAKE_SOURCE_DIR}/../scripts/gen_tvm_ops.py
          --input-path ${CONFIG_KENNING_MODEL_PATH}
          --output-path model_impl.graph_data
          --header-template-path ${CMAKE_SOURCE_DIR}/../lib/kenning_inference_lib/runtimes/tvm/generated/model_impl.h.template
          --board ${BOARD}
          ${CONFIG_KENNING_MODEL_COMPILER_ARGS}
      COMMAND
        cp model_impl.graph_data model_data
    )
  endif("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")

elseif(${CONFIG_KENNING_ML_RUNTIME_TFLITE})
  if("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    set(model_path ${CMAKE_CURRENT_SOURCE_DIR}/model/tflite/magic_wand.tflite)
  else("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    set(model_path ${CONFIG_KENNING_MODEL_PATH})
  endif("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")

  add_custom_command(
    OUTPUT
      model_data
    COMMAND
      cp
        ${model_path}
        model_data
  )
elseif(${CONFIG_KENNING_ML_RUNTIME_IREE})
  if("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    set(tflite_model_path ${CMAKE_CURRENT_SOURCE_DIR}/model/tflite/magic_wand_no_softmax.tflite)
  else("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    set(tflite_model_path ${CONFIG_KENNING_MODEL_PATH})
  endif("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")

  set(model_path model.vmfb)
  set(import_path model.mlir)

  add_custom_command(
    OUTPUT
      ${import_path}
    COMMAND
      iree-import-tflite -o ${import_path} ${tflite_model_path}
  )

  set(target_cpu_features "")
  if("${CONFIG_ARM64}")
    set(target_triple "arm64")
    set(target_cpu "${GCC_M_CPU}")
  elseif("${CONFIG_ARM}")
    set(target_triple "arm")
    set(target_cpu "${GCC_M_CPU}")
  elseif("${CONFIG_RISCV}" AND "${CONFIG_64BIT}")
    set(target_triple "riscv64")
    set(target_cpu "generic-rv64")
    set(target_cpu_features "+m,+f")
  elseif("${CONFIG_RISCV}")
    set(target_triple "riscv32")
    set(target_cpu "generic-rv32")
    set(target_cpu_features "+m,+f")
  elseif("${CONFIG_X86}" AND "${CONFIG_64BIT}")
    set(target_triple "x86_64")
    set(target_cpu "generic-x86")
  elseif("${CONFIG_X86}")
    set(target_triple "x86")
    set(target_cpu "generic-x86")
  else()
    message(FATAL_ERROR "unsupported architecture")
  endif()

  if("${CONFIG_KENNING_IREE_LOADER_VMVX}")
    add_custom_command(
      OUTPUT
        ${model_path}
      DEPENDS
        ${import_path}
      COMMAND
        iree-compile
          --iree-input-type=tosa
          --iree-hal-target-backends vmvx
          -o ${model_path}
          ${import_path}
    )
  elseif("${CONFIG_KENNING_IREE_LOADER_EMBEDDED_ELF}")
    add_custom_command(
      OUTPUT
        ${model_path}
      DEPENDS
        ${import_path}
      COMMAND
        iree-compile
          --iree-input-type=tosa
          --iree-hal-target-backends llvm-cpu
          --iree-vm-bytecode-module-strip-source-map=true
          --iree-vm-emit-polyglot-zip=false
          --iree-llvm-debug-symbols=false
          --iree-llvm-target-triple="${target_triple}"
          --iree-llvm-target-cpu="${target_cpu}"
          --iree-llvm-target-cpu-features="${target_cpu_features}"
          -o ${model_path}
          ${import_path}
    )
  elseif()
    message(FATAL_ERROR "Invalid IREE loader")
  endif()

  add_custom_command(
    OUTPUT
      model_data
    DEPENDS
      ${model_path}
    COMMAND
      cp
        ${model_path}
        model_data
  )
endif()

add_custom_command(
  OUTPUT
    src/model_data.h
  DEPENDS
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
