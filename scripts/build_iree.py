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
import onnx
from kenning.optimizers.iree import IREECompiler

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

    compiler_args = []

    if args.iree_backend == "vmvx":
        backend_name = "vmvx"
    elif args.iree_backend == "elf":
        if not args.target_cpu:
            print("User needs to provide target cpu for the elf backend: --target-cpu")
            return 1
        if not args.target_triple:
            print("User needs to provide target triple for the elf backend: --target-triple")
            return 1
        if not args.target_cpu_features:
                args.target_cpu_features = ""

        backend_name = "llvm-cpu"

        compiler_args = [
            "iree-vm-bytecode-module-strip-source-map=true",
            "iree-opt-level=O3",
            "iree-llvmcpu-link-embedded=true",
            "iree-vm-emit-polyglot-zip=true",
            "iree-llvmcpu-debug-symbols=false",
            f"iree-llvmcpu-target-triple={args.target_triple}",
            f"iree-llvmcpu-target-cpu={args.target_cpu}",
            f"iree-llvmcpu-target-cpu-features={args.target_cpu_features}",
        ]
    else:
        print(f"Iree backend: {args.iree_backend} not recognized. Backends supported: vmvx, elf.")
        return 1

    compiler = IREECompiler(
        dataset=None,
        compiled_model_path=args.output_model_path,
        backend=backend_name,
        compiler_args=compiler_args,
    )

    with open(args.input_model_path.with_suffix(args.input_model_path.suffix + '.json')) as original_iospec:
        io_spec = json.load(original_iospec)

        compiler.compile(args.input_model_path, io_spec)

    return 0

if __name__ == "__main__":
    sys.exit(main())
