import typing as t
from enum import StrEnum, auto
from dataclasses import dataclass

__all__ = "Token", "TokenType"


class TokenType(StrEnum):
    LEFT_PAREN = auto()
    RIGHT_PAREN = auto()
    LEFT_BRACE = auto()
    RIGHT_BRACE = auto()

    COMMA = auto()
    DOT = auto()
    MINUS = auto()
    PLUS = auto()
    PLUS_PLUS = auto()
    SEMICOLON = auto()
    SLASH = auto()
    STAR = auto()

    BANG = auto()
    BANG_EQUAL = auto()
    EQUAL = auto()
    EQUAL_EQUAL = auto()
    GREATER = auto()
    GREATER_EQUAL = auto()
    LESS = auto()
    LESS_EQUAL = auto()

    IDENTIFIER = auto()
    STRING = auto()
    NUMBER = auto()

    AND = auto()
    CLASS = auto()
    ELSE = auto()
    FALSE = auto()
    FUN = auto()
    FOR = auto()
    IF = auto()
    NIL = auto()
    OR = auto()

    PRINT = auto()
    RETURN = auto()
    SUPER = auto()
    THIS = auto()
    TRUE = auto()
    VAR = auto()
    WHILE = auto()

    PIPE = auto()
    CARET = auto()

    EOF = auto()


@dataclass(slots=True, frozen=True)
class Token:
    type: TokenType
    lexeme: str
    literal: t.Any
    line: int

    def __str__(self) -> str:
        return f"{self.type} {self.lexeme} {self.literal}"

    def __repr__(self) -> str:
        return f"Token(type={self.type} lexeme='{self.lexeme}' literal={self.literal})"
