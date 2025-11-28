#!/usr/bin/env bash

# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e
set -x

declare -a modules=("tvm" "dlpack" "iree" "executorch")

for module in "${modules[@]}"
do
    mkdir -p "../zephyr/modules/${module}"
    mkdir -p "../${module}/zephyr"
    cp "./modules/${module}/module.yml" "../${module}/zephyr"
    cp -r ./modules/${module}/* "../zephyr/modules/${module}/"
done

cp "./modules/tvm/crt_config.h" "../tvm/include/"
cp "./modules/iree/zephyr_config.h" "../iree/zephyr/"
