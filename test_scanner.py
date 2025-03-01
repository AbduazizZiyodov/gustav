import os
import sys
import subprocess
from rich import print


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.executable} test.py <TEST_FILE_NAME>")
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

    print(f"TRACE: {cmd=}")
    print(f"INFO : Test results should be: {test_cases}")

    result: bytes = subprocess.run(cmd, shell=True, capture_output=True).stderr

    output: str = result.decode()
    print("[bold blue][OUTPUT][/bold blue]")
    print(output)
    print("[bold blue][END OUTPUT][/bold blue]")

    output_lines: list[str] = list()

    debug_log_began: bool = False

    for line in output.split("\n"):
        if "SCANNING COMPLETED" in line:
            debug_log_began = True
            continue

        if debug_log_began and line != "":
            splitted = line.split("DEBUG: ")
            output_lines.append(splitted[1])

    print("[DEBUG INFO]")
    print(f"{test_cases=}\n{output_lines=}\n{len(test_cases)=}\n{len(output_lines)=}")
    print("[END DEBUG INFO]")

    assert len(output_lines) == len(test_cases), "Not enough lines produced"

    passed_count, failed_count = 0, 0

    for expected, got in zip(test_cases, output_lines):
        passed: bool = expected == got
        status = (
            "[bold green]PASSED[/bold green]"
            if passed
            else "[bold red]FAILED[/bold red]"
        )
        print(f"[{status}] {expected=} {got=}")

        if passed:
            passed_count += 1
        else:
            failed_count += 1

    print(
        f"[bold blue]Results[/bold blue] => Passed [bold green]{passed_count}[/bold green] Failed [bold red]{failed_count}[/bold red]"
    )

    return os.EX_OK


if __name__ == "__main__":
    raise SystemExit(main())
