# Zephyr runtime for Kenning

Copyright (c) 2023-2024 [Antmicro](https://www.antmicro.com)

### Building the runtime

Before building the binary install required packages
```bash
apt-get update
apt-get install -y \
    ccache \
    device-tree-compiler \
    dfu-util \
    file \
    g++-multilib \
    gcc \
    gcc-multilib \
    git \
    gperf \
    make \
    ninja-build \
    python3-dev \
    python3-pip \
    python3-venv \
    software-properties-common \
    wget \
    xz-utils
```

You also need to download the Zephyr SDK with the following commands (official instructions can be found [here](https://docs.zephyrproject.org/latest/develop/getting_started/index.html))
```bash
cd /tmp
wget "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.3/zephyr-sdk-0.16.3_linux-x86_64.tar.xz"
tar xvf ./zephyr-sdk-0.16.3_linux-x86_64.tar.xz
mv ./zephyr-sdk-0.16.3 $HOME/.local/opt/zephyr-sdk
rm zephyr-sdk-0.16.3_linux-x86_64.tar.xz
```

As everything is installed, you can initialize the Zephyr environment.
First, create directory for zephyr workspace and clone this repository there
```bash
mkdir zephyr-workspace
cd zephyr-workspace
git clone https://github.com/antmicro/kenning-zephyr-runtime
cd kenning-zephyr-runtime
```

Create python virtual environment and install required packages
```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install \
    cmake \
    git+https://github.com/antmicro/renode-run \
    west \
```

And then initialize Zephyr environment
```bash
west init -l .
west update
west zephyr-export
python3 -m pip install -r ../zephyr/scripts/requirements.txt
```

Now you are ready to build the runtime binary
```bash
west build -p always -b nrf52840dk_nrf52840 app
```

### Running the runtime in Renode

Before starting renode simulation make sure that the `./renode/log` directory exists"
```bash
mkdir -p ./renode/log
````

To run Renode simulation use `scripts/run_renode.sh` script as follows:
```bash
./scripts/run_renode.sh ./renode/config/nrf52840dk_nrf52840.resc
```
