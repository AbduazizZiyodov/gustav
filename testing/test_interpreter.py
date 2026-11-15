from pathlib import Path

import pytest

from .utils import collect_tests, extract_expected, make_relative, run_file

interpreter_tests = collect_tests()


@pytest.mark.parametrize(
    "file_path",
    interpreter_tests,
    ids=make_relative(test_files=interpreter_tests),
)  # type: ignore[misc,unused-ignore]
def test_interpreter(file_path: Path) -> None:
    expected = extract_expected(file_path)

    # recursion error test case
    if "stack_overflow.gus" in file_path.as_posix():
        actual = run_file(file_path, output_as_str=True)
        assert "RecursionError" in actual
        return

    actual = run_file(file_path)
    assert actual == expected, f"{file_path} failed"
