from .token import Token


class GusParseError(BaseException):
    pass


class GusRuntimeError(RuntimeError):
    token: Token
    error_message: str

    def __init__(self, token: Token, message: str) -> None:
        self.token = token
        self.error_message = message
