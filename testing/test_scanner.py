from pathlib import Path

import pytest

from .utils import collect_tests, extract_expected, make_relative, run_scanner

scanner_tests = collect_tests(test_scanner=True)


@pytest.mark.parametrize(
    "file_path",
    scanner_tests,
    ids=make_relative(test_files=scanner_tests),
)  # type: ignore[misc,unused-ignore]
def test_scanner_tokens(file_path: Path) -> None:
    source = file_path.read_text()
    expected = extract_expected(file_path)
    actual = run_scanner(source)

    assert actual == expected, f"{file_path} failed"
