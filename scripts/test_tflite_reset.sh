#!/bin/bash

set -e

west build -p always -b max32690evkit/max32690/m4 app -- \
    -DEXTRA_CONF_FILE=tflite.conf \
    -DCONFIG_KENNING_TFLITE_OPS=\"Add,FullyConnected,Mul,Reshape,Tanh,Sub,Div,StridedSlice,Exp,Cast,Less,Log,Sum,Sqrt,ReduceMax,Cos,Logistic\"

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

RESOURCES_URL="${KENNING_RESOURCES_URL:-https://dl.antmicro.com/kenning}"

for MODEL in vae.0 vae.1; do
    wget ${RESOURCES_URL}/models/tests/${MODEL}.tflite -O vae.tflite
    wget ${RESOURCES_URL}/models/tests/${MODEL}.tflite.json -O vae.tflite.json

    RESULT=./measurements/${MODEL}.json
    kenning test --measurements $RESULT --cfg ./tests/scenarios/tflite-test-vae.yml --verbosity DEBUG
done
