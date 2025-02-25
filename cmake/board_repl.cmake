# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

# Adds board-repl target
function(kenning_add_board_repl_target)
  add_custom_command(
    OUTPUT
      ${BOARD}.repl
    COMMAND
      gcc
        -H -E -P -x assembler-with-cpp
        -I ${ZEPHYR_BASE}/include/
        -I ${ZEPHYR_BASE}/dts/
        -I ${ZEPHYR_BASE}/dts/arm/
        -I ${ZEPHYR_BASE}/dts/riscv/
        -I ${ZEPHYR_BASE}/dts/common/
        -I ${WEST_TOPDIR}/modules/hal/stm32/dts/
        ${dts_files}
        1>${BOARD}_flat.dts
        2>includes.txt
    COMMAND
      dts2repl
        --output ${BOARD}.repl
        ${BOARD}_flat.dts
  )

  add_custom_target(
    board-repl
    DEPENDS
      ${BOARD}.repl
  )
endfunction()
