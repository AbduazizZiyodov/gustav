import pytest

from pathlib import Path

from .utils import extract_expected, run_scanner, collect_tests, make_relative


scanner_tests = collect_tests(test_scanner=True)


@pytest.mark.parametrize(
    "file_path",
    scanner_tests,
    ids=make_relative(test_files=scanner_tests),
)  # type: ignore[misc]
def test_scanner_tokens(file_path: Path) -> None:
    source = file_path.read_text()
    expected = extract_expected(file_path)
    actual = run_scanner(source)

    assert actual == expected, f"{file_path} failed"
