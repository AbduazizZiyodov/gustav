import os
import sys
import subprocess

from ._logging import log
from .scanner import Scanner
from .errors import had_error
from .enums import ReplCommands, ReplStatus


def repl_main() -> int:
    log.debug(f"{sys.executable=} {sys.argv[1:]=}")

    if len(sys.argv) > 2:
        print("Usage: /.venv/bin/python3.13 -m gustav [script]")
        return os.EX_USAGE

    if len(sys.argv) == 2:
        return run_file(sys.argv[1])

    return run_prompt()


def run_file(file_name: str) -> int:
    log.debug(f"Running from file: {file_name}")

    try:
        with open(file_name, "r") as source_file:
            source: str = source_file.read()
            exec_source(source)
            return os.EX_DATAERR if (had_error) else os.EX_OK

    except FileNotFoundError as exc:
        log.debug(f"File with {file_name=} is not found: {exc=}")
        return os.EX_NOINPUT


def run_prompt() -> int:
    global had_error

    line_no: int = 1

    while True:
        try:
            line = input(f"[{line_no}] => ")

            if not (line := line.strip()):
                log.debug(f"Ignoring empty lines: {line}")
                continue

            match handle_if_command(line):
                case ReplStatus.NOT_HANDLED, None:
                    exec_source(line)
                    had_error = False
                    line_no += 1
                    continue
                case ReplStatus.HANDLED, result:
                    if isinstance(result, int):
                        return result

                    line_no += 1
                    continue

        except (KeyboardInterrupt, EOFError) as exc:
            sys.stdout.write("\n")
            log.debug(f"Received keyboard interrupt/eof: {exc=}")
            return os.EX_OK
        except Exception as exc:
            sys.stdout.write("\n")
            log.error(f"Unexpected exception occurred: {exc=}")
            return -1


def handle_if_command(line: str) -> tuple[ReplStatus, int | None]:
    # if handler returns INT, it will considered as an exit code
    match line:
        case ReplCommands.CLEAR:
            if os.name in ("nt", "dos"):
                subprocess.call("cls")
            elif os.name in ("linux", "osx", "posix"):
                subprocess.call("clear")
            else:
                pass  # idk
            return ReplStatus.HANDLED, None
        case ReplCommands.EXIT:
            sys.stdout.write("Exiting ...\n")
            return ReplStatus.HANDLED, os.EX_OK
        case ReplCommands.HELP:
            sys.stdout.write("Help text ...\n")
            return ReplStatus.HANDLED, None
        case _:
            return ReplStatus.NOT_HANDLED, None


def exec_source(source: str) -> None:
    log.debug(f"Received: {len(source)=} \n{source=}")

    tokens = Scanner(source).scan_tokens()

    log.debug(f"{'#' * 10} SCANNING COMPLETED {'#' * 10}")

    for token in tokens:
        log.debug(token)
