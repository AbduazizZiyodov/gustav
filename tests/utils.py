import subprocess
import sys
from pathlib import Path

from pytest import CaptureFixture

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

GUS_EXEC = "./target/gustav_release"
MARKER = "// expect: "
EXIT_MARKER = "// expect exit: "

# TODO(abduaziz): these features must be implemented !
EXCLUDED_SCENARIOS: tuple[str, ...] = (
    "loop_statement.gus",
    "break_continue_errors.gus",
    "break_continue.gus",
    "ternary.gus",
    "lambda.gus",
    "pipe.gus",
)


def extract_expected(file_path: Path) -> tuple[list[str], int | None]:
    expected: list[str] = []
    expected_exit_code: int | None = None

    with file_path.open() as file:
        for line in file:
            if MARKER in line:
                _, _, part = line.partition(MARKER)
                part = part.replace("\n", "")
                expected.append(part)

            if EXIT_MARKER in line and expected_exit_code is None:
                _, _, part = line.partition(EXIT_MARKER)
                part = part.strip()
                expected_exit_code = int(part)

    return expected, expected_exit_code


def run_file(capfd: CaptureFixture, file_path: Path) -> tuple[list[str], str, int]:
    args = [*GUS_EXEC.split(), str(file_path)]
    result = subprocess.run(args)  # noqa: PLW1510

    captured = capfd.readouterr()
    output = captured.out + captured.err

    extracted_result = output.splitlines()

    if extracted_result and extracted_result[0].startswith("Gustav v"):
        extracted_result.pop(0)

    if extracted_result and extracted_result[0] == "":
        extracted_result.pop(0)

    return extracted_result, file_path.open().read(), result.returncode


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
                and "multiline_comments" not in file.parts
            ),
            root.rglob("*.gus"),
        )
    )
