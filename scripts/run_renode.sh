#!/usr/bin/env bash

# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

renode-run -- --console --disable-xwt -e \
    "\$bin=@build/zephyr/zephyr.elf;\
     i @$1;\
     start;"
