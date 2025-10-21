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
python -m west sdk install --toolchains x86_64-zephyr-elf arm-zephyr-eabi riscv64-zephyr-elf

echo "The environment is configured"
