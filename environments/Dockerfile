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
    gcc \
    gcc-multilib \
    git \
    g++-multilib \
    jq \
    libdbus-1-3 \
    libfontconfig \
    libgl1 \
    libgssapi-krb5-2 \
    libhidapi-hidraw0 \
    libhidapi-libusb0 \
    libmagic1 \
    libsdl2-dev \
    libusb-0.1-4 \
    libxcb1 \
    libxcb-glx0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render0 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-shm0 \
    libxcb-sync1 \
    libxcb-util1 \
    libxcb-xfixes0 \
    libxcb-xinerama0 \
    libxcb-xkb1 \
    libxkbcommon0 \
    libxkbcommon-x11-0 \
    make \
    mono-complete \
    ninja-build \
    patch \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-tk \
    python3-venv \
    python3-wheel \
    wget \
    xxd \
    xz-utils \
    && apt-get autoremove && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN $PIPINST \
    pip \
    setuptools

RUN $PIPINST \
    "kenning[iree,tvm,torch,anomaly_detection,tensorflow,tflite,reports,renode,uart] @ git+https://github.com/antmicro/kenning.git"

RUN mkdir -p /opt/renode && \
    wget -O /opt/renode/renode-latest.pkg.tar.gz https://builds.renode.io/renode-latest.pkg.tar.xz

ENV PYRENODE_PKG /opt/renode/renode-latest.pkg.tar.gz
