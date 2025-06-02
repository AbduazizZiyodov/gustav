import typing as t

from ._logging import log  # noqa: F401
from ._token import Token
from .errors import panic
from .enums import TokenType
from .mappings import RESERVED_KEYWORDS


class Scanner:
    def __init__(self, source: str) -> None:
        self.source: str = source
        self.tokens: list[Token] = []

        self.line: int = 1
        self.start: int = 0
        self.current: int = 0

    def scan_tokens(self) -> list[Token]:
        while not self.is_end:
            self.start = self.current
            self.next_token()

        self.eof()

        return self.tokens

    def next_token(self) -> None:
        char: str = self.move_current()

        match char:
            case "(":
                self.add_token(type=TokenType.LEFT_PAREN)

            case ")":
                self.add_token(type=TokenType.RIGHT_PAREN)

            case "{":
                self.add_token(type=TokenType.LEFT_BRACE)

            case "}":
                self.add_token(type=TokenType.RIGHT_BRACE)

            case ",":
                self.add_token(type=TokenType.COMMA)

            case ".":
                self.add_token(type=TokenType.DOT)

            case "-":
                self.add_token(type=TokenType.MINUS)

            case "+":
                self.add_token(type=TokenType.PLUS)

            case ";":
                self.add_token(type=TokenType.SEMICOLON)

            # operators
            case "*":
                self.add_token(type=TokenType.STAR)

            case "!":
                self.add_token(
                    type=(TokenType.BANG, TokenType.BANG_EQUAL)[self.match_token("=")]
                )

            case "=":
                self.add_token(
                    type=(TokenType.EQUAL, TokenType.EQUAL_EQUAL)[self.match_token("=")]
                )

            case "<":
                self.add_token(
                    type=(TokenType.LESS, TokenType.LESS_EQUAL)[self.match_token("=")]
                )

            case ">":
                self.add_token(
                    type=(TokenType.GREATER, TokenType.GREATER_EQUAL)[
                        self.match_token("=")
                    ]
                )

            case "/":
                if self.match_token("/"):
                    self.comment()
                elif self.match_token("*"):
                    self.multiline_comment()
                else:
                    self.add_token(type=TokenType.SLASH)

            case "\n":
                self.line += 1

            case " " | "\r" | "\t":
                pass

            case '"':
                self.string()

            case _:
                if char.isdigit():
                    self.number()
                elif self.is_alpha_numeric(char):
                    self.identifier()
                else:
                    panic(self.line, "Unexpected character")

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

    def add_token(self, type: TokenType, literal: t.Optional[t.Any] = None) -> None:
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
        type: TokenType = RESERVED_KEYWORDS.get(text) or TokenType.IDENTIFIER

        self.add_token(type=type)

    def number(self) -> None:
        while self.peek().isdigit():
            self.move_current()

        if self.peek() == "." and self.peek_next().isdigit():
            self.move_current()

            while self.peek().isdigit():
                self.move_current()

        part = self.source[self.start : self.current]
        self.add_token(type=TokenType.NUMBER, literal=float(part))

    def multiline_comment(self) -> None:
        while not (self.peek() == "*" and self.peek_next() == "/"):
            if self.peek() == "\n":
                self.line += 1
            self.move_current()

        self.move_current()
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
            panic(self.line, "Unterminated string")
            return

        self.move_current()

        self.add_token(
            type=TokenType.STRING,
            literal=self.source[self.start + 1 : self.current - 1],
        )

    def eof(self) -> None:
        self.tokens.append(Token(TokenType.EOF, "", None, self.line))

    @property
    def current_char(self) -> str:
        return self.source[self.current]

    @property
    def is_end(self) -> bool:
        return self.current >= len(self.source)

    @staticmethod
    def is_alpha_numeric(char: str) -> bool:
        return char.isalnum() or char == "_"
