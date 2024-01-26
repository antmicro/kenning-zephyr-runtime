#!/bin/bash

set -e -o pipefail

export DEBIAN_FRONTEND=noninteractive
export TZ=UTC

bash <(echo -e 'set -e\n' && tuttest --language bash README.md | grep -v '^\$')
