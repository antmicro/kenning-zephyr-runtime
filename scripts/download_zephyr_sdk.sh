#!/usr/bin/env bash

# Copyright (c) 2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

sdk_version=$1
if [[ -z $sdk_version ]]; then
    echo "No version specified"
    exit 1
fi

pushd ~

sdk_checksum=
case "$sdk_version" in
    0.17.4)
        sdk_checksum=a7258bf50c892c4669712b2aacdfcd5e303245bcf36cb458a296039a664594f5
        ;;
    1.0.1)
        sdk_checksum=ca9bc0ff66fafca1dac9d592a36d953cf16d096a9d09b1c0357f021cf9f6a7eb
        ;;
    *)
        sdk_checksum=
        ;;
esac

if [[ -z $sdk_checksum ]]; then
    echo "No checksum supplied for Zephyr SDK $sdk_version"
    exit 1
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
