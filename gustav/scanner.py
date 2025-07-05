import typing as t

from gustav.logging import LOG  # noqa: F401
from gustav import gustav
from gustav.token import Token
from gustav.types import TokenType as TT
from gustav.mappings import RESERVED_KEYWORDS, SINGLE_TOKEN_MAPPING

__all__ = ("Scanner",)


class Scanner:
    def __init__(self, source: str) -> None:
        self.source: str = source
        self.tokens: list[Token] = []

        self.line: int = 1
        self.start: int = 0
        self.current: int = 0

    def get_tokens(self) -> list[Token]:
        while not self.is_end:
            self.start = self.current
            self.next_token()

        self.eof()

        return self.tokens

    def next_token(self) -> None:
        char: str = self.move_current()

        if char in SINGLE_TOKEN_MAPPING:
            self.add_token(type=SINGLE_TOKEN_MAPPING[char])
            return

        match char:
            case "!":
                self.add_token(type=(TT.BANG, TT.BANG_EQUAL)[self.match_token("=")])

            case "=":
                self.add_token(type=(TT.EQUAL, TT.EQUAL_EQUAL)[self.match_token("=")])

            case "<":
                self.add_token(type=(TT.LESS, TT.LESS_EQUAL)[self.match_token("=")])

            case ">":
                self.add_token(
                    type=(TT.GREATER, TT.GREATER_EQUAL)[self.match_token("=")]
                )

            case "+":
                if self.match_token("+"):
                    self.add_token(type=TT.PLUS_PLUS)
                else:
                    self.add_token(type=TT.PLUS)

            case "/":
                if self.match_token("/"):
                    self.comment()

                elif self.match_token("*"):
                    self.multiline_comment()

                else:
                    self.add_token(type=TT.SLASH)

            case "\n":
                self.line += 1

            case " " | "\r" | "\t" | "\f" | "\v":
                pass

            case '"':
                self.string()

            case _:
                if char == "|" and self.match_token(">"):
                    self.add_token(TT.PIPE)

                elif char.isdigit():
                    self.number()

                elif self.is_alpha_numeric(char):
                    self.identifier()

                else:
                    gustav.panic(self.line, "Unexpected character")

    def peek(self) -> str:
        return "\0" if self.is_end else self.current_char

    def peek_next(self) -> str:
        if self.current + 1 >= len(self.source):
            return "\0"

        return self.source[self.current + 1]

    def move_current(self) -> str:
        char: str = self.current_char
        self.current += 1

        return char

    def add_token(self, type: TT, literal: t.Optional[t.Any] = None) -> None:
        text = self.source[self.start : self.current]

        new_token = Token(type, text, literal, self.line)
        self.tokens.append(new_token)

    def match_token(self, expected: str) -> bool:
        if self.is_end:
            return False

        if self.current_char != expected:
            return False

        self.current += 1

        return True

    def identifier(self) -> None:
        while self.is_alpha_numeric(self.peek()):
            self.move_current()

        text: str = self.source[self.start : self.current]
        type: TT = RESERVED_KEYWORDS.get(text) or TT.IDENTIFIER

        self.add_token(type=type)

    def number(self) -> None:
        while self.peek().isdigit():
            self.move_current()

        if self.peek() == "." and self.peek_next().isdigit():
            self.move_current()

            while self.peek().isdigit():
                self.move_current()

        part = self.source[self.start : self.current]
        self.add_token(type=TT.NUMBER, literal=float(part))

    def multiline_comment(self) -> None:
        if self.peek() == "\0":
            gustav.panic(self.line, "Multiline comment is not terminated.")
            return

        while True:
            # opening, we've already consumed /*,
            # if we encounter one again, it means nesting.
            if self.peek() == "/" and self.peek_next() == "*":
                gustav.panic(self.line, "Nesting multiline comments are not allowed.")
                return

            # looking for terminating points
            if self.peek() == "*" and self.peek_next() == "/":
                self.move_current(), self.move_current()
                break

            self.move_current()

    def comment(self) -> None:
        while self.peek() != "\n" and not self.is_end:
            self.move_current()

    def string(self) -> None:
        while self.peek() != '"' and not self.is_end:
            if self.peek() == "\n":
                self.line += 1

            self.move_current()

        if self.is_end:
            gustav.panic(self.line, "Unterminated string")
            return

        self.move_current()

        self.add_token(
            type=TT.STRING,
            literal=self.source[self.start + 1 : self.current - 1],
        )

    def eof(self) -> None:
        self.tokens.append(Token(TT.EOF, "", None, self.line))

    @property
    def current_char(self) -> str:
        return self.source[self.current]

    @property
    def is_end(self) -> bool:
        return self.current >= len(self.source)

    @staticmethod
    def is_alpha_numeric(char: str) -> bool:
        return char.isalnum() or char == "_"
