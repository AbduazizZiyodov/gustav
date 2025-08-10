from enum import StrEnum, auto

__all__ = "FunctionType", "ClassType", "TokenType"


class FunctionType(StrEnum):
    NONE = auto()
    FUNCTION = auto()
    METHOD = auto()
    INITIALIZER = auto()
    LAMBDA = auto()


class ClassType(StrEnum):
    NONE = auto()
    CLASS = auto()
    SUBCLASS = auto()


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
    PLUS_PLUS = auto()  # NOTE(abduazizziyodov): not now
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
    LAMBDA = auto()
    FOR = auto()
    IF = auto()
    NIL = auto()
    OR = auto()

    BREAK = auto()
    CONTINUE = auto()

    PRINT = auto()

    RETURN = auto()
    SUPER = auto()
    THIS = auto()
    TRUE = auto()
    VAR = auto()
    WHILE = auto()

    LOOP = auto()

    PIPE = auto()
    CARET = auto()

    COLON = auto()
    QUESTION_MARK = auto()

    EOF = auto()
