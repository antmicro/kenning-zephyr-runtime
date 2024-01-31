# Zephyr runtime for Kenning

Copyright (c) 2023-2024 [Antmicro](https://www.antmicro.com)

This project provides Zephyr library for [Kenning](https://github.com/antmicro/kenning) runtime API, along with application for model evaluation.

## Overview

This repository provides:

* `kenning_inference_lib` - a Zephyr library providing generic wrapper methods for loading models and running inference, regardless of underlying implementation.
  The aim is to simplify adoption and switching between existing runtime implementations.
* `kenning-zephyr-runtime` app - a Zephyr application for evaluating models and runtimes on devices with [Kenning](https://github.com/antmicro/kenning).

## Building the project

This section contains instructions for preparing Zephyr and building the runtime.

### Preparing Zephyr development environment

To be able to build and use the project, several dependencies need to be installed:

* [Zephyr dependencies](https://docs.zephyrproject.org/latest/develop/getting_started/index.html#install-dependencies)
* `jq`
* `curl`
* `west`
* `CMake`

On Debian-based Linux distributions, the above-listed dependencies can be installed as follows:

```bash
sudo apt update

sudo apt install -y --no-install-recommends git ninja-build \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel python3-venv xz-utils file \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 jq curl
```

### Preparing the project

First of, create a workspace directory and clone the repository in it:

```bash skip
mkdir zephyr-workspace && cd zephyr-workspace
git clone https://github.com/antmicro/kenning-zephyr-runtime.git
cd kenning-zephyr-runtime
```

After entering the project's directory, create a Python's virtual environment, and install `CMake` and `west`:

```bash
python3 -m venv venv
source venv/bin/activate
pip install west cmake
```

Then, initialize Zephyr workspace and ensure that Zephyr SDK is installed:

```bash
./scripts/prepare_zephyr_env.sh
```

And prepare additional modules:

```bash
./scripts/prepare_modules.sh
```

### Building the Kenning runtime tester application

To build Kenning Zephyr runtime, select a supported machine learning runtime and a board.

```bash skip
west build --board <board> app -- -DEXTRA_CONF_FILE=<runtime>.conf
```

The available runtimes that can be provided in `<runtime>` are:

* [tvm](https://tvm.apache.org/docs/topic/microtvm/index.html)
* [tflite](https://github.com/tensorflow/tflite-micro)

The boards on which the project was tested:

* [stm32f746g_disco](https://renodepedia.renode.io/boards/stm32f746g_disco/)
* [nrf52840dongle_nrf52840](https://renodepedia.renode.io/boards/nrf52840dongle_nrf52840)
* [nrf52840dk_nrf52840](https://renodepedia.renode.io/boards/nrf52840dk_nrf52840)

Check [Adding support for more boards section](#adding-support-for-more-boards) for information on what is needed to add support for your target device.

The built binary after `west build` can be found in `build/zephyr/zephyr.elf`.

### Installing Kenning with Renode

Use `pip` to install [Kenning](https://github.com/antmicro/kenning) with Renode support enabled:

```bash
pip install --upgrade pip
pip install "kenning[tvm,tensorflow,reports,renode] @ git+https://github.com/antmicro/kenning.git"
```

## Evaluating the model in Kenning

[Kenning](https://github.com/antmicro/kenning) provides:

* Optimization and compilation of the model
* Evaluation of the model on target device:
    * Sending the model to the device using UART communication (e.g. execution graph or TFLite Flatbuffer)
    * Sending input data for the model to run inference on
    * Collecting output data from the model, and evaluating the quality and performance of the model on target device with selected runtime
* Report rendering, including comparison reports that allow to compare various runtimes, boards, models and applied optimizations

With Kenning, we can also evaluate the runtime by simulating the device in Renode.
This allows us to:

* Check how the model would behave without having the actual hardware
* Check the performance and correctness of the model and runtime running on device in Continuous Integration pipelines without the actual device in the loop
* Check the performance of runtimes and models on currently developed platforms
* Obtain more detailed metrics regarding device usage, e.g. histogram of instructions

The switch between Renode and actual hardware is seamless - both communicate with Kenning using UART communication.

### Building the project and evaluating models in Renode

![TFLite Micro scenario with Renode simulation](img/renode-scenario-example.png)

This section will demonstrate how to build the project and evaluate model for recognizing gestures on `stm32f746g_disco`.

#### Using TFLite Micro runtime

First of, build the `kenning-zephyr-runtime` app for `stm32f746g_disco` and TFLite Micro configuration:

```bash
west build -p always -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=tflite.conf
```

Then, evaluate the model in Renode using a sample scenario located in `kenning-scenarios/renode-zephyr-tflite-magic-wand-inference.json` and generate a report with performance and quality metrics:

```bash
kenning optimize test report \
    --json-cfg kenning-scenarios/renode-zephyr-tflite-magic-wand-inference.json \
    --measurements results.json --verbosity INFO \
    --report-path reports/stm32-renode-tflite-magic-wand/report.md \
    --to-html \
    --verbosity INFO
```

The report on model's performance in Markdown format should be available under `reports/stm32-renode-tflite-magic-wand/report.md`.
The HTML report built on that should be accessible from `reports/stm32-renode-tflite-magic-wand/report/report.html`.

#### Using microTVM

To build the `kenning-zephyr-runtime` app to work with microTVM runtime, just set `-DEXTRA_CONF_FILE` to `tvm.conf`, e.g.:

```bash
west build -p always -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=tvm.conf
```

Evaluate the model using a sample scenario located in `kenning-scenarios/renode-zephyr-tvm-magic-wand-inference.json`:

```bash
kenning optimize test report \
    --json-cfg kenning-scenarios/renode-zephyr-tvm-magic-wand-inference.json \
    --measurements results.json --verbosity INFO \
    --report-path reports/stm32-renode-tvm-magic-wand/report.md \
    --to-html \
    --verbosity INFO
```

## Evaluating the model in Kenning using actual hardware

Kenning can evaluate the runtime running on a physical device.
To do so, we need to flash the device and replace the `RenodeRuntime` in evaluation scenarios for Kenning with proper runtimes.

### Running evaluation on NRF52840 dongle

![TFLite Micro scenario running on hardware](img/device-scenario-example.png)

Build the runtime for `nrf52840dongle_nrf52840` (let's use TFLite Micro in this example):

```bash skip
west build -p always -b nrf52840dongle_nrf52840 app -- -DEXTRA_CONF_FILE=tflite.conf
```

Flash Kenning runtime on the device by following [instructions](https://docs.zephyrproject.org/latest/boards/arm/nrf52840dongle_nrf52840/doc/index.html#option-1-using-the-built-in-bootloader-only) in Zephyr docs.

In the end, evaluate the model and generate a report with performance and quality metrics:

```bash skip
kenning optimize test report \
    --json-cfg kenning-scenarios/zephyr-tflite-magic-wand-inference.json \
    --measurements results.json --verbosity INFO \
    --report-types performance classification \
    --report-path reports/nrf-tflite-magic-wand/report.md \
    --to-html \
    --verbosity INFO
```

### Running evaluation on STM32F746

Build the runtime for `stm32f746g_disco` (let's use TFLite Micro in this example):

```bash skip
west build -p always -b stm32f746g_disco app -- -DEXTRA_CONF_FILE=tvm.conf
```

Flash the connected device with the `kenning-zephyr-runtime` app:

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

## Adding support for more boards

Adapting `kenning-zephyr-runtime` for new boards is pretty straightforward.
In general, as long as underlying runtime implementation supports a given board without additional configuration, the process of adapting the application for new board boils down to picking an UART for communicating with the Kenning application running on host.
Such UART is expected to be aliased `kcomms` in the application.

The alias can be set in the overlay file under `app/boards/<board_name>.overlay`, where `<board_name>` is the name of the board in Zephyr, passed in `--board` flag in `west build`:

```dts
/ {
    aliases {
        kcomms = &uart0;
    };
};
```

It is crucial that selected UART isn't used anywhere else (e.g. as `zephyr,console`).
