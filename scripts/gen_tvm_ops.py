# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Python script for generating TVM ops for given model.
"""

import argparse
from pathlib import Path

from kenning.optimizers.tvm import TVMCompiler
from kenning.utils.resource_manager import ResourceURI
from kenning.utils.logger import KLogger

if __name__ == "__main__":
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

    compiler = TVMCompiler(
        dataset=None,
        compiled_model_path=args.output_path,
        model_framework="any",
        target="zephyr",
        target_attrs=args.conf,
        target_microtvm_board=args.board,
        zephyr_header_template=args.header_template_path,
        conv2d_data_layout="NCHW",
    )

    compiler.compile(args.input_path)

    KLogger.info(f"TVM ops saved to {args.output_path}")
