class Token:
    def __init__(
        self, type: str, lexeme: str | None, literal: object, line: int
    ) -> None:
        self.type, self.lexeme, self.literal, self.line = (type, lexeme, literal, line)

    def __repr__(self) -> str:
        return f"Token({self.type} {self.lexeme} {self.literal})"
