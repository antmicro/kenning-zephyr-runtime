#!/bin/bash

# Copyright (c) 2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e

west build -p always -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=llext.conf

west build app -t llext-tvm -- -DEXTRA_CONF_FILE="llext.conf;llext_tvm.conf"
west build app -t llext-tflite -- -DEXTRA_CONF_FILE="llext.conf;llext_tflite.conf"

west build -t board-repl

rm -f /tmp/uart /tmp/uart-log

python3 ./scripts/run_renode.py &
RUN_RENODE_PID=$!
trap "kill -s SIGTERM ${RUN_RENODE_PID}" EXIT KILL INT

until [ -e /tmp/uart ]; do
    echo "Waiting for /tmp/uart..."
    sleep 1
done

echo "Found /tmp/uart"

mkdir -p measurements

kenning optimize test --cfg tests/scenarios/test-tvm-llext.yml  --measurements measurements/results-tvm.json
kenning optimize test --cfg tests/scenarios/test-tflite-llext.yml  --measurements measurements/results-tflite.json
