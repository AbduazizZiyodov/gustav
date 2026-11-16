import os
import sys
import typing as t  # noqa: F401

from rich import print as printr

from gustav.ast.statement import Statement
from gustav.compiler import Compiler
from gustav.enums import TokenType as TT
from gustav.errors import had_error, had_runtime_error
from gustav.logging import DEBUG, LOG
from gustav.parser import Parser
from gustav.scanner import Scanner
from gustav.token import Token


def main() -> int:
    if len(sys.argv) != 2:  # pragma: no cover
        sys.stdout.write(f"Usage: {sys.executable} -m gustav [script]")
        return os.EX_USAGE

    file_name = sys.argv[1]

    source: str = ""

    try:
        with open(file_name, "r", encoding="utf-8") as file:
            source = file.read()

    except FileNotFoundError:  # pragma: no cover
        sys.stderr.write(f"Couldn't find file with {file_name=}")
        return os.EX_NOINPUT

    if not source.endswith("\n"):
        source += "\n"

    tokens: list[Token] = Scanner(source).get_tokens()

    if had_error:
        LOG.debug("scanning failed, exiting")
        return os.EX_DATAERR

    if len(tokens) == 1 and tokens[0].type == TT.EOF:
        LOG.debug(f"no tokens received {tokens=}")
        return os.EX_DATAERR

    LOG.debug(f"scanning finished, got {len(tokens)} tokens:\n")

    if DEBUG:
        current_line = tokens[0].line

        for token in tokens:
            if token.line != current_line:
                current_line = token.line
                print()

            printr(
                f"{token.type}"
                + (f"({token.lexeme})" if token.type == TT.NUMBER else ""),
                end=" ",
            )

        print("\n")

    statements: list[Statement] = Parser(tokens).parse()

    LOG.debug("parsing finished:")

    if DEBUG:
        for index, statement in enumerate(statements):
            printr(f"statement {index}:")
            printr(statement)
            sys.stdout.write("\n")

    LOG.debug("compiling ...")
    Compiler(statements).do()

    if had_runtime_error:
        return 70

    return os.EX_OK
