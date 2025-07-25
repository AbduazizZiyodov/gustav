import pytest

from pathlib import Path

from .utils import extract_expected, run_file, collect_tests, make_relative


interpreter_tests = collect_tests()


@pytest.mark.parametrize(
    "file_path",
    interpreter_tests,
    ids=make_relative(test_files=interpreter_tests),
)  # type: ignore[misc,unused-ignore]
def test_interpreter(file_path: Path) -> None:
    expected = extract_expected(file_path)
    actual = run_file(file_path)

    assert actual == expected, f"{file_path} failed"
