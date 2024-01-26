#!/usr/bin/env bash

set -e
set -x

ZEPHYR_SDK_PATH=$HOME/.local/opt/zephyr-sdk
PROJECT_ROOT=$(realpath $(pwd))

# setup SDK
if [ ! -d "$ZEPHYR_SDK_PATH" ]; then
  # determine latest SDK version
  ZEPHYR_SDK_VERSION=$(curl "https://api.github.com/repos/zephyrproject-rtos/sdk-ng/tags" | jq -r '[.[].name | select(.|test("-")|not)][0]')
  ZEPHYR_SDK_VERSION="${ZEPHYR_SDK_VERSION:1}"

  ZEPHYR_SDK_ARCHIVE="zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.xz"

  mkdir -p $(dirname $ZEPHYR_SDK_PATH)
  cd /tmp
  wget "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/${ZEPHYR_SDK_ARCHIVE}"
  tar xvf ${ZEPHYR_SDK_ARCHIVE}
  mv ./zephyr-sdk-${ZEPHYR_SDK_VERSION} ${ZEPHYR_SDK_PATH}
  rm ${ZEPHYR_SDK_ARCHIVE}
  chmod -R o+w $ZEPHYR_SDK_PATH
  cd ${ZEPHYR_SDK_PATH}
  ./setup.sh -t x86_64-zephyr-elf
  ./setup.sh -t arm-zephyr-eabi
  ./setup.sh -h
  ./setup.sh -c
else
  echo "Zephyr SDK already downloaded"
fi

cd ${PROJECT_ROOT}
# setup Zephyr project
if [ ! -d ".west" ]; then
  west init -l .
  west update
  west zephyr-export
  python3 -m pip install -r ../zephyr/scripts/requirements.txt
else
  echo "Zephyr project already downloaded"
fi
