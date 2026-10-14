#!/usr/bin/env bash

# Copyright (c) 2023-2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -x
set -e

PIP_EXEC="python3 -m pip"
VENV_EXEC="python3 -m venv"

SET_ZEPHYR_VERSION=./scripts/set_zephyr_version.sh

ZEPHYR_VERSION="${ZEPHYR_VERSION:-v4.3.0}"

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
$SET_ZEPHYR_VERSION download_sdk $ZEPHYR_VERSION


echo "The environment is configured"
