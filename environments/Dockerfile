FROM ubuntu:22.04

ENV INST 'apt-get install -y --no-install-recommends'
ENV PIPINST 'python3 -m pip install --no-cache-dir --upgrade'

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && $INST \
    ccache \
    curl \
    device-tree-compiler \
    dfu-util \
    file \
    fonts-lato \
    g++-multilib \
    gcc \
    gcc-multilib \
    git \
    jq \
    libmagic1 \
    libsdl2-dev \
    make \
    ninja-build \
    patch \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-tk \
    python3-wheel \
    python3-venv \
    mono-complete \
    wget \
    xxd \
    xz-utils \
    && apt-get autoremove && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN $PIPINST \
    pip \
    setuptools

RUN $PIPINST \
    "kenning[iree,tvm,tensorflow,tflite,reports,renode,uart] @ git+https://github.com/antmicro/kenning.git"

RUN mkdir -p /opt/renode && \
    wget -O /opt/renode/renode-latest.pkg.tar.gz https://builds.renode.io/renode-latest.pkg.tar.xz

ENV PYRENODE_PKG /opt/renode/renode-latest.pkg.tar.gz
