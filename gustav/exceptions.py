import typing as t

from gustav.token import Token

__all__ = "GusParseError", "GusRuntimeError"


class GusParseError(BaseException):
    pass


class GusRuntimeError(RuntimeError):
    token: Token
    error_message: str

    def __init__(self, token: Token, message: str) -> None:
        self.token = token
        self.error_message = message


class GusReturn(RuntimeError):
    __slots__ = ("value",)

    def __init__(self, value: t.Any, *args: t.Any) -> None:
        super().__init__(*args)
        self.value = value
