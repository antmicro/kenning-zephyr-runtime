#!/bin/bash

# Copyright (c) 2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

set -e

# Training a decision tree with scikit-learn framework, on minispot dataset
kenning train --cfg ./kenning-scenarios/minispot/emlearn-decision-tree/renode-stm32f746g.yml

# Optimizing the tree to emlearn
kenning optimize --cfg ./kenning-scenarios/minispot/emlearn-decision-tree/renode-stm32f746g.yml

# Building the evaluation app with the model (required by emlearn)
west build -p -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=emlearn.conf -DCONFIG_KENNING_MODEL_PATH=\"$(realpath ./output/minispot.c)\"
west build -t board-repl

# Running evaluation and report generation
kenning test report --cfg ./kenning-scenarios/minispot/emlearn-decision-tree/renode-stm32f746g.yml --measurements results.json --report-path ./reports/stm32_minispot_emlearn/report.md --to-html
