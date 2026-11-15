import sys

from gustav.enums import TokenType as TT
from gustav.token import Token

had_error: bool = False
had_runtime_error: bool = False


class GusParseError(BaseException):
    pass


class GusRuntimeError(RuntimeError):
    __slots__ = ("error_message", "token")

    def __init__(self, token: Token, message: str) -> None:
        self.token = token
        self.error_message = message


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
