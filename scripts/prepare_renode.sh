#!/bin/sh

# Copyright (c) 2025-2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

RENODE_VERSION=${1:-1.16.1+20260524gitbcbf1b6ab}

if [ -d renode_latest ] ; then
  echo "Renode already downloaded."
else
  echo "Downloading Renode..."
  curl https://builds.renode.io/renode-${RENODE_VERSION}.linux-portable.tar.gz -o renode-pkg.tar.gz
  tar -xf renode-pkg.tar.gz
  rm -f renode-pkg.tar.gz
  mv -f renode_*-portable renode_portable
fi

echo "Preparing environment..."
export PYRENODE_BIN=$(realpath renode_portable/renode)
export PYRENODE_RUNTIME=coreclr
export PATH=$PATH:$(realpath renode_portable/)
