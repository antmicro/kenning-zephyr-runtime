# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(kenning_inference_lib)

add_compile_definitions(__UNIT_TEST__=1)

if ("${TESTED_MODULE}" STREQUAL "MODEL")
  target_sources(testbinary PRIVATE
    src/core/test_model.c
    ../../../lib/kenning_inference_lib/core/model.c
  )

  target_include_directories(testbinary PRIVATE
    ../../../include
    src
  )
elseif ("${TESTED_MODULE}" STREQUAL "UART")
  target_sources(testbinary PRIVATE
    src/protocols/test_uart.c
    ../../../lib/kenning_inference_lib/protocols/uart.c
  )

  target_include_directories(testbinary PRIVATE
    ../../../include
    src
  )
elseif ("${TESTED_MODULE}" STREQUAL "KENNING_PROTOCOL")
  target_sources(testbinary PRIVATE
    src/core/test_kenning_protocol.c
    ../../../lib/kenning_inference_lib/core/kenning_protocol.c
  )

  target_include_directories(testbinary PRIVATE
    ../../../include
    src
  )
elseif ("${TESTED_MODULE}" STREQUAL "CALLBACKS")
  target_sources(testbinary PRIVATE
    src/core/test_callbacks.c
    ../../../lib/kenning_inference_lib/core/callbacks.c
  )

  target_include_directories(testbinary PRIVATE
    ../../../include
    src
  )
elseif ("${TESTED_MODULE}" STREQUAL "INFERENCE_SERVER")
  target_sources(testbinary PRIVATE
    src/core/test_inference_server.c
    ../../../lib/kenning_inference_lib/core/inference_server.c
  )

  target_include_directories(testbinary PRIVATE
    ../../../include
    src
  )
endif()
