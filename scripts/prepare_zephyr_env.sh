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

ZEPHYR_SDK_PATH=${ZEPHYR_SDK_PATH:-$HOME/.local/opt/zephyr-sdk}
PROJECT_ROOT=$(realpath $(pwd))

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

# check if west and other project dependencies are installed
if [ -z "$(python3 -m pip freeze -r requirements.txt 2>&1 | grep "not installed")" ]; then
    echo "Project dependencies installed"
else
    echo "Installing missing dependencies"
    $PIP_EXEC install pip setuptools --upgrade
    $PIP_EXEC install -r requirements.txt
fi

# setup SDK
if [ ! -d "$ZEPHYR_SDK_PATH" ]; then
    cd /tmp

    # determine latest SDK version
    while true
    do
        ZEPHYR_SDK_VERSIONS=$(curl "https://api.github.com/repos/zephyrproject-rtos/sdk-ng/tags" | jq -r "[.[].name | select(.|test(\"-\")|not)][:3][]" | sed "/v0.16.6/d")
        if [ -n "$ZEPHYR_SDK_VERSIONS" ]
        then
            break
        fi
        echo "Couldn't determine the lastest SDK version. Retrying in 60 seconds..."
        sleep 60s
    done

    for ZEPHYR_SDK_VERSION in $ZEPHYR_SDK_VERSIONS
    do
        ZEPHYR_SDK_VERSION="${ZEPHYR_SDK_VERSION:1}"
        ZEPHYR_SDK_ARCHIVE="zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz"

        WGET_RETVAL=0
        wget "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/${ZEPHYR_SDK_ARCHIVE}" || WGET_RETVAL=$?
        if [ "$WGET_RETVAL" -eq 0 ]; then
            break
        fi
        echo "Fallback to older SDK version"
    done
    mkdir -p $(dirname $ZEPHYR_SDK_PATH)
    tar xvf ${ZEPHYR_SDK_ARCHIVE}
    mv ./zephyr-sdk-${ZEPHYR_SDK_VERSION} ${ZEPHYR_SDK_PATH}
    rm ${ZEPHYR_SDK_ARCHIVE}
    chmod -R o+w $ZEPHYR_SDK_PATH
else
    echo "Zephyr SDK already downloaded"
fi

cd ${ZEPHYR_SDK_PATH}
./setup.sh -t x86_64-zephyr-elf
./setup.sh -t arm-zephyr-eabi
./setup.sh -t riscv64-zephyr-elf
./setup.sh -h
./setup.sh -c

cd -

if [ $ZEPHYR_SDK_ONLY -eq 1 ]; then
    echo "Zephyr SDK installed successfully"
    exit 0
fi

cd ${PROJECT_ROOT}

# setup Zephyr project
if [ ! -d "../.west" ]; then
    python3 -m west init -l .
    python3 -m west update
    python3 -m west zephyr-export
else
    echo "Zephyr project already downloaded"
    python3 -m west update
fi


# install Zephyr's Python dependencies
if [ ! -f ".venv/zephyr-deps.stamp" ]; then
    $PIP_EXEC install -r ../zephyr/scripts/requirements-base.txt
    $PIP_EXEC install -r ../zephyr/scripts/requirements-build-test.txt
    $PIP_EXEC install -r ../zephyr/scripts/requirements-run-test.txt
    touch .venv/zephyr-deps.stamp
fi

echo "The environment is configured"
