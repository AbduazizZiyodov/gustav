import os
import sys
import subprocess
from rich import print as printr
from difflib import HtmlDiff


def main() -> int:
    if len(sys.argv) != 2:
        printr(f"Usage: {sys.executable} test.py <TEST_FILE_NAME>")
        return os.EX_DATAERR

    filename: str = sys.argv[1]

    with open(filename) as file:
        source = file.readlines()

    test_cases: list[str] = list(
        map(
            lambda line: "".join(line.split("// expect: ")).replace("\n", ""),
            filter(lambda line: "// expect:" in line, source),
        )
    )

    cmd = f"DEBUG=yes {sys.executable} -m gustav {filename}"

    printr(f"TRACE: {cmd=}")
    printr(f"INFO : Test results should be: {test_cases}")

    result: bytes = subprocess.run(cmd, shell=True, capture_output=True).stderr

    output: str = result.decode()
    printr("[bold blue][OUTPUT][/bold blue]")
    printr(output)
    printr("[bold blue][END OUTPUT][/bold blue]")

    output_lines: list[str] = list()

    debug_log_began: bool = False

    for line in output.split("\n"):
        if "SCANNING COMPLETED" in line:
            debug_log_began = True
            continue

        if debug_log_began and line != "":
            splitted = line.split("DEBUG: ")
            output_lines.append(splitted[1])

    printr("[DEBUG INFO]")
    printr(f"{test_cases=}\n{output_lines=}\n{len(test_cases)=}\n{len(output_lines)=}")
    printr("[END DEBUG INFO]")

    if len(output_lines) != len(test_cases):
        diff = HtmlDiff()
        html_diff = diff.make_file(test_cases, output_lines)

        with open("diff.html", "w", encoding="utf-8") as diff_file:
            diff_file.write(html_diff)

        printr(
            "[bold red][ERROR][/bold red] Not enough lines produced, check diff.html (test cases diff output lines)"
        )
        return os.EX_DATAERR

    passed_count, failed_count = 0, 0

    for expected, got in zip(test_cases, output_lines):
        passed: bool = expected == got
        status = (
            "[bold green]PASSED[/bold green]"
            if passed
            else "[bold red]FAILED[/bold red]"
        )
        printr(f"[{status}] {expected=} {got=}")

        if passed:
            passed_count += 1
        else:
            failed_count += 1

    printr(
        f"[bold blue]Results[/bold blue] => Passed [bold green]{passed_count}[/bold green] Failed [bold red]{failed_count}[/bold red]"
    )

    return os.EX_OK


if __name__ == "__main__":
    raise SystemExit(main())
