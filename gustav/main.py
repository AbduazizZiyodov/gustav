import os
import sys
import typing as t  # noqa: F401

from .types import Token
from .logging import LOG
from .parser import Parser
from .scanner import Scanner
from .ast_printer import AstPrinter
from .errors import has_error, reset_error

__all__ = ["Gustav"]


class Gustav:
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

        except FileNotFoundError as exc:
            LOG.debug(f"File with {file_name=} is not found: {exc=}")
            return os.EX_NOINPUT
        else:
            return os.EX_DATAERR if has_error() else os.EX_OK

    def run_from_repl(self) -> int:
        line_no: int = 1
        is_running: bool = True

        while is_running:
            try:
                line = input(f"[{line_no}] => ")

                if not (line.strip()):
                    LOG.debug(f"Ignoring empty lines: {line}")
                    continue

                self.exec_source(line)

                reset_error()

                line_no += 1

            except (KeyboardInterrupt, EOFError) as exc:
                sys.stdout.write("\n")
                LOG.debug(f"Received keyboard interrupt/eof: {exc=}")
                return os.EX_OK

        return os.EX_OK

    def exec_source(self, source: str) -> None:
        source += "\n"
        LOG.debug(
            f"Received source's length is {len(source)} chars & contents of source:\n{source}"
        )

        scanner = Scanner(source)
        tokens: list[Token] = scanner.get_tokens()

        LOG.debug(f"Scanning completed, {len(tokens)} tokens scanned:")

        for index, token in enumerate(tokens):
            print(f"{index} => {token}")

        parser: Parser = Parser(tokens)
        expression = parser.parse()

        LOG.debug("Parsing completed")

        if has_error():
            LOG.debug("Parsing failed, returning ...")
            return

        LOG.debug(f"Resulting expression after parsing:\n{expression=}")

        printer = AstPrinter()
        representation: str = printer.get_string_repr(expression)

        LOG.debug(f"Result of ast printer:\n{representation}")
