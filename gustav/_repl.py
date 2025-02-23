import os
import sys

from ._logging import log
from .scanner import Scanner
from .errors import had_error


def repl_main() -> int:
    log.debug(f"{sys.executable=} {sys.argv[1:]=}")

    if len(sys.argv) > 2:
        print("Usage: python3 -m lil.py [script]")
        return os.EX_USAGE

    if len(sys.argv) == 2:
        return run_file(sys.argv[1])

    return run_prompt()


def execute(source: str) -> None:
    log.debug(f"Received: {len(source)=} \n{source=}")

    scanner = Scanner(source)
    tokens = scanner.scan_tokens()

    log.debug(f"{'#' * 10} SCANNING COMPLETED {'#' * 10}")

    for token in tokens:
        log.debug(token)


def run_file(file_name: str) -> int:
    log.debug(f"Running from file: {file_name}")

    try:
        with open(file_name, "r") as source_file:
            source: str = source_file.read()
            execute(source)
            return os.EX_DATAERR if (had_error) else os.EX_OK

    except FileNotFoundError as exc:
        log.debug(f"File with {file_name=} is not found: {exc=}")
        return os.EX_NOINPUT


def run_prompt() -> int:
    global had_error

    while True:
        try:
            line = input("=> ")
            execute(line)
            had_error = False
        except (KeyboardInterrupt, EOFError) as exc:
            log.debug(f"Received keyboard interrupt/eof: {exc=}")
            return os.EX_OK
