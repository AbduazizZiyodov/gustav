from pathlib import Path

import pytest

from tests.utils import collect_tests, extract_expected, make_relative, run_file

interpreter_tests = collect_tests()


# NOTE(abduaziz): acts kind of test "generator"
#   from scenarios
@pytest.mark.parametrize(
    "file_path", interpreter_tests, ids=make_relative(test_files=interpreter_tests)
)
def test(file_path: Path) -> None:
    expected = extract_expected(file_path)
    actual = run_file(file_path)
    assert actual == expected, f"[FAILED] '{file_path}'"
