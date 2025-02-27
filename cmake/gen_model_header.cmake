# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

# Saves model data header to a specified path. It requires
# CONFIG_KENNING_MODEL_PATH to be specified.
#
# @param output_path Path to which model data header should be saved.
function(kenning_gen_model_header output_path)
  set(model_data_path "${KENNING_MODULE_BINARY_DIR}/lib/kenning_inference_lib/model_data.h")

  add_custom_command(
    OUTPUT
      ${output_path}
    DEPENDS
      ${model_data_path}
    COMMAND
      cp ${model_data_path} ${output_path}
  )

  add_custom_target(
    generate_model_data_h
    DEPENDS
      ${output_path}
  )

  add_dependencies(app generate_model_data_h)

  unset(model_data_path)
endfunction(kenning_gen_model_header)
