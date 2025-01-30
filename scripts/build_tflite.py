# Copyright (c) 2024-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Script for generating TFLite model and TFLite Micro ops resolver header
for a given model and/or ops list.
"""

import sys
import argparse
from pathlib import Path

from kenning.utils.resource_manager import ResourceURI
from kenning.optimizers.tflite import TFLiteCompiler


def main():
    parser = argparse.ArgumentParser(__doc__)

    parser.add_argument(
        "--model-path",
        type=ResourceURI,
        help="Path to the input model",
    )
    parser.add_argument(
        "--ops-list",
        type=str,
        nargs="?",
        default="",
        help="Comma-separated list of ops to include in the ops resolver",
    )
    parser.add_argument(
        "--output-resolver-path",
        type=Path,
        help="Path to output source file with TFLite ops resolver",
    )
    parser.add_argument(
        "--output-model-path",
        type=Path,
        help="Path to output TFLite model after compilation, if model was provided",  # noqa: E501
    )

    args = parser.parse_args()

    if args.model_path:
        if not args.output_model_path:
            print("If --model-path is provided, --output-model-path is a mandatory argument.")  # noqa: E501
            return 1
        if args.model_path.suffix != ".tflite":
            compiler = TFLiteCompiler(
                dataset=None,
                compiled_model_path=args.output_model_path,
                model_framework="any",
                resolver_output_path=args.output_resolver_path if args.output_resolver_path else None,  # noqa: E501
            )
            compiler.compile(args.model_path)
            return 0
        if args.output_resolver_path:
            with args.model_path.open("rb") as modelfd:
                modeldata = modelfd.read()
                TFLiteCompiler.create_resolver_file_from_model(
                    modeldata=modeldata,
                    additional_ops=args.ops_list.split(',') if args.ops_list else None,  # noqa: E501
                    output_path=args.output_resolver_path,
                )
        args.output_model_path.write_bytes(args.model_path.read_bytes())
        return 0
    if args.ops_list:
        TFLiteCompiler.create_resolver_file_from_ops_list(
            opcode_names=args.ops_list.split(','),
            output_path=args.output_resolver_path,
        )
        return 0

    print("User needs to provide either --input-path or --ops-list, or both.")  # noqa: E501
    return 1


if __name__ == "__main__":
    sys.exit(main())
