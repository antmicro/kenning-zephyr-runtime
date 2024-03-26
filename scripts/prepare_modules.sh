#!/usr/bin/env bash

# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e
set -x

declare -a modules=("tvm" "dlpack" "iree")

for module in "${modules[@]}"
do
    mkdir -p "../zephyr/modules/${module}"
    mkdir -p "../${module}/zephyr"
    cp "./modules/${module}/module.yml" "../${module}/zephyr"
    cp "./modules/${module}/Kconfig" "../zephyr/modules/${module}/"
    cp "./modules/${module}/CMakeLists.txt" "../zephyr/modules/${module}/"
done

cp "./modules/tvm/crt_config.h" "../tvm/include/"
cp "./modules/iree/zephyr_config.h" "../iree/zephyr/"
cp -r "./modules/iree/flatcc" "../zephyr/modules/iree/"
