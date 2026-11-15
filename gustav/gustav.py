import os
import sys
import typing as t  # noqa: F401

from rich import print as printr

from gustav.ast.statement import Statement
from gustav.compiler import Compiler
from gustav.enums import TokenType as TT
from gustav.exceptions import GusRuntimeError
from gustav.logging import LOG
from gustav.parser import Parser
from gustav.scanner import Scanner
from gustav.token import Token

__all__ = (
    "error",
    "main",
    "panic",
    "report",
)

had_error: bool = False
had_runtime_error: bool = False


def main() -> int:
    if len(sys.argv) != 2:  # pragma: no cover
        sys.stdout.write(f"Usage: {sys.executable} -m gustav [script]")
        return os.EX_USAGE

    return run_from_file(sys.argv[1])


def run(source: str) -> None:
    if not source.endswith("\n"):
        source += "\n"

    scanner = Scanner(source)

    LOG.debug("Scanning ...")

    tokens: list[Token] = scanner.get_tokens()

    if had_error:
        LOG.debug("Scanning failed, exiting")
        return

    LOG.debug(f"Scanning finished, got {len(tokens)} tokens")

    if len(tokens) == 1 and tokens[0].type == TT.EOF:
        LOG.debug(f"No tokens received {tokens=}")
        return

    parser: Parser = Parser(tokens)

    LOG.debug("Parsing ...")

    statements: list[Statement] = parser.parse()

    LOG.debug("Parsing finished")

    for index, statement in enumerate(statements):
        printr(f"[bold green]Statement[/bold green] {index}:")
        printr(statement)
        sys.stdout.write("\n")

    compiler = Compiler()

    if had_error:
        LOG.debug("Resolving failed")
        return

    LOG.debug("compiling ...")
    compiler.do_compile(statements)


def run_from_file(file_name: str) -> int:
    LOG.debug(f"Running from file: {file_name=}")

    source: str = ""

    try:
        with open(file_name, "r", encoding="utf-8") as file:
            source = file.read()

    except FileNotFoundError:  # pragma: no cover
        sys.stderr.write(f"Couldn't find file with {file_name=}")
        return os.EX_NOINPUT

    run(source)

    if had_error:
        return os.EX_DATAERR

    if had_runtime_error:
        return 70

    return os.EX_OK


def panic(line: int, message: str) -> None:
    report(line, "", message)


def warning(token: Token, message: str) -> None:
    sys.stderr.write(f"[WARNING] at line {token.line}: {message}.\n")


def error(token: Token, message: str) -> None:
    if token.type == TT.EOF:
        report(token.line, " at end", message)
    else:
        report(token.line, f" at '{token.lexeme}'", message)


def runtime_error(exc: GusRuntimeError) -> None:
    global had_runtime_error
    had_runtime_error = True

    sys.stderr.write(f"[at line {exc.token.line}] {exc.error_message}.\n")


def report(line: int, where: str, message: str) -> None:
    global had_error
    had_error = True

    sys.stderr.write(f"[at line {line}] Error{where}: {message}.\n")
