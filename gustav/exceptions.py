from gustav.token import Token

__all__ = (
    "GusParseError",
    "GusRuntimeError",
)


class GusParseError(BaseException):
    pass


class GusRuntimeError(RuntimeError):
    __slots__ = ("error_message", "token")

    def __init__(self, token: Token, message: str) -> None:
        self.token = token
        self.error_message = message
