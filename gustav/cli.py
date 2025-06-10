import os
import sys

from .types import Token
from .logging import log
from .parser import Parser
from .scanner import Scanner
from .ast_printer import AstPrinter
from .errors import reset_had_error, get_had_error

__all__ = ["cli_handler"]


def cli_handler() -> int:
    if len(sys.argv) > 2:
        print(f"Usage: {sys.executable} -m gustav [script]")
        return os.EX_USAGE

    if len(sys.argv) == 2:
        return run_from_file(sys.argv[1])

    return repl_main()


def run_from_file(file_name: str) -> int:
    log.debug(f"Running from file: {file_name}")

    try:
        with open(file_name, "r") as source_file:
            exec_source(source_file.read())

    except FileNotFoundError as exc:
        log.debug(f"File with {file_name=} is not found: {exc=}")
        return os.EX_NOINPUT
    else:
        return os.EX_DATAERR if (get_had_error()) else os.EX_OK


def repl_main() -> int:
    line_no: int = 1
    is_running: bool = True

    while is_running:
        try:
            line = input(f"[{line_no}] => ")

            if not (line.strip()):
                log.debug(f"Ignoring empty lines: {line}")
                continue

            exec_source(line)
            reset_had_error()
            line_no += 1

        except (KeyboardInterrupt, EOFError) as exc:
            sys.stdout.write("\n")
            log.debug(f"Received keyboard interrupt/eof: {exc=}")
            return os.EX_OK

    return os.EX_OK


def exec_source(source: str) -> None:
    source += "\n"
    log.debug(
        f"Received source's length is {len(source)} chars & contents of source:\n{source}"
    )
    scanner = Scanner(source)
    tokens: list[Token] = scanner.get_tokens()

    log.debug(f"{'#' * 10} SCANNING COMPLETED {'#' * 10}")

    for token in tokens:
        log.debug(token)

    log.debug(f"{'#' * 10} BEGIN PARSING {'#' * 10}")

    parser: Parser = Parser(tokens)
    expression = parser.parse()

    if had_error := get_had_error():  # noqa: F841
        log.debug("Parsing failed, returning ...")
        return

    log.debug(f"{'#' * 10} AST PRINT {'#' * 10}")
    log.debug(f"Parse result => {expression=}")

    printer = AstPrinter()
    representation: str = printer.get_string_repr(expression)
    print(representation)
