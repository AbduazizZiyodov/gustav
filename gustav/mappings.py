import typing as t

from .token import TokenType

__all__ = ["RESERVED_KEYWORDS", "SINGLE_TOKEN_MAPPING"]

RESERVED_KEYWORDS: t.Final[dict[str, TokenType]] = {
    "class": TokenType.CLASS,
    "super": TokenType.SUPER,
    "this": TokenType.THIS,
    "fun": TokenType.FUN,
    "return": TokenType.RETURN,
    "yes": TokenType.TRUE,  # yes
    "no": TokenType.FALSE,  # no
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


SINGLE_TOKEN_MAPPING: t.Final[dict[str, TokenType]] = {
    "(": TokenType.LEFT_PAREN,
    ")": TokenType.RIGHT_PAREN,
    "{": TokenType.LEFT_BRACE,
    "}": TokenType.RIGHT_BRACE,
    ",": TokenType.COMMA,
    ".": TokenType.DOT,
    "-": TokenType.MINUS,
    ";": TokenType.SEMICOLON,
    "*": TokenType.STAR,
    "^": TokenType.CARET,
}
