import subprocess
import sys
from pathlib import Path

from pytest import CaptureFixture

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

GUS_EXEC = "./target/gustav_release"
MARKER = "// expect: "

EXCLUDED_SCENARIOS: tuple[str, ...] = (
    "builtins.gus",
    "loop_statement.gus",
    "break_continue_errors.gus",
    "break_continue.gus",
    "ternary.gus",
)


def extract_expected(file_path: Path) -> list[str]:
    expected: list[str] = []

    with file_path.open() as file:
        for line in file:
            if MARKER in line:
                _, _, part = line.partition(MARKER)
                part = part.replace("\n", "")
                expected.append(part)

    return expected


def run_file(capfd: CaptureFixture, file_path: Path) -> list[str]:
    args = [*GUS_EXEC.split(), str(file_path)]
    subprocess.run(args)  # noqa: PLW1510

    captured = capfd.readouterr()
    output = captured.out + captured.err

    result = output.splitlines()

    if result and result[0].startswith("Gustav v"):
        result.pop(0)

    if result and result[0] == "":
        result.pop(0)

    return result


def make_relative(test_files: list[Path]) -> list[str]:
    return [file.relative_to(Path(__file__).parent).as_posix() for file in test_files]


def collect_tests() -> list[Path]:
    root = Path(__file__).parent / "scenarios"

    return sorted(
        filter(
            lambda file: (
                file.name not in EXCLUDED_SCENARIOS
                and "benchmark" not in file.parts
                and "scanning" not in file.parts
            ),
            root.rglob("*.gus"),
        )
    )
