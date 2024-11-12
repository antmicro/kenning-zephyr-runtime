# Copyright (c) 2020-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

"""
Python script for uploading built LLEXT runtime.
"""

import argparse
from pathlib import Path

from kenning.protocols.uart import UARTProtocol
from kenning.core.protocol import MessageType, Message
from kenning.utils.logger import KLogger

if __name__ == "__main__":
    parser = argparse.ArgumentParser(__doc__)

    parser.add_argument(
        "--llext", type=Path, help="Path to the LLEXT binary", required=True
    )
    parser.add_argument("--uart", type=Path, help="Path to the UART", required=True)
    parser.add_argument(
        "--uart-baudrate", type=int, help="Baudrate of the UART", default=115200
    )
    parser.add_argument(
        "--verbosity", "-v", type=str, help="Logging verbosity", default="INFO"
    )

    args = parser.parse_args()
    KLogger.set_verbosity(level=args.verbosity)

    if not args.llext.exists():
        raise FileNotFoundError(f"{args.llext} does not exist")

    if not args.uart.exists():
        raise FileNotFoundError(f"{args.uart} does not exist")

    protocol = UARTProtocol(str(args.uart.resolve()), args.uart_baudrate)

    protocol.initialize_client()

    protocol.send_message(Message(MessageType.RUNTIME, args.llext.read_bytes()))

    protocol.disconnect()
