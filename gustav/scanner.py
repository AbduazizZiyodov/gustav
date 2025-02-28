import typing as t

from ._logging import log
from ._token import Token
from .errors import panic
from .enums import TokenType


KEYWORDS: t.Final[dict[str, TokenType]] = {
    "class": TokenType.CLASS,
    "super": TokenType.SUPER,
    "@": TokenType.THIS,
    "fn": TokenType.FUNCTION,
    "ret": TokenType.RETURN,
    "false": TokenType.FALSE,
    "true": TokenType.TRUE,
    "null": TokenType.NIL,
    "for": TokenType.FOR,
    "while": TokenType.WHILE,
    "if": TokenType.IF,
    "else": TokenType.ELSE,
    "and": TokenType.AND,
    "or": TokenType.OR,
    "log": TokenType.PRINT,
    "var": TokenType.VAR,
}


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
        char = self.advance()

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
                if not self.match_token("/"):
                    self.add_token(type=TokenType.SLASH)
                else:
                    self.comment()

            case "\n":
                self.line += 1

            case " " | "\r" | "\t":
                pass

            case '"':
                self.string()

            case _:
                if char.isdigit():
                    self.number()

                elif char.isalpha() or char == "_":
                    self.identifier()

                else:
                    panic(self.line, "Unexpected character")

    def peek(self) -> str:
        return "\0" if self.is_end else self.source[self.current]

    def peek_next(self) -> str:
        if self.current + 1 >= len(self.source):
            return "\0"
        return self.source[self.current + 1]

    def advance(self) -> str:
        char: str = self.source[self.current]
        self.current += 1
        return char

    @property
    def is_end(self) -> bool:
        return self.current >= len(self.source)

    def add_token(self, type: TokenType, literal: t.Optional[t.Any] = None) -> None:
        text = self.source[self.start : self.current]

        token = Token(type, text, literal, self.line)
        self.tokens.append(token)

    def match_token(self, expected: str) -> bool:
        if self.is_end:
            return False

        if self.source[self.current] != expected:
            return False

        self.current += 1
        return True

    def identifier(self) -> None:
        while self.peek().isalnum():
            self.advance()

        text: str = self.source[self.start : self.current]
        type: TokenType = KEYWORDS.get(text) or TokenType.IDENTIFIER

        self.add_token(type=type)

    def number(self) -> None:
        while self.peek().isdigit():
            self.advance()

        if self.peek() == "." and self.peek_next().isdigit():
            self.advance()

            while self.peek().isdigit():
                self.advance()

        part = self.source[self.start : self.current]
        self.add_token(type=TokenType.NUMBER, literal=float(part))

    def comment(self) -> None:
        while self.peek() != "\n" and not self.is_end:
            log.debug(f"Skipping comment: {self.source[self.current]}")
            self.advance()

    def string(self) -> None:
        while self.peek() != '"' and not self.is_end:
            if self.peek() == "\n":
                self.line += 1
            self.advance()

        if self.is_end:
            panic(self.line, "Unterminated string")
            return

        self.advance()

        self.add_token(
            type=TokenType.STRING,
            literal=self.source[self.start + 1 : self.current - 1],
        )

    def eof(self) -> None:
        eof_token: Token = Token(TokenType.EOF, "", None, self.line)
        self.tokens.append(eof_token)
