# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Python script for convering IO spec in JSON format to model struct.
"""

import argparse
import json
import numpy as np
import re
from typing import Dict, List, Any, Tuple
from math import prod
from pathlib import Path

from kenning.utils.resource_manager import ResourceURI
from kenning.interfaces.io_spec_serializer import IOSpecSerializer

STRUCT_TEMPLATE = """
/* encode type string as uint32_t */
#define ENCODE_TYPE(t0, t1, t2, t3) ((t0) | ((t1) << 8) | ((t2) << 16) | ((t3) << 24))

#define QUANTIZATION_INPUT_SCALE {quantization_input_scale}
#define QUANTIZATION_INPUT_ZERO_POINT {quantization_input_zero_point}
#define QUANTIZATION_OUTPUT_SCALE {quantization_output_scale}
#define QUANTIZATION_OUTPUT_ZERO_POINT {quantization_output_zero_point}

const model_spec_t model_spec = {{
    .num_input = {num_input},
    .num_input_dim = {num_input_dim},
    .input_shape = {input_shape},
    .input_data_type = {input_data_type},
    .num_output = {num_output},
    .num_output_dim = {num_output_dim},
    .output_shape = {output_shape},
    .output_data_type = {output_data_type},
    .entry_func = "{entry_func}",
    .model_name = "{model_name}",
}};

"""


def py_arr_to_c_arr(arr: list):
    """
    Converts python array to C array.

    Parameters
    ----------
    arr : List
        Array to be converted.

    Returns
    -------
    str :
        Array in C format.
    """
    return str(arr).replace("[", "{").replace("]", "}")

def py_tuple_to_c_struct(tpl: Tuple):
    return str(tpl).replace("(", "{").replace(")", "}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)

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


    model_spec = STRUCT_TEMPLATE.format(
        quantization_input_scale=io_spec_input[0].get("scale", 0),
        quantization_input_zero_point=io_spec_input[0].get("zero_point", 0),
        quantization_output_scale=io_spec_output[0].get("scale", 0),
        quantization_output_zero_point=io_spec_output[0].get("zero_point", 0),
        num_input=len(io_spec_input),
        num_input_dim=py_arr_to_c_arr([len(inp["shape"]) for inp in io_spec_input]),
        input_shape=py_arr_to_c_arr([inp["shape"] for inp in io_spec_input]),
        input_data_type=py_tuple_to_c_struct(py_arr_to_c_arr(IOSpecSerializer.io_spec_parse_types(io_spec_input))),
        num_output=len(io_spec_output),
        num_output_dim=py_arr_to_c_arr([len(out["shape"]) for out in io_spec_output]),
        output_shape=py_arr_to_c_arr([out["shape"] for out in io_spec_output]),
        output_data_type=py_tuple_to_c_struct(py_arr_to_c_arr(IOSpecSerializer.io_spec_parse_types(io_spec_output))),
        entry_func="module.main",
        model_name="module",
    )

    args.output_path.write_text(model_spec)
