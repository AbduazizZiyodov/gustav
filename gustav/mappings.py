import typing as t

from gustav.enums import TokenType as TT

RESERVED_KEYWORDS: t.Final = {
    # "fun": TT.FUN,
    # "return": TT.RETURN,
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
    "break": TT.BREAK,
    "continue": TT.CONTINUE,
}


SINGLE_TOKEN_MAPPING: t.Final = {
    "(": TT.LEFT_PAREN,
    ")": TT.RIGHT_PAREN,
    "{": TT.LEFT_BRACE,
    "}": TT.RIGHT_BRACE,
    ",": TT.COMMA,
    ".": TT.DOT,
    "-": TT.MINUS,
    ";": TT.SEMICOLON,
    "*": TT.STAR,
    "?": TT.QUESTION_MARK,
    ":": TT.COLON,
}
