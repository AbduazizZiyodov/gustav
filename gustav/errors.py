import sys

from .types import Token
from .enums import TokenType

__all__ = ["had_error", "panic", "error", "reset_had_error", "get_had_error"]


had_error: bool = False


def panic(line: int, message: str) -> None:
    report(line, "", message)


def report(line: int, where: str, message: str) -> None:
    global had_error
    had_error = True

    print(f"[line {line}] Error {where}: {message}", file=sys.stderr)


def error(token: Token, message: str) -> None:
    if token.type != TokenType.EOF:
        report(token.line, " at the end", message)
    else:
        report(token.line, f" at '{token.lexeme}'", message)


def reset_had_error() -> None:
    global had_error
    had_error = False


def get_had_error() -> bool:
    global had_error
    return had_error
