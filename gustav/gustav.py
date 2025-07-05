import os
import sys
import typing as t  # noqa: F401

from pprint import pformat

from gustav.parser import Parser
from gustav.scanner import Scanner
from gustav.logging import LOG, DEBUG
from gustav.interpreter import Interpreter
from gustav.ast.statement import Statement
from gustav.exceptions import GusRuntimeError
from gustav.token import Token, TokenType as T

rich_installed = False

try:
    from rich import print as printr

    rich_installed = True
except ImportError:
    LOG.debug("Rich is not installed, to install run 'uv add --dev rich'")

__all__ = "report", "error", "panic", "main"

had_error: bool = False
had_runtime_error: bool = False


def main() -> int:
    if len(sys.argv) > 2:
        print(f"Usage: {sys.executable} -m gustav [script]")
        return os.EX_USAGE

    if len(sys.argv) == 2:
        return run_from_file(sys.argv[1])

    return run_from_repl()


def run(source: str) -> None:
    if not source.endswith("\n"):
        source += "\n"

    scanner = Scanner(source)
    tokens: list[Token] = scanner.get_tokens()

    if had_error:
        LOG.debug("Scanning failed, exiting")
        return

    LOG.debug(f"Scanning completed, {len(tokens)} tokens scanned")

    for index, token in enumerate(tokens):
        LOG.debug(f"{index} => {token}")

    parser: Parser = Parser(tokens)

    statements: list[Statement] = parser.parse()
    LOG.debug("Parsing finished\n")

    if DEBUG:  # log statements on DEBUG mode
        for index, statement in enumerate(statements):
            if rich_installed:
                printr(f"[bold green]Statement[/bold green] {index}:")
                printr(statement)
                print()
            else:
                formatted_statement = pformat(statement)
                LOG.debug(f"{index} => {formatted_statement}")

    LOG.debug("Interpretation result:")

    interpreter = Interpreter()
    interpreter.interpret(statements)


def run_from_file(file_name: str) -> int:
    LOG.debug(f"Running from file: {file_name=}")

    source: str = str()

    try:
        with open(file_name, "r") as file:
            source = file.read()

    except FileNotFoundError as exc:
        LOG.debug(f"File with {file_name=} is not found: {exc=}")
        sys.stderr.write(f"Couldn't find file with {file_name=}")
        return os.EX_NOINPUT

    run(source)

    if had_error:
        return os.EX_DATAERR

    if had_runtime_error:
        return 70

    return os.EX_OK


def run_from_repl() -> int:
    PROMPT = "=>"

    while True:
        try:
            line = input(f"{PROMPT} ")
        except (KeyboardInterrupt, EOFError) as exc:
            sys.stdout.write("\n")
            LOG.debug(f"Received keyboard interrupt/eof: {exc=}")
            break

        if not (line.strip()):
            LOG.debug("Ignoring empty line")
            continue

        if not line.endswith(";"):
            line += ";"  # only for repl
            sys.stderr.write("Warning: do not forget about semicolon(';') at the end\n")

        run(line)

        global had_error
        had_error = False

    return os.EX_OK


def panic(line: int, message: str) -> None:
    report(line, "", message)


def report(line: int, where: str, message: str) -> None:
    global had_error
    had_error = True
    print(f"[line {line}] Error {where}: {message}", file=sys.stderr)


def error(token: Token, message: str) -> None:
    if token.type != T.EOF:
        report(token.line, " at the end", message)
    else:
        report(token.line, f" at '{token.lexeme}'", message)


def runtime_error(exc: GusRuntimeError) -> None:
    global had_runtime_error
    print(
        f"{exc.error_message} at [line {exc.token.line}, token={exc.token.type}]",
        file=sys.stderr,
    )
    had_runtime_error = True
