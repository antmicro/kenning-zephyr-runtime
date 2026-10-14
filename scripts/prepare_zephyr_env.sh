#!/usr/bin/env bash

# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -x
set -e

PIP_EXEC="python3 -m pip"
VENV_EXEC="python3 -m venv"

SET_ZEPHYR_VERSION=./scripts/set_zephyr_version.sh

if command -v uv >/dev/null 2>&1; then
    PIP_EXEC="uv pip"
    VENV_EXEC="uv venv"
fi

# prepare venv for the project
if [ ! -d ".venv" ]; then
    $VENV_EXEC .venv
fi

# source venv
source .venv/bin/activate

$PIP_EXEC install pip setuptools --upgrade
$PIP_EXEC install west
$PIP_EXEC install yq

# Update Zephyr version in west.yml
if [[ -n "$ZEPHYR_VERSION" ]]; then
    $SET_ZEPHYR_VERSION west_revision $ZEPHYR_VERSION
fi

# setup west workspace
python -m west init -l . || true
python -m west update --fetch-opt=--quiet
python -m west zephyr-export

if [[ -n "$ZEPHYR_VERSION" ]]; then
    $SET_ZEPHYR_VERSION zephelin_patches $ZEPHYR_VERSION
fi

# install dependencies
$PIP_EXEC install -r requirements.txt -r ../zephyr/scripts/requirements-base.txt -r ../zephyr/scripts/requirements-build-test.txt -r ../zephyr/scripts/requirements-run-test.txt

python -m west zephyr-export

# setup SDK

# dir consistent with default west sdk install behavior
pushd ~

sdk_version=0.17.4
sdk_checksum=a7258bf50c892c4669712b2aacdfcd5e303245bcf36cb458a296039a664594f5

if [[ "$ZEPHYR_VERSION" != "v4.3.0" ]]; then
    sdk_version=1.0.1
    sdk_checksum=ca9bc0ff66fafca1dac9d592a36d953cf16d096a9d09b1c0357f021cf9f6a7eb
fi

sdk_dir_name="zephyr-sdk-${sdk_version}"
sdk_archive_name="zephyr-sdk-${sdk_version}_linux-x86_64_minimal.tar.xz"


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
