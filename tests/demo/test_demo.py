# Copyright (c) 2023-2024 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

from pathlib import Path

from pyrenode3.wrappers import Emulation, TerminalTester
import pytest

runtime_binaries = [binary for binary in Path("./artifacts/").glob("*.elf")]


class TestDemo:
    @pytest.mark.parametrize(
        "binary", runtime_binaries, ids=[b.stem for b in runtime_binaries]
    )
    def test_demo(self, binary: Path):
        *_, board = binary.stem.split("-")
        repl_path = Path(f"artifacts/{board}.repl")

        if not repl_path.exists():
            pytest.xfail(f"Missing repl for {board}")

        emulation = Emulation()

        platform = emulation.add_mach(board)
        platform.load_repl(repl_path.resolve())
        platform.load_elf(binary)

        testers = [
            TerminalTester(getattr(platform.sysbus, uart_name), 30)
            for uart_name in dir(platform.sysbus)
            if "uart" in uart_name or "usart" in uart_name
        ]

        emulation.StartAll()

        assert any(
            tester.WaitFor("I: inference done", includeUnfinishedLine=True)
            for tester in testers
        ), "\n\n".join(tester.GetReport() for tester in testers)

        emulation.clear()
