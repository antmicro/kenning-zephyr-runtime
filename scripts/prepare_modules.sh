#!/usr/bin/env bash

set -e
set -x

declare -a modules=("tvm" "dlpack")

for module in "${modules[@]}"
do
    mkdir -p "../zephyr/modules/${module}"
    mkdir -p "../${module}/zephyr"
    cp "./modules/${module}/module.yml" "../${module}/zephyr"
    cp "./modules/${module}/Kconfig" "../zephyr/modules/${module}/"
    cp "./modules/${module}/CMakeLists.txt" "../zephyr/modules/${module}/"
done

cp "./modules/tvm/crt_config.h" "../tvm/include/"
