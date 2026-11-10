FROM ubuntu:22.04

ENV INST 'apt-get install -y --no-install-recommends'
ENV PIPINST 'uv pip install --no-cache --upgrade'

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && $INST \
    ca-certificates \
    ccache \
    cmake \
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
    libncurses5 \
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
    ninja-build \
    patch \
    picocom \
    wget \
    xxd \
    xz-utils \
    && apt-get autoremove && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN wget https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb && dpkg -i packages-microsoft-prod.deb && rm packages-microsoft-prod.deb

RUN apt-get update && $INST dotnet-sdk-9.0 && apt-get autoremove && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN wget -O /tmp/renode-portable-dotnet.tar.gz https://builds.renode.io/renode-latest.linux-portable-dotnet.tar.gz && \
    mkdir -p /opt/renode/ && \
    tar -C /opt/renode --strip-components=1 -xzvf /tmp/renode-portable-dotnet.tar.gz && rm /tmp/renode-portable-dotnet.tar.gz

ENV PYRENODE_RUNTIME=coreclr
ENV PYRENODE_BIN=/opt/renode/renode

# Use Python 3.11 from a venv instead of the system Python 3.10
RUN curl -LsSf https://astral.sh/uv/install.sh | UV_INSTALL_DIR=/usr/local/bin sh && \
    uv venv /opt/venv --seed --python 3.11
ENV VIRTUAL_ENV="/opt/venv"
ENV PATH="/opt/venv/bin:$PATH"

RUN $PIPINST \
    pip \
    setuptools \
    west \
    pyelftools

RUN $PIPINST \
    "kenning[anomaly_detection,iree,tvm,tensorflow,litert,torch,reports,renode,uart,auto_pytorch] @ git+https://github.com/antmicro/kenning.git"
