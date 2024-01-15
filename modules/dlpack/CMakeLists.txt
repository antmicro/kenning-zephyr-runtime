# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_ZEPHYR_DLPACK_MODULE)
  set(DLPACK_DIR ${ZEPHYR_CURRENT_MODULE_DIR})

  zephyr_library()

  zephyr_include_directories(
    ${DLPACK_DIR}/include/
  )
endif(CONFIG_ZEPHYR_DLPACK_MODULE)
