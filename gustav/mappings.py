import typing as t

from .enums import TokenType


KEYWORDS: t.Final[dict[str, TokenType]] = {
    "class": TokenType.CLASS,
    "super": TokenType.SUPER,
    "this": TokenType.THIS,
    "fun": TokenType.FUN,
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
    "print": TokenType.PRINT,
    "var": TokenType.VAR,
}
