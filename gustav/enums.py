from enum import StrEnum, auto

__all__ = "FunctionType", "TokenType", "VariableState"


class FunctionType(StrEnum):
    NONE = auto()
    FUNCTION = auto()
    METHOD = auto()
    INITIALIZER = auto()
    LAMBDA = auto()


class VariableState(StrEnum):
    USED = auto()
    DECLARED = auto()
    DEFINED = auto()


class TokenType(StrEnum):
    LEFT_PAREN = auto()
    RIGHT_PAREN = auto()
    LEFT_BRACE = auto()
    RIGHT_BRACE = auto()

    COMMA = auto()
    DOT = auto()
    MINUS = auto()
    PLUS = auto()
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
    ELSE = auto()
    FALSE = auto()
    FOR = auto()
    IF = auto()
    NIL = auto()
    OR = auto()

    BREAK = auto()
    CONTINUE = auto()

    PRINT = auto()

    TRUE = auto()
    VAR = auto()
    WHILE = auto()

    LOOP = auto()

    COLON = auto()
    QUESTION_MARK = auto()

    EOF = auto()
