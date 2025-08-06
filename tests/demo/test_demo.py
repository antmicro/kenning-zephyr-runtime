# Copyright (c) 2023-2025 Antmicro <www.antmicro.com>
#
# SPDX-License-Identifier: Apache-2.0

import re
from pathlib import Path

import numpy as np
import pytest
from pyrenode3.wrappers import Emulation, TerminalTester

RUNTIME_BINARIES = [binary for binary in Path("./artifacts/").glob("*.elf")]
RUNTIME_BINARIES.sort()
FLOAT_RE = r"(?:-?[0-9]+.[0-9]+)|(?:nan)"
MODEL_OUTPUT_RE = re.compile(
    rf"I: model output: \[wing: ({FLOAT_RE}), ring: ({FLOAT_RE}), "
    rf"slope: ({FLOAT_RE}), negative: ({FLOAT_RE})\]"
)
EXPECTED_PREDICTIONS = [0, 3, 2, 0, 1, 2, 0, 0, 0, 2]


@pytest.fixture(autouse=True)
def test_setup_cleanup():
    # setup
    pass

    # run test
    yield

    # cleanup
    Emulation().clear()


class TestDemo:
    @pytest.mark.parametrize(
        "binary", RUNTIME_BINARIES, ids=[b.stem for b in RUNTIME_BINARIES]
    )
    def test_demo(self, binary: Path):
        *_, board = binary.stem.split("-")
        repl_path = Path(f"artifacts/{board}.repl")

        if not repl_path.exists():
            pytest.xfail(f"Missing repl for {board}")

        emulation = Emulation()

        platform = emulation.add_mach(board)
        if platform is None:
            # workaround for some Renode versions
            platform = emulation.get_mach(board)

        platform.load_repl(repl_path.resolve())
        platform.load_elf(binary.resolve())

        testers = [
            TerminalTester(getattr(platform.sysbus, uart_name), 30)
            for uart_name in dir(platform.sysbus)
            if "uart" in uart_name or "usart" in uart_name
        ]

        emulation.StartAll()

        assert any(
            tester.WaitFor(
                "I: inference done",
                timeout=None,
                treatAsRegex=False,
                includeUnfinishedLine=True,
                pauseEmulation=False,
                matchNextLine=False,
            )
            for tester in testers
        ), "\n\n".join(tester.GetReport() for tester in testers)

        runtime_log = None
        for tester in testers:
            if "Booting Zephyr OS" in tester.GetReport():
                runtime_log = tester.GetReport()
                break

        predictions = re.findall(MODEL_OUTPUT_RE, runtime_log)

        assert len(EXPECTED_PREDICTIONS) == len(
            predictions
        ), f"invalid predictions:\n{predictions}"

        predictions = np.array(predictions, dtype=np.float32).argmax(axis=1)

        assert (
            EXPECTED_PREDICTIONS == predictions.tolist()
        ), f"Invalid predictions:\n{predictions}"
