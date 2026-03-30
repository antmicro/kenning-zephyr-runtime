#!/usr/bin/env bash

# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e
set -x

west patch clean
west patch -sm zephelin clean

if [ "$SKIP_ZEPHELIN" = "1" ]; then
    # only TVM patches will be applied
    west patch -sm zephelin -dm tvm apply
    declare -a modules=("tvm" "dlpack" "iree" "executorch")
else
    west patch -sm zephelin apply
    declare -a modules=("iree" "executorch")
fi

for module in "${modules[@]}"
do
    mkdir -p "../zephyr/modules/${module}"
    mkdir -p "../${module}/zephyr"
    cp "./modules/${module}/module.yml" "../${module}/zephyr"
    cp -r ./modules/${module}/* "../zephyr/modules/${module}/"
done

cp "./modules/iree/zephyr_config.h" "../iree/zephyr/"

# Temporary until we transfer all module changes to patches.
west patch apply
