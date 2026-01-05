#!/usr/bin/env bash

# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e
set -x

west patch clean
west patch -sm zephelin clean

declare -a modules=("iree" "executorch")

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
west patch -sm zephelin apply
