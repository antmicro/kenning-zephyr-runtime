# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

import argparse
import json
import numpy as np
from math import prod
from pathlib import Path

from kenning.utils.resource_manager import ResourceURI

STRUCT_TEMPLATE = """
/* encode type string as uint32_t */
#define ENCODE_TYPE(t0, t1, t2, t3) ((t0) | ((t1) << 8) | ((t2) << 16) | ((t3) << 24))

#define QUANTIZATION_INPUT_SCALE {quantization_input_scale}
#define QUANTIZATION_INPUT_ZERO_POINT {quantization_input_zero_point}
#define QUANTIZATION_OUTPUT_SCALE {quantization_output_scale}
#define QUANTIZATION_OUTPUT_ZERO_POINT {quantization_output_zero_point}

const MlModel model_struct = {{
    .num_input = {num_input},
    .num_input_dim = {num_input_dim},
    .input_shape = {input_shape},
    .input_length = {input_length},
    .input_size_bytes = {input_size_bytes},
    .num_output = {num_output},
    .output_length = {output_length},
    .output_size_bytes = {output_size_bytes},
    .hal_element_type = {hal_element_type},
    .entry_func = "{entry_func}",
    .model_name = "{model_name}",
}};

"""


def py_arr_to_c_arr(arr: list):
    return str(arr).replace("[", "{").replace("]", "}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--input-path",
        "-i",
        type=ResourceURI,
        help="Path to IO spec",
        required=True,
    )
    parser.add_argument(
        "--output-path",
        "-o",
        type=Path,
        help="Path to header with struct",
        required=True,
    )

    args = parser.parse_args()

    input_path = Path(args.input_path)
    if not input_path.exists():
        raise FileNotFoundError(f"{input_path} IO spec not found")

    if not args.output_path.parent.exists():
        args.output_path.parent.mkdir(parents=True, exist_ok=True)

    io_spec = json.loads(input_path.read_text())

    io_spec_input = io_spec.get("processed_input", io_spec["input"])
    io_spec_output = io_spec["output"]

    input_elem_size_bytes = [np.dtype(inp["dtype"]).itemsize for inp in io_spec_input]
    output_elem_size_bytes = np.dtype(io_spec_output[0]["dtype"]).itemsize

    hal_element_type_map = {
        "float32": "ENCODE_TYPE('f', '3', '2', 0)",
        "int8": "ENCODE_TYPE('i', '8', 0, 0)",
    }

    model_struct = STRUCT_TEMPLATE.format(
        quantization_input_scale=io_spec_input[0].get("scale", 0),
        quantization_input_zero_point=io_spec_input[0].get("zero_point", 0),
        quantization_output_scale=io_spec_output[0].get("scale", 0),
        quantization_output_zero_point=io_spec_output[0].get("zero_point", 0),
        num_input=len(io_spec_input),
        num_input_dim=py_arr_to_c_arr([len(inp["shape"]) for inp in io_spec_input]),
        input_shape=py_arr_to_c_arr([inp["shape"] for inp in io_spec_input]),
        input_length=py_arr_to_c_arr([prod(inp["shape"]) for inp in io_spec_input]),
        input_size_bytes=py_arr_to_c_arr(input_elem_size_bytes),
        num_output=len(io_spec_output),
        output_length=py_arr_to_c_arr([prod(outp["shape"]) for outp in io_spec_output]),
        output_size_bytes=output_elem_size_bytes,
        hal_element_type=hal_element_type_map[io_spec_input[0]["dtype"]],
        entry_func="module.main",
        model_name="module",
    )

    args.output_path.write_text(model_struct)
