# Copyright (c) 2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

# Adds board-repl target that generates REPL file needed to simulate selected
# board in Renode. The REPL will be saved to build/<board_name>.repl.
function(kenning_add_board_repl_target)
  add_custom_command(
    OUTPUT
      ${BOARD}.repl
    COMMAND
      dts2repl
        --output ${BOARD}.repl
        zephyr/zephyr.dts
  )

  add_custom_target(
    board-repl
    DEPENDS
      ${BOARD}.repl
  )
endfunction()
