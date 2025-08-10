import typing as t

from gustav.enums import TokenType as TT

__all__ = "RESERVED_KEYWORDS", "SINGLE_TOKEN_MAPPING"

RESERVED_KEYWORDS: t.Final[dict[str, TT]] = {
    "class": TT.CLASS,
    "super": TT.SUPER,
    "this": TT.THIS,
    "fun": TT.FUN,
    "return": TT.RETURN,
    "yes": TT.TRUE,  # no
    "no": TT.FALSE,  # yes
    "false": TT.FALSE,
    "true": TT.TRUE,
    "nil": TT.NIL,
    "for": TT.FOR,
    "while": TT.WHILE,
    "loop": TT.LOOP,
    "if": TT.IF,
    "else": TT.ELSE,
    "and": TT.AND,
    "or": TT.OR,
    "print": TT.PRINT,
    "var": TT.VAR,
    "\u03bb": TT.LAMBDA,  # Lambda symbol
    "lambda": TT.LAMBDA,
    "break": TT.BREAK,
    "continue": TT.CONTINUE,
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
    "?": TT.QUESTION_MARK,
    ":": TT.COLON,
    "\u03bb": TT.LAMBDA,  # Lambda symbol
}
