# Copyright (c) 2023-2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Python script for generating TVM implementation for a given model.
"""

import argparse
import shutil
from pathlib import Path

from kenning.optimizers.tvm import TVMCompiler
from kenning.utils.resource_manager import ResourceURI
from kenning.utils.logger import KLogger

def prepare_tvm_model():
    parser = argparse.ArgumentParser(__doc__)

    parser.add_argument(
        "--input-path",
        "-i",
        type=ResourceURI,
        help="Path to input model",
        required=True,
    )
    parser.add_argument(
        "--output-path", "-o", type=Path, help="Path to compiled model", required=True
    )
    parser.add_argument(
        "--header-template-path",
        "-t",
        type=ResourceURI,
        help="Path to header template",
        required=True,
    )
    parser.add_argument("--board", "-b", type=str, help="Target board", required=True)
    parser.add_argument("--conf", "-c", type=str, help="Target config", default="")
    parser.add_argument(
        "--verbosity", "-v", type=str, help="Logging verbosity", default="INFO"
    )

    args = parser.parse_args()
    KLogger.set_verbosity(level=args.verbosity)

    if not args.input_path.exists():
        raise FileNotFoundError(f"{args.input_path} does not exist")

    c_input_path = args.input_path.with_suffix(".c")
    h_input_path = args.input_path.with_suffix(".h")
    json_input_path = args.input_path.with_suffix(args.input_path.suffix + ".json")
    if c_input_path.exists() and h_input_path.exists() and json_input_path.exists():
        KLogger.info(f"TVM ops already exist, copying to {args.output_path}")
        args.output_path.parent.mkdir(exist_ok=True)
        shutil.copy2(args.input_path, args.output_path)
        shutil.copy2(json_input_path, args.output_path.with_suffix(args.output_path.suffix + ".json"))
        shutil.copy2(c_input_path, args.output_path.with_suffix(".c"))
        shutil.copy2(h_input_path, args.output_path.with_suffix(".h"))
        return

    compiler = TVMCompiler(
        dataset=None,
        compiled_model_path=args.output_path,
        model_framework="any",
        target="zephyr",
        target_attrs=args.conf,
        target_microtvm_board=args.board,
        zephyr_header_template=args.header_template_path,
        conv2d_data_layout="NCHW",
        module_name=None, # Only empty string as module name is supported by Kenning Zephyr Runtime
    )

    compiler.init()
    compiler.compile(args.input_path)

    KLogger.info(f"TVM ops saved to {args.output_path}")

if __name__ == "__main__":
    prepare_tvm_model()
