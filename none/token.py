from none.enums import TokenType


class Token:
    def __init__(
        self, type: TokenType, lexeme: str, literal: object, line: int
    ) -> None:
        self.type, self.lexeme, self.literal, self.line = (type, lexeme, literal, line)

    def __str__(self) -> str:
        return f"Token({self.type} {self.lexeme} {self.literal})"
