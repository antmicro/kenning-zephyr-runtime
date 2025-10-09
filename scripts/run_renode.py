#!/usr/bin/env python3

# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Python script for running Kenning Zephyr Runtime in Renode.
"""

import argparse
import re

import serial
from pyrenode3.wrappers import Emulation


def get_cmake_var(cmake_var: str) -> str:
    """
    Retrieves variable from CMake cache.
    """
    with open("./build/CMakeCache.txt", "r") as cache_file:
        cmake_cache = cache_file.read()

    match = re.findall(rf"^{cmake_var}=([^\n\t\s]*)", cmake_cache, re.MULTILINE)
    if len(match):
        return match[0]

    raise Exception(f"{cmake_var} variable not found in CMake cache")


def get_zephyr_console_uart(board: str) -> str:
    """
    Retrieves Zephyr console UART from device tree.
    """
    with open("./build/zephyr/zephyr.dts", "r") as dts_file:
        board_dts = dts_file.read()

    match = re.findall(r"zephyr,console = &([a-zA-Z0-9]*);", board_dts, re.MULTILINE)
    if len(match):
        return match[0]

    raise Exception("Zephyr console UART not found")


def get_kenning_communication_uart(boards: str) -> str:
    """
    Retrieves UART used for Kenning communication from device tree.
    """
    with open("./build/zephyr/zephyr.dts", "r") as dts_file:
        board_dts = dts_file.read()

    match = re.findall(r"kcomms = &([a-zA-Z0-9]*);", board_dts, re.MULTILINE)
    if len(match):
        return match[0]

    raise Exception("Kenning communication UART not found")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)
    parser.add_argument("--debug", action="store_true", help="Enable GDB server")
    parser.add_argument("--no-kcomms", action="store_true", help="Disable Kenning communication")
    args = parser.parse_args()

    board = get_cmake_var("BOARD:STRING").split("/")[0]
    build_path = get_cmake_var("APPLICATION_BINARY_DIR:PATH")
    project_name = get_cmake_var("CMAKE_PROJECT_NAME:STATIC")

    emulation = Emulation()

    platform = emulation.add_mach(board)
    platform.load_repl(f"{build_path}/{board}.repl")
    platform.load_elf(f"{build_path}/zephyr/zephyr.elf")

    if args.debug:
        platform.StartGdbServer(3333)
        print("gdb server started at :3333")
        print("Press ENTER to start simulation")

        input()

    # create pty terminal for UART with logs
    console_uart = get_zephyr_console_uart(board)
    emulation.CreateUartPtyTerminal("console_uart_term", "/tmp/uart-log")
    emulation.Connector.Connect(
        getattr(platform.sysbus, console_uart).internal,
        emulation.externals.console_uart_term,
    )

    console = serial.Serial("/tmp/uart-log", baudrate=115200)

    if "app" == project_name and not args.no_kcomms:
        # create pty terminal for Kenning communication UART
        kcomms_uart = get_kenning_communication_uart(board)

        emulation.CreateUartPtyTerminal("kcomms_uart_term", "/tmp/uart")
        emulation.Connector.Connect(
            getattr(platform.sysbus, kcomms_uart).internal,
            emulation.externals.kcomms_uart_term,
        )

    print("Starting Renode simulation. Press CTRL+C to exit.")
    emulation.StartAll()

    logs_tail = ""
    while True:
        try:
            logs = console.read_all().decode(errors="ignore")

            print(logs, end="", flush=True)

            logs_tail = logs_tail.split("\n")[-1] + logs if logs_tail else logs
            if (
                "demo_app" == project_name
                and "I: inference finished successfully" in logs_tail
                and "\n" in logs_tail
            ):
                break
        except KeyboardInterrupt:
            print("Exiting...")
            break
        except Exception:
            raise

    console.close()
    emulation.clear()
