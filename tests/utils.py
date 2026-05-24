import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

GUS_EXEC = "./target/gustav_release"
MARKER = "// expect: "

EXCLUDED_SCENARIOS: tuple[str, ...] = (
    "builtins.gus",
    "loop_statement.gus",
    "break_continue_errors.gus",
    "break_continue.gus",
    "ternary.gus"
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


def run_file(file_path: Path, output_as_str: bool = False) -> str | list[str]:
    args = [*GUS_EXEC.split(), str(file_path)]
    proc = subprocess.run(args, capture_output=True, text=True)

    output = proc.stdout + proc.stderr

    result = output.splitlines()

    if result[0].startswith("Gustav v"):
        result.pop(0)

    if result[0] == "":
        result.pop(0)

    return result


def make_relative(test_files: list[Path]) -> list[str]:
    return list(
        map(
            lambda file: file.relative_to(Path(__file__).parent).as_posix(),
            test_files,
        )
    )


def collect_tests() -> list[Path]:
    root = Path(__file__).parent / "scenarios"

    return sorted(
        filter(
            lambda file: file.name not in EXCLUDED_SCENARIOS
            and "benchmark" not in file.parts
            and "scanning" not in file.parts,
            root.rglob("*.gus"),
        )
    )
