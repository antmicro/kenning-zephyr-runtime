#!/bin/bash

# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e -o pipefail

export DEBIAN_FRONTEND=noninteractive
export TZ=UTC

bash <(echo -e 'set -ex\n' && tuttest --language bash README.md | grep -v '^\$')
