import sys
import subprocess
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from gustav.gustav import Scanner

GUS_EXEC = f"{sys.executable} -m gustav"
MARKER = "// expect: "


def extract_expected(file_path: Path) -> list[str]:
    expected: list[str] = []

    with file_path.open() as file:
        for line in file:
            if MARKER in line:
                _, _, part = line.partition(MARKER)
                part = part.replace("\n", "")

                expected.append(part)

    return expected


def run_file(file_path: Path) -> list[str]:
    args = [*GUS_EXEC.split(), str(file_path)]
    proc = subprocess.run(args, capture_output=True, text=True)

    return (proc.stdout + proc.stderr).splitlines()


def compare_output(file_path: Path) -> bool:
    expected = extract_expected(file_path)
    actual = run_file(file_path)

    return expected == actual


def run_scanner(source: str) -> list[str]:
    return list(map(lambda token: token.__test_repr__(), Scanner(source).get_tokens()))


def make_relative(test_files: list[Path]) -> list[str]:
    return list(
        map(
            lambda file: file.relative_to(Path(__file__).parent).as_posix(),
            test_files,
        )
    )


def collect_tests(test_scanner: bool = False) -> list[Path]:
    root = Path(__file__).parent / "scenarios"

    return sorted(
        filter(
            lambda file: "scanning" in file.parts
            if test_scanner
            else "scanning" not in file.parts,
            root.rglob("*.gus"),
        )
    )
