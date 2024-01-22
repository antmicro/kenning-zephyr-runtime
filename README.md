# Zephyr runtime for Kenning

Copyright (c) 2023-2024 [Antmicro](https://www.antmicro.com)

## Overview

Zephyr runtime for Kenning allows for testing multiple ML runtimes on various devices.

## Building the project

This section contains instructions for preparing Zephyr and building the runtime.

### Preparing Zephyr environment

Install required dependencies with `apt`:
```bash
apt update
apt install -y --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel python3-venv xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 jq curl
```

Create Python virtual environment and install required packages:
```bash
python3 -m venv venv
source venv/bin/activate
pip install west cmake
```

Initialize Zephyr workspace and ensure that Zephyr SDK is installed:
```bash
./scripts/prepare_zephyr_env.sh
```
Prepare additional modules:
```bash
./scripts/prepare_modules.sh
```

### Building the runtime

To build Kenning Zephyr runtime, select a supported machine learning runtime and a board.

```bash skip
west build --board <board> app -- -DEXTRA_CONF_FILE=<runtime>.conf
```


#### Supported runtimes
* [tvm](https://tvm.apache.org/docs/topic/microtvm/index.html)
* [tflite](https://github.com/tensorflow/tflite-micro)

#### Supported boards
* [stm32f746g_disco](https://renodepedia.renode.io/boards/stm32f746g_disco/)
* [nrf52840dongle_nrf52840](https://renodepedia.renode.io/boards/nrf52840dongle_nrf52840)
* [nrf52840dk_nrf52840](https://renodepedia.renode.io/boards/nrf52840dk_nrf52840)

## Evaluating the model in Kenning using Renode

Kenning can evaluate the runtime using Renode - it allows the user to:
* Check how the model would behave on actual hardware
* Measure the performance and quality of the model on simulated device
* See the instructions used during inference

### Installing Kenning with Renode
Use `pip` to install Kenning with Renode support enabled:
```bash
pip install --upgrade pip
pip install "kenning[tvm,tensorflow,reports,renode] @ git+https://github.com/antmicro/kenning.git"
```

### Using TFLite micro

Build Kenning runtime with TFLite for `stm32f746g_disco`.
```bash
west build -p always -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=tflite.conf
```


Evaluate the model using a sample scenario located in `kenning-scenarios/renode-zephyr-tflite-magic-wand-inference.json` and generate a report with performance, quality and emulation metrics:
```bash
kenning optimize test report \
    --json-cfg kenning-scenarios/renode-zephyr-tflite-magic-wand-inference.json \
    --measurements results.json --verbosity INFO \
    --report-types performance classification renode_stats \
    --report-path reports/stm32-renode-tflite-magic-wand/report.md \
    --to-html \
    --verbosity INFO
```
### Using microTVM

Build Kenning runtime with microTVM for `stm32f746g_disco`.
```bash
west build -p always -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=tvm.conf
```


Evaluate the model using a sample scenario located in `kenning-scenarios/renode-zephyr-tvm-magic-wand-inference.json` and generate a report with performance, quality and emulation metrics:
```bash
kenning optimize test report \
    --json-cfg kenning-scenarios/renode-zephyr-tvm-magic-wand-inference.json \
    --measurements results.json --verbosity INFO \
    --report-types performance classification renode_stats \
    --report-path reports/stm32-renode-tvm-magic-wand/report.md \
    --to-html \
    --verbosity INFO
```

## Evaluating the model in Kenning using actual hardware

Kenning can evaluate the runtime running on a physical device.

### Installing Kenning

Use `pip` to install Kenning:
```bash skip
pip install --upgrade pip
pip install "kenning[tvm,tensorflow,reports] @ git+https://github.com/antmicro/kenning.git"
```

### On STM32F746

Build the runtime with microTVM for `stm32f746g_disco`:
```bash skip
west build -p always -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=tvm.conf
```

Flash Kenning runtime on the device:
```bash skip
west flash
```

Evaluate the model and generate a report with performance and quality metrics:

```bash skip
kenning optimize test report \
    --json-cfg kenning-scenarios/zephyr-tvm-magic-wand-inference.json \
    --measurements results.json --verbosity INFO \
    --report-types performance classification \
    --report-path reports/stm32-tvm-magic-wand/report.md \
    --to-html \
    --verbosity INFO
```

### On NRF52840 dongle
Build the runtime with TFLite for `nrf52840dongle_nrf52840`:
```bash skip
west build -p always -b nrf52840dongle_nrf52840 app -- -DEXTRA_CONF_FILE=tflite.conf
```

Flash Kenning runtime on the device by following the [instructions](https://docs.zephyrproject.org/latest/boards/arm/nrf52840dongle_nrf52840/doc/index.html#option-1-using-the-built-in-bootloader-only) in Zephyr docs.


Evaluate the model and generate a report with performance and quality metrics:

```bash skip
kenning optimize test report \
    --json-cfg kenning-scenarios/zephyr-tflite-magic-wand-inference.json \
    --measurements results.json --verbosity INFO \
    --report-types performance classification \
    --report-path reports/nrf-tflite-magic-wand/report.md \
    --to-html \
    --verbosity INFO
```

## Adding support for more boards

Zephyr makes extending support to other boards straightforward.

### Configure UART in board overlay

Kenning runtime uses UART for host-device communications. UART is selected using the `kcomms` alias set in the `app/boards/<board_name>.overlay`:
```dts
/ {
    aliases {
        kcomms = &uart0;
    };
};
```
> **NOTE:** Ensure that selected UART isn't used anywhere else (e.g. `zephyr,console`).
