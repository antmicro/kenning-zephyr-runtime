#!/usr/bin/env bash

# Copyright (c) 2026 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

# A utility to set the Zephyr revision to build against.

usage()
{
    supported_zephyr_versions_array=(
        "v4.3.0"
        "v4.4.1"
        "v4.5.0"
        "v4.5.0-rc1"
        "main"
    )

    original_ifs="$IFS"
    IFS=", "
    supported_zephyr_versions="${supported_zephyr_versions_array[*]}"
    IFS="$original_ifs"

    echo "Usage: $0 [west_revision|zephelin_patches|download_sdk] ZEPHYR_REVISION" 1>&2
    echo "    ZEPHYR_REVISION can be one of: $supported_zephyr_versions" 1>&2
}

update_west_revision()
{
    # Update the revision for the given project in west.yml
    local project_name="$1"
    local new_revision="$2"

    if ! command -v yq > /dev/null; then
        echo "yq is not installed. Install it with 'uv pip install yq'" 1>&2
        exit 1
    fi

    yq -i -y --arg proj "$project_name" --arg rev "$new_revision" '
        (.manifest.projects[] | select(.name == $proj) | .revision) = $rev
    ' west.yml
}

update_zephelin_patches()
{
    local zephyr_rev="$1"
    if [ -n "$zephyr_rev" ]; then
        ln -sfn "./patches-releases/${zephyr_rev}" ../zephelin/zephyr/patches
        ln -sfn "./patches-releases/${zephyr_rev}.yml" ../zephelin/zephyr/patches.yml
    fi
}

download_sdk()
{
    local zephyr_sdk_version="$1"
    ./scripts/download_zephyr_sdk.sh "$zephyr_sdk_version"
}

update_all()
{
    local zephyr_rev="$1"
}

ACTION="all"
case "$1" in
    west_revision|zephelin_patches|download_sdk)
        ACTION="$1"
        VERSION="$2"
        ;;
    *)
        VERSION="$1"
        ;;
esac

case "$VERSION" in
    v4.3.*)
        ZEPHYR_SDK_VERSION="0.17.4"
        ZEPHYR_REV="v4.3.0"
        TFLITE_REV="zephyr-v4.1.0"
        PATCH_REV="v4.3.0"
        ;;
    v4.4.*)
        ZEPHYR_SDK_VERSION="1.0.1"
        ZEPHYR_REV="v4.4.1"
        TFLITE_REV="zephyr-v4.4.0"
        PATCH_REV="v4.4.1"
        ;;
    v4.5.*)
        ZEPHYR_SDK_VERSION="1.0.1"
        ZEPHYR_REV="v4.5.0"
        TFLITE_REV="zephyr-v4.5.0"
        # Zephelin is not yet supported for the v4.5.0-rc1 revision.
        # Use the newest version of patches but skip Zephelin.
        PATCH_REV="v4.4.1"
        SKIP_ZEPHELIN=1
        ;;
    main)
        ZEPHYR_SDK_VERSION="1.0.1"
        ZEPHYR_REV="main"
        TFLITE_REV="zephyr-v4.5.0"
        # Zephelin is not yet supported for the main revision.
        # Use the newest version of patches but skip Zephelin.
        PATCH_REV="v4.4.1"
        SKIP_ZEPHELIN=1
        ;;
    *)
        usage
        exit 1
        ;;
esac

if [ "$ACTION" = "all" ] || [ "$ACTION" = "west_revision" ]; then
    update_west_revision zephyr "$ZEPHYR_REV"
    update_west_revision tflite-micro "$TFLITE_REV"
fi

if [ "$ACTION" = "all" ] || [ "$ACTION" = "zephelin_patches" ]; then
    update_zephelin_patches "$PATCH_REV"
    if [ -n "$SKIP_ZEPHELIN" ]; then
        echo "SKIP_ZEPHELIN=1" >> .env
    fi
fi

if [ "$ACTION" = "all" ] || [ "$ACTION" = "download_sdk" ]; then
    download_sdk "$ZEPHYR_SDK_VERSION"
fi
