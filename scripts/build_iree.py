# Copyright (c) 2025-2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Script for compiling a tflite model with IREE compiler.
"""

import sys
import argparse
import json
from typing import List
from pathlib import Path
import os
from kenning.utils.resource_manager import ResourceURI
import onnx
from kenning.converters.tflite_converter import TFLiteConverter

def main():
    parser = argparse.ArgumentParser(__doc__)

    parser.add_argument(
        "--input-model-path",
        type=Path,
        help="Path to the input model",
        required=True,
    )
    parser.add_argument(
        "--output-model-path",
        type=Path,
        help="Path to output model after compilation, in selecred iree format.",
        required=True,
    )
    parser.add_argument(
        "--iree-backend",
        type=str,
        help="Desired IREE backend - vmvx (defaut) or elf.",
        required=False,
        default="vmvx"
    )
    parser.add_argument(
        "--target-cpu",
        type=str,
        help="Target CPU for the elf backend.",
    )
    parser.add_argument(
        "--target-triple",
        type=str,
        help="Target triple for the elf backend.",
    )
    parser.add_argument(
        "--target-cpu-features",
        type=str,
        help="Target cpu features for the elf backend.",
        nargs='?',
        const='',
    )

    args = parser.parse_args()

    model = None

    with open(args.input_model_path.with_suffix(args.input_model_path.suffix + '.json')) as original_iospec:
        io_spec = json.load(original_iospec)
        try:
            output_names = [spec["name"] for spec in io_spec["output"]]
        except KeyError:
            output_names = None
        model = TFLiteConverter(args.input_model_path).to_onnx()

        # Conversion from tflite to onnx changes name of the entry function.
        io_spec["entry_func"] = "module." + model.graph.name

        with open(args.output_model_path.with_suffix(args.output_model_path.suffix + '.json'), "w") as new_iospec:
            json.dump(io_spec, new_iospec)

    onnx_path = args.output_model_path.with_suffix(".tmp.onnx")
    mlir_path = args.output_model_path.with_suffix(".tmp.mlir")

    onnx.save(model, onnx_path)

    os.system(f"iree-import-onnx {onnx_path.resolve()} --opset-version 17 -o {mlir_path.resolve()}")

    if args.iree_backend == "vmvx":
        os.system(f"iree-compile {mlir_path.resolve()} --iree-hal-target-backends=vmvx -o {args.output_model_path.resolve()}")
    elif args.iree_backend == "elf":
        if not args.target_cpu:
            print("User needs to provide target cpu for the elf backend: --target-cpu")
            return 1
        if not args.target_triple:
            print("User needs to provide target triple for the elf backend: --target-triple")
            return 1
        if not args.target_cpu_features:
                args.target_cpu_features = ""

        os.system(f"""iree-compile {mlir_path.resolve()} \
            --iree-hal-target-backends=llvm-cpu \
            --iree-vm-bytecode-module-strip-source-map=true \
            --iree-opt-level=O3  \
            --iree-llvmcpu-link-embedded=true \
            --iree-vm-emit-polyglot-zip=true   \
            --iree-llvmcpu-debug-symbols=false  \
            --iree-llvmcpu-target-triple="{args.target_triple}" \
            --iree-llvmcpu-target-cpu="{args.target_cpu}"   \
            --iree-llvmcpu-target-cpu-features="{args.target_cpu_features}" \
            -o {args.output_model_path.resolve()}""")
    else:
        print(f"Iree backend: {args.iree_backend} not recognized. Backends supported: vmvx, elf.")
        return 1

    return 0

if __name__ == "__main__":
    sys.exit(main())
