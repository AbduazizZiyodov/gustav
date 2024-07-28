import os
import sys
import logging

from none.enums import Token
from none.scanner import Scanner

log = logging.getLogger(__name__)

logging.basicConfig(
    level=logging.DEBUG if os.getenv("DEBUG_LOGGING") else logging.INFO,
    format="%(asctime)s [%(levelname)s]: %(message)s",
    datefmt="%Y-%m-%d %H:%M",
)

had_error = False


def main() -> int:
    log.debug(f"{sys.executable=} {sys.argv[1:]=}")

    if len(sys.argv) > 2:
        print("Usage: python3 -m none [script]")
        return os.EX_USAGE

    if len(sys.argv) == 2:
        return run_file(sys.argv[1])

    return run_prompt()


def run(source: str) -> int:
    log.info(f"Received {source=}")
    scanner = Scanner(source)
    tokens: list[Token] = scanner.scan_tokens()

    print("Tokens: ")
    for token in tokens:
        print(token)


def run_file(file_name: str) -> int:
    log.debug(f"Running from file: {file_name}")

    try:
        with open(file_name, "r") as source_file:
            source: str = source_file.read()
            run(source)
            return os.EX_DATAERR if (had_error) else os.EX_OK

    except FileNotFoundError as exc:
        log.debug(f"File with {file_name=} is not found: {exc=}")
        return os.EX_NOINPUT


def run_prompt() -> int:
    global had_error

    while True:
        try:
            line = input("=> ")
            run(line)
            had_error = False
        except KeyboardInterrupt as exc:
            log.debug(f"Received keyboard interrupt(sigint): {exc=}")
            return os.EX_OK


def error(line: int, message: str) -> None:
    report(line, "", message)


def report(line: int, where: str, message: str) -> None:
    global had_error
    print(f"[line {line}] Error {where}: {message}", file=sys.stderr)
    had_error = True
