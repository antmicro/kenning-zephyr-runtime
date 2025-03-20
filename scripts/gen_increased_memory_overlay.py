#!/usr/bin/env python3

# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Python script for generating board overlay with increased memory.
"""

from pathlib import Path

import argparse
import re

OVERLAY_TEMPLATE = """
&{node} {{
    reg = <{reg_base} {reg_size}>;
}};
"""


def get_zephyr_dts_sram(build_path: Path) -> str:
    """
    Retrieves Zephyr sram from device tree.
    """
    with open(build_path / "zephyr/zephyr.dts", "r") as dts_file:
        board_dts = dts_file.read()

    match = re.findall(r"zephyr,sram = &([a-zA-Z0-9]*);", board_dts, re.MULTILINE)
    if len(match):
        return match[0]

    raise Exception("Zephyr sram not found")


def get_zephyr_dts_get_node_reg(build_path: Path, node: str) -> tuple[str, str]:
    """
    Retrieves node reg from device tree.
    """
    with open(build_path / "zephyr/zephyr.dts", "r") as dts_file:
        board_dts = dts_file.read()

    match = re.findall(rf"{node}:(?:.*\n)*?.*reg = <(.*)>;", board_dts)
    if len(match):
        return match[0].split()

    raise Exception(f"Node size {node} not found")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument(
        "--output-path",
        type=Path,
        help="Path where generated overlay will be saved",
        required=True,
    )
    parser.add_argument(
        "--build-path",
        type=Path,
        help="Path to the build directory",
        required=True,
    )
    parser.add_argument(
        "--memory-size-kb",
        type=int,
        help="Size of the increased memory expressed in kilobytes",
        required=True,
    )
    args = parser.parse_args()

    # get sram from board device tree
    sram = get_zephyr_dts_sram(args.build_path)

    # get sram reg data
    sram_reg = get_zephyr_dts_get_node_reg(args.build_path, sram)

    reg_size = f"{args.memory_size_kb * 1024:#x}"

    overlay = OVERLAY_TEMPLATE.format(
        node=sram, reg_base=sram_reg[0], reg_size=reg_size
    )

    args.output_path.write_text(overlay)

    print(f"Overlay file saved to {args.output_path}")
