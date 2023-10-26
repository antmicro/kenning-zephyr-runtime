#!/usr/bin/env bash

renode-run -- --console --disable-xwt -e "i @$1; start;"
