# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

# Generates TVM model ops source and adds it to provided list.
#
# @param runtime_src List with runtime sources to which model sources will be
#                    added.
macro(kenning_gen_tvm_model_sources runtime_src)
  if("${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND}" OR "${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND_INT8}")
    if("${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND}")
      set(model_src_path "runtimes/tvm/generated/magic_wand")
    elseif("${CONFIG_KENNING_TVM_MODEL_MAGIC_WAND_INT8}")
      set(model_src_path "runtimes/tvm/generated/magic_wand_int8")
    endif()

    add_custom_command(
      OUTPUT
        runtimes/tvm/generated/model_impl.c
        runtimes/tvm/generated/model_impl.h
      DEPENDS
        "${model_src_path}.c"
        "${model_src_path}.h"
      COMMAND
        cp "${CMAKE_CURRENT_SOURCE_DIR}/${model_src_path}.c" runtimes/tvm/generated/model_impl.c
      COMMAND
        cp "${CMAKE_CURRENT_SOURCE_DIR}/${model_src_path}.h" runtimes/tvm/generated/model_impl.h
    )

    if(NOT "${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
      set(model_graph_path "${CONFIG_KENNING_MODEL_PATH}_graph.json")
      set(model_params_path "${CONFIG_KENNING_MODEL_PATH}_params.bin")
      set(model_graph_data_path runtimes/tvm/generated/model_impl.graph_data)

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
          "${model_graph_data_path}"
          "${model_graph_data_path}.json"
        COMMAND
          stat --printf="%s" model_graph_min.json |
            xargs printf "0: %08x" |
            xxd -r -g0 1>${model_graph_data_path}
        COMMAND
          stat --printf="%s" ${model_params_path} |
            xargs printf "0: %08x" |
            xxd -r -g0 1>>${model_graph_data_path}
        COMMAND
          objcopy
            -I binary
            -O binary
            --reverse-bytes=4
            ${model_graph_data_path}
            ${model_graph_data_path}
        COMMAND
          cat
            model_graph_min.json ${model_params_path}
            1>>${model_graph_data_path}
        COMMAND
          cp "${CONFIG_KENNING_MODEL_PATH}.json" ${model_graph_data_path}.json
      )
      unset(model_graph_path)
      unset(model_params_path)
    endif(NOT "${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")

    unset(model_src_path)
  elseif("${CONFIG_KENNING_TVM_MODEL_PRE_GEN}")
    get_filename_component(model_path_dir "${CONFIG_KENNING_MODEL_PATH}" DIRECTORY)
    get_filename_component(model_path_name "${CONFIG_KENNING_MODEL_PATH}" NAME_WE)
    set(model_path "${model_path_dir}/${model_path_name}")

    add_custom_command(
      OUTPUT
        runtimes/tvm/generated/model_impl.c
        runtimes/tvm/generated/model_impl.h
        runtimes/tvm/generated/model_impl.graph_data
        runtimes/tvm/generated/model_impl.graph_data.json
      DEPENDS
        "${CONFIG_KENNING_MODEL_PATH}"
      COMMAND
        cp "${model_path}.c" runtimes/tvm/generated/model_impl.c
      COMMAND
        cp "${model_path}.h" runtimes/tvm/generated/model_impl.h
      COMMAND
        cp "${model_path}.graph_data" runtimes/tvm/generated/model_impl.graph_data
      COMMAND
        cp "${model_path}.graph_data.json" runtimes/tvm/generated/model_impl.graph_data.json
    )

    unset(model_path)
    unset(model_path_dir)
    unset(model_path_name)
  elseif("${CONFIG_KENNING_TVM_MODEL_GEN}")
    if("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
      message(FATAL_ERROR "Provide model path via CONFIG_KENNING_MODEL_PATH")
    endif("${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")

    add_custom_command(
      OUTPUT
        runtimes/tvm/generated/model_impl.c
        runtimes/tvm/generated/model_impl.h
        runtimes/tvm/generated/model_impl.graph_data
        runtimes/tvm/generated/model_impl.graph_data.json
      COMMAND
        python3 ${KENNING_LIB_DIR}/scripts/build_tvm.py
          --input-path ${CONFIG_KENNING_MODEL_PATH}
          --output-path runtimes/tvm/generated/model_impl.graph_data
          --header-template-path ${CMAKE_CURRENT_LIST_DIR}/runtimes/tvm/generated/model_impl.h.template
          --board ${BOARD}
          --conf '${CONFIG_KENNING_TVM_TARGET_ATTRS}'
          ${CONFIG_KENNING_MODEL_COMPILER_ARGS}
    )
  else()
    message(FATAL_ERROR "Invalid TVM model selected")
  endif()

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
endmacro(kenning_gen_tvm_model_sources)

# Generates TFLite ops resolver source and adds it to provided list.
#
# @param runtime_src List with runtime sources to which model sources will be
#                    added.
macro(kenning_gen_tflite_model_sources runtime_src)
  if(NOT "${CONFIG_KENNING_MODEL_PATH}" STREQUAL "")
    add_custom_command(
      OUTPUT
        runtimes/tflite/generated/model.tflite
        runtimes/tflite/generated/model.tflite.json
        runtimes/tflite/generated/ops_resolver.h
      COMMAND
        python3 ${KENNING_LIB_DIR}/scripts/build_tflite.py
          --model-path ${CONFIG_KENNING_MODEL_PATH}
          --ops-list "${CONFIG_KENNING_TFLITE_OPS}"
          --output-resolver-path runtimes/tflite/generated/ops_resolver.h
          --output-model-path runtimes/tflite/generated/model.tflite
    )
  elseif(NOT "${CONFIG_KENNING_TFLITE_OPS}" STREQUAL "")
    add_custom_command(
      OUTPUT
        runtimes/tflite/generated/model.tflite
        runtimes/tflite/generated/model.tflite.json
        runtimes/tflite/generated/ops_resolver.h
      COMMAND
        python3 ${KENNING_LIB_DIR}/scripts/build_tflite.py
          --ops-list "${CONFIG_KENNING_TFLITE_OPS}"
          --output-resolver-path runtimes/tflite/generated/ops_resolver.h
          --output-model-path runtimes/tflite/generated/model.tflite
    )
  else()
    add_custom_command(
      OUTPUT
        runtimes/tflite/generated/model.tflite
        runtimes/tflite/generated/model.tflite.json
        runtimes/tflite/generated/ops_resolver.h
      COMMAND
        python3 ${KENNING_LIB_DIR}/scripts/build_tflite.py
          --ops-list "Conv2D,FullyConnected,MaxPool2D,Reshape,Softmax"
          --output-resolver-path runtimes/tflite/generated/ops_resolver.h
          --output-model-path runtimes/tflite/generated/model.tflite
    )
  endif()
  list(APPEND ${runtime_src} "runtimes/tflite/generated/ops_resolver.h")
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/runtimes/tflite/)
endmacro(kenning_gen_tflite_model_sources)

# Adds IREE sources to provided list.
#
# @param runtime_src List with runtime sources to which model sources will be
#                    added.
macro(kenning_gen_iree_model_sources runtime_src)
  set(model_vmfb_path "runtimes/iree/generated/model.vmfb")
  set(import_path "runtimes/iree/generated/model.mlir")

  add_custom_command(
    OUTPUT
      ${import_path}
    COMMAND
      iree-import-tflite -o ${import_path} ${CONFIG_KENNING_MODEL_PATH}
  )

  add_custom_command(
    OUTPUT
      ${model_vmfb_path}.json
    COMMAND
      cp ${CONFIG_KENNING_MODEL_PATH}.json ${model_vmfb_path}.json
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
        ${model_vmfb_path}
      DEPENDS
        ${import_path}
      COMMAND
        iree-compile
          --iree-input-type=tosa
          --iree-hal-target-backends vmvx
          -o ${model_vmfb_path}
          ${import_path}
    )
  elseif("${CONFIG_KENNING_IREE_LOADER_EMBEDDED_ELF}")
    add_custom_command(
      OUTPUT
        ${model_vmfb_path}
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
          -o ${model_vmfb_path}
          ${import_path}
    )
  elseif()
    message(FATAL_ERROR "Invalid IREE loader")
  endif()

  unset(model_vmfb_path)
  unset(import_path)
endmacro(kenning_gen_iree_model_sources)

# Generate model data header that contains quantization params, model struct
# and model data.
macro(kenning_gen_model_data)
  set(model_data_path)
  set(model_json_path)
  if(${CONFIG_KENNING_ML_RUNTIME_STUB})
    # nop
  elseif(${CONFIG_KENNING_ML_RUNTIME_TVM})
    set(model_data_path runtimes/tvm/generated/model_impl.graph_data)
    set(model_json_path runtimes/tvm/generated/model_impl.graph_data.json)
  elseif(${CONFIG_KENNING_ML_RUNTIME_TFLITE})
    set(model_data_path runtimes/tflite/generated/model.tflite)
    set(model_json_path runtimes/tflite/generated/model.tflite.json)
  elseif(${CONFIG_KENNING_ML_RUNTIME_IREE})
    set(model_data_path runtimes/iree/generated/model.vmfb)
    set(model_json_path runtimes/iree/generated/model.vmfb.json)
  endif()

  add_custom_command(
    OUTPUT
      model_data_weights.h
    DEPENDS
      ${model_data_path}
    COMMAND
      cp ${model_data_path} model_data
    COMMAND
      xxd
        -i
        model_data
        model_data_weights.h
    COMMAND
      rm model_data
    COMMAND
      sed -i -e "'s/unsigned/const unsigned/g'"
        model_data_weights.h
    COMMAND
      sed -i -e "'s/const unsigned char/const unsigned char __attribute((aligned(32)))/g'"
        model_data_weights.h
  )

  add_custom_command(
    OUTPUT
      model_data_struct.h
    DEPENDS
      ${model_json_path}
    COMMAND
      python3 ${KENNING_LIB_DIR}/scripts/io_spec_to_struct.py
        --input-path ${model_json_path}
        --output-path model_data_struct.h
  )

  add_custom_command(
    OUTPUT
      model_data.h
    DEPENDS
      model_data_weights.h
      model_data_struct.h
    COMMAND
      cat model_data_struct.h model_data_weights.h
        >model_data.h
  )

  add_custom_target(
    lib_generate_model_data_h
    DEPENDS
      model_data.h
  )

  unset(model_data_path)
  unset(model_json_path)
endmacro(kenning_gen_model_data)
