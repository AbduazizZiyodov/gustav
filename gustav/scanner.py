import typing as t

from ._logging import log
from ._token import Token
from .errors import error
from .enums import TokenType


KEYWORDS: dict[str, TokenType] = {
    "klass": TokenType.CLASS,
    "super": TokenType.SUPER,
    "this": TokenType.THIS,
    "fn": TokenType.FUN,
    "return": TokenType.RETURN,
    "false": TokenType.FALSE,
    "true": TokenType.TRUE,
    "nil": TokenType.NIL,
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
        self.source = source
        self.tokens: list[Token] = []

        self.line = 1
        self.start = 0
        self.current = 0

    def scan_tokens(self) -> list[Token]:
        while not self.is_end():
            self.start = self.current
            self.scan_token()

        eof_token: Token = Token(TokenType.EOF, "", None, self.line)
        self.tokens.append(eof_token)

        return self.tokens

    def scan_token(self) -> None:
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

            case "*":
                self.add_token(type=TokenType.STAR)
            # operators
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
                if not (self.match_token("/") or self.match_token("*")):
                    self.add_token(type=TokenType.SLASH)
                else:
                    self.comment()

            case "\n":
                self.line += 1

            case " " | "\r" | "\t":
                pass  # ignore whitespaces

            case '"':
                self.string()

            case _:  # default case
                if char.isdigit():
                    self.number()

                elif char.isalpha():
                    self.identifier()
                else:
                    error(self.line, "Unexpected character")

    def add_token(self, type: TokenType, literal: t.Optional[t.Any] = None) -> None:
        lexeme: str | None = None

        if literal:
            lexeme = self.source[self.start : self.current]

        token = Token(type, lexeme, literal, self.line)
        self.tokens.append(token)

    def peek(self) -> str:
        if self.is_end():
            return "\0"
        return self.source[self.current]

    def peek_next(self) -> str:
        if self.current + 1 >= len(self.source):
            return "\0"
        return self.source[self.current + 1]

    def advance(self) -> str:
        char: str = self.source[self.current]
        self.current += 1
        return char

    def is_end(self) -> bool:
        return self.current >= len(self.source)

    def match_token(self, expected: str) -> bool:
        if self.is_end() or self.source[self.current] != expected:
            return False

        self.current += 1
        return True

    def identifier(self) -> None:
        while self.peek().isalpha() or self.peek().isnumeric():
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

        self.add_token(
            type=TokenType.NUMBER,
            literal=float(self.source[self.start : self.current]),
        )

    def comment(self) -> None:
        if self.source[self.current - 1] == "*":
            log.debug("Handling c style comments")
            while not self.is_end() and (
                self.peek() != "*" and self.peek_next() != "/"
            ):
                log.debug(f"Skipping comment(c style): {self.source[self.current]}")
                self.advance()

        else:
            while self.peek() != "\n" and not self.is_end():
                log.debug(f"Skipping comment: {self.source[self.current]}")
                self.advance()

    def string(self) -> None:
        while self.peek() != '"' and not self.is_end():
            if self.peek() == "\n":
                self.line += 1
            self.advance()

        if self.is_end():
            return error(self.line, "Unterminated string")

        self.advance()

        self.add_token(
            type=TokenType.STRING,
            literal=self.source[self.start + 1 : self.current - 1],
        )
