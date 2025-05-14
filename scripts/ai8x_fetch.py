# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Script for fetching AI8X model and associated files.

`input_uri` should be a path to the `.bin` file.
"""

import argparse
from pathlib import Path

from kenning.utils.resource_manager import ResourceURI


def download_or_copy(inp: str, out: Path):
    """
    Copies ResourceURI-like resource to given path.
    """
    out.write_bytes(ResourceURI(inp).read_bytes())


def main():
    parser = argparse.ArgumentParser(__doc__)

    parser.add_argument(
        "input_uri",
        type=str,
        help="Path or URL to the file"
    )
    parser.add_argument(
        "out_dir",
        type=Path,
        help="Path where the output model should be stored"
    )

    args = parser.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)

    download_or_copy(args.input_uri, args.out_dir / "model.bin")
    download_or_copy(args.input_uri + ".json", args.out_dir / "model.bin.json")
    download_or_copy(args.input_uri.rsplit('.', 1)[0] + "/cnn_model.c", args.out_dir / "cnn_model.c")  # noqa: E501


if __name__ == "__main__":
    main()
