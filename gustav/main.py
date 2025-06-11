import os
import sys
import typing as t  # noqa: F401

from pprint import pformat

from .types import Token
from .ast import Statement
from .parser import Parser
from .scanner import Scanner
from .logging import LOG, DEBUG
from .interpreter import Interpreter
from .exceptions import GusParseError
from .errors import has_error, reset_error, has_runtime_error

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

    def main(self) -> int:
        if len(sys.argv) > 2:
            print(f"Usage: {sys.executable} -m gustav [script]")
            return os.EX_USAGE

        if len(sys.argv) == 2:
            return self.run_from_file(sys.argv[1])

        return self.run_from_repl()

    def run_from_file(self, file_name: str) -> int:
        LOG.debug(f"Running from file: {file_name}")

        try:
            with open(file_name, "r") as source_file:
                self.exec_source(source_file.read())

        except GusParseError as exc:
            print(f"ParseError occurred: {exc=}")

        except FileNotFoundError as exc:
            LOG.debug(f"File with {file_name=} is not found: {exc=}")
            return os.EX_NOINPUT
        else:
            if has_error():
                return os.EX_DATAERR
            if has_runtime_error():
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

            self.exec_source(line)

        except GusParseError as exc:
            print(f"ParseError occurred: {exc=}")

        except (KeyboardInterrupt, EOFError) as exc:
            sys.stdout.write("\n")
            LOG.debug(f"Received keyboard interrupt/eof: {exc=}")
            self.repl_is_running = False

        finally:
            reset_error()
            self.line_no += 1

    def exec_source(self, source: str) -> None:
        global DEBUG, rich_installed

        source += "\n"
        LOG.debug(
            f"Received source's length is {len(source)} chars & contents of source:\n{source}"
        )

        scanner = Scanner(source)
        tokens: list[Token] = scanner.get_tokens()

        if has_error():
            LOG.debug("Scanning failed, returning ...")
            return

        LOG.debug(f"Scanning completed, {len(tokens)} tokens scanned:")

        for index, token in enumerate(tokens):
            LOG.debug(f"{index} => {token}")

        parser: Parser = Parser(tokens)

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
        interpreter = Interpreter()
        interpreter.interpret(statements)
