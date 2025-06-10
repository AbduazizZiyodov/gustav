import sys

from .types import Token
from .enums import TokenType
from .exceptions import GusRuntimeError

had_error: bool = False
had_runtime_error: bool = False


def panic(line: int, message: str) -> None:
    report(line, "", message)


def report(line: int, where: str, message: str) -> None:
    set_error()
    print(f"[line {line}] Error {where}: {message}", file=sys.stderr)


def error(token: Token, message: str) -> None:
    if token.type != TokenType.EOF:
        report(token.line, " at the end", message)
    else:
        report(token.line, f" at '{token.lexeme}'", message)


def set_error() -> None:
    global had_error
    had_error = True


def reset_error() -> None:
    global had_error
    had_error = False


def has_error() -> bool:  # yes
    global had_error
    return had_error


def set_runtime_error() -> None:
    global had_runtime_error
    had_runtime_error = True


def reset_runtime_error() -> None:
    global had_runtime_error
    had_runtime_error = False


def has_runtime_error() -> bool:  # yes
    global had_runtime_error
    return had_runtime_error


def runtime_error(exc: GusRuntimeError) -> None:
    print(f"{exc.error_message} at [line {exc.token.line}]", file=sys.stderr)
    set_runtime_error()
