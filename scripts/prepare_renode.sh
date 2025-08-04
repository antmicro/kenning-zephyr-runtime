#!/bin/sh

# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

if [ -d renode_latest ] ; then
  echo "Renode already downloaded."
else
  echo "Downloading Renode..."
  curl https://builds.renode.io/renode-latest.linux-portable-dotnet.tar.gz -o renode-latest.tar.gz
  tar -xf renode-latest.tar.gz
  rm -f renode-latest.tar.gz
  mv -f renode_*-dotnet_portable renode_latest
fi

echo "Preparing environment..."
export PYRENODE_BIN=$(realpath renode_latest/renode)
export PYRENODE_RUNTIME=coreclr
export PATH=$PATH:$(realpath renode_latest/)
