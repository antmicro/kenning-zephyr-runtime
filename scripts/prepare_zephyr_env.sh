#!/usr/bin/env bash

# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -x
set -e

PIP_EXEC="python3 -m pip"
VENV_EXEC="python3 -m venv"

if command -v uv >/dev/null 2>&1; then
    PIP_EXEC="uv pip"
    VENV_EXEC="uv venv"
fi

# prepare venv for the project
if [ ! -d ".venv" ]; then
    if [[ ! -z "$CI" ]] || [[ -f /.dockerenv ]]; then
        # include global packages when run in CI or docker container
        $VENV_EXEC .venv --system-site-packages
    else
        $VENV_EXEC .venv
    fi
fi

# source venv
source .venv/bin/activate

$PIP_EXEC install pip setuptools --upgrade
$PIP_EXEC install west

# setup west workspace
python -m west init -l .
python -m west update
python -m west zephyr-export


# install dependencies
$PIP_EXEC install -r requirements.txt -r ../zephyr/scripts/requirements-base.txt -r ../zephyr/scripts/requirements-build-test.txt -r ../zephyr/scripts/requirements-run-test.txt

# setup SDK

# dir consistent with default west sdk install behavior
pushd ~

sdk_version=0.17.4
sdk_dir_name="zephyr-sdk-${sdk_version}"
sdk_archive_name="zephyr-sdk-${sdk_version}_linux-x86_64_minimal.tar.xz"
sdk_checksum=a7258bf50c892c4669712b2aacdfcd5e303245bcf36cb458a296039a664594f5

if [ -d ${sdk_dir_name} ]; then
    echo "SDK directory already exists, skipping download"
else
    wget "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${sdk_version}/${sdk_archive_name}"
    if echo "${sdk_checksum}  ${sdk_archive_name}" | sha256sum --check --status; then
        echo "Valid checksum"
    else
        echo "Invalid sdk checksum"
        rm "${sdk_archive_name}"
        exit 1
    fi

    tar xf "${sdk_archive_name}"
    rm "${sdk_archive_name}"
fi

./${sdk_dir_name}/setup.sh -t x86_64-zephyr-elf -t arm-zephyr-eabi -t riscv64-zephyr-elf -t aarch64-zephyr-elf -h

popd

echo "The environment is configured"
