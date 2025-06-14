import typing as t

from .token import TokenType as TT

__all__ = "RESERVED_KEYWORDS", "SINGLE_TOKEN_MAPPING"

RESERVED_KEYWORDS: t.Final[dict[str, TT]] = {
    "class": TT.CLASS,
    "super": TT.SUPER,
    "this": TT.THIS,
    "fun": TT.FUN,
    "return": TT.RETURN,
    "yes": TT.TRUE,  # yes
    "no": TT.FALSE,  # no
    "false": TT.FALSE,
    "true": TT.TRUE,
    "nil": TT.NIL,
    "for": TT.FOR,
    "while": TT.WHILE,
    "if": TT.IF,
    "else": TT.ELSE,
    "and": TT.AND,
    "or": TT.OR,
    "print": TT.PRINT,
    "var": TT.VAR,
}


SINGLE_TOKEN_MAPPING: t.Final[dict[str, TT]] = {
    "(": TT.LEFT_PAREN,
    ")": TT.RIGHT_PAREN,
    "{": TT.LEFT_BRACE,
    "}": TT.RIGHT_BRACE,
    ",": TT.COMMA,
    ".": TT.DOT,
    "-": TT.MINUS,
    ";": TT.SEMICOLON,
    "*": TT.STAR,
    "^": TT.CARET,
}
