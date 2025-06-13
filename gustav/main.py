import os
import sys
import typing as t  # noqa: F401

from pprint import pformat

from .ast import Statement
from .parser import Parser
from .scanner import Scanner
from .logging import LOG, DEBUG
from .token import Token, TokenType
from .interpreter import Interpreter
from .exceptions import GusRuntimeError

rich_installed = False

try:
    from rich import print as printr

    rich_installed = True
except ImportError:
    LOG.debug("Rich is not installed, to install run 'uv add --dev rich'")


__all__ = ["Gustav"]


class Gustav:
    line_no: int = 1
    exit_code: int = os.EX_OK  # default
    repl_is_running: bool = False

    had_error: bool = False
    had_runtime_error: bool = False

    def main(self) -> int:
        if len(sys.argv) > 2:
            print(f"Usage: {sys.executable} -m gustav [script]")
            return os.EX_USAGE

        if len(sys.argv) == 2:
            return self.run_from_file(sys.argv[1])

        return self.run_from_repl()

    def run(self, source: str) -> None:
        if not source.endswith("\n"):
            source += "\n"

        scanner = Scanner(source, self)
        tokens: list[Token] = scanner.get_tokens()

        if self.had_error:
            LOG.debug("Scanning failed, exiting ...")
            return

        LOG.debug(f"Scanning completed, {len(tokens)} tokens scanned:")

        for index, token in enumerate(tokens):
            LOG.debug(f"{index} => {token}")

        parser: Parser = Parser(tokens, self)

        statements: list[Statement] = parser.parse()
        LOG.debug("Parsing finished\n")

        if DEBUG:
            for index, statement in enumerate(statements):
                if rich_installed:
                    printr(f"[bold green]Statement[/bold green] {index}:")
                    printr(statement)
                    print()
                else:
                    formatted_statement = pformat(statement)
                    LOG.debug(f"{index} => {formatted_statement}")

        LOG.debug("Interpreter result:")
        interpreter = Interpreter(self)
        interpreter.interpret(statements)

    def run_from_file(self, file_name: str) -> int:
        LOG.debug(f"Running from file: {file_name}")

        try:
            with open(file_name, "r") as source_file:
                self.run(source_file.read())

        except FileNotFoundError as exc:
            LOG.debug(f"File with {file_name=} is not found: {exc=}")
            return os.EX_NOINPUT
        else:
            if self.had_error:
                return os.EX_DATAERR

            if self.had_runtime_error:
                return 70  # yes

        return os.EX_OK

    def run_from_repl(self) -> int:
        self.repl_is_running = True

        while self.repl_is_running:
            self.repl_main_loop()

        return self.exit_code

    def repl_main_loop(self) -> None:
        try:
            line = input(f"[{self.line_no}] => ")

            if not (line.strip()):
                LOG.debug(f"Ignoring empty input: {line}")
                return

            if not line.endswith(";"):
                line += ";"  # only supported for repl
                print(
                    "Warning: do not forget about semicolon(';') at the end of your statement"
                )

            self.run(line)

        except (KeyboardInterrupt, EOFError) as exc:
            sys.stdout.write("\n")
            LOG.debug(f"Received keyboard interrupt/eof: {exc=}")
            self.repl_is_running = False

        finally:
            self.had_error = False
            self.line_no += 1

    def panic(self, line: int, message: str) -> None:
        self.report(line, "", message)

    def report(self, line: int, where: str, message: str) -> None:
        self.had_error = True
        print(f"[line {line}] Error {where}: {message}", file=sys.stderr)

    def error(self, token: Token, message: str) -> None:
        if token.type != TokenType.EOF:
            self.report(token.line, " at the end", message)
        else:
            self.report(token.line, f" at '{token.lexeme}'", message)

    def runtime_error(self, exc: GusRuntimeError) -> None:
        print(f"{exc.error_message} at [line {exc.token.line}]", file=sys.stderr)
        self.had_runtime_error = True
