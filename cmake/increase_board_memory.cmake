# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

# Adds target that generates board overlay with increase memory based on
# CONFIG_KENNING_INCREASE_MEMORY and CONFIG_KENNING_INCREASE_MEMORY_SIZE
# and adds boards overlay with increased memory to the build if
# CONFIG_KENNING_INCREASE_MEMORY is set
function(kenning_increase_board_memory)
  string(REPLACE "/" "_" BOARD_SANITIZED "${BOARD}")
  add_custom_command(
    OUTPUT
      "${CMAKE_BINARY_DIR}/increased_memory.overlay"
    COMMAND
      python3 ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../scripts/gen_increased_memory_overlay.py
        --output-path "${CMAKE_SOURCE_DIR}/boards/${BOARD_SANITIZED}_increased_memory.overlay"
        --build-path ${CMAKE_BINARY_DIR}
        --memory-size-kb "${CONFIG_KENNING_INCREASE_MEMORY_SIZE}"
  )

  add_custom_target(
    increase-memory
    DEPENDS
      "${CMAKE_BINARY_DIR}/increased_memory.overlay"
  )

  if(CONFIG_KENNING_INCREASE_MEMORY)
    string(REPLACE "/" "_" BOARD_SANITIZED "${BOARD}")
    set(OVERLAY_FILE_PATH "${CMAKE_SOURCE_DIR}/boards/${BOARD_SANITIZED}_increased_memory.overlay")
    if(EXISTS "${OVERLAY_FILE_PATH}")
      set(EXTRA_DTC_OVERLAY_FILE "${OVERLAY_FILE_PATH}" CACHE INTERNAL "EXTRA_DTC_OVERLAY_FILE")
    endif(EXISTS "${OVERLAY_FILE_PATH}")
  endif(CONFIG_KENNING_INCREASE_MEMORY)
endfunction()
