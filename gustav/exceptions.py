import typing as t

from gustav.token import Token

__all__ = "GusParseError", "GusRuntimeError", "GusReturn"


class GusParseError(BaseException):
    pass


class GusRuntimeError(RuntimeError):
    __slots__ = ("token", "error_message")

    def __init__(self, token: Token, message: str) -> None:
        self.token = token
        self.error_message = message


class GusReturn(RuntimeError):
    """Used for unwinding, to get desired position in call stack.
    Not a good practice, however enough for tree-walk interpreter.
    """

    __slots__ = ("value",)

    def __init__(self, value: t.Any, *args: t.Any) -> None:
        self.value = value


class GusStopIteration(RuntimeError):
    """Used in break statement"""

    pass


class GusContinueIteration(RuntimeError):
    """Used in continue statement"""

    pass
