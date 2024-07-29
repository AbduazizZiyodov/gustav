###
# Imports
###
import os
import sys
import logging
from enum import StrEnum


###
# LOGGING
###
log = logging.getLogger(__name__)

logging.basicConfig(
    level=logging.DEBUG if os.getenv("DEBUG_LOGGING") else logging.INFO,
    format="%(asctime)s [%(levelname)s]: %(message)s",
    datefmt="%Y-%m-%d %H:%M",
)

had_error = False

###
# ERROR HANDLING
###


def error(line: int, message: str) -> None:
    report(line, "", message)


def report(line: int, where: str, message: str) -> None:
    global had_error
    print(f"[line {line}] Error {where}: {message}", file=sys.stderr)
    had_error = True


###
# ENUMS
###


class TokenType(StrEnum):
    LEFT_PAREN = "LEFT_PAREN"
    RIGHT_PAREN = "RIGHT_PAREN"
    LEFT_BRACE = "LEFT_BRACE"
    RIGHT_BRACE = "RIGHT_BRACE"

    COMMA = "COMMA"
    DOT = "DOT"
    MINUS = "MINUS"
    PLUS = "PLUS"
    SEMICOLON = "SEMICOLON"
    SLASH = "SLASH"
    STAR = "STAR"

    # One or two character tokens.
    BANG = "BANG"
    BANG_EQUAL = "BANG_EQUAL"
    EQUAL = "EQUAL"
    EQUAL_EQUAL = "EQUAL_EQUAL"
    GREATER = "GREATER"
    GREATER_EQUAL = "GREATER_EQUAL"
    LESS = "LESS"
    LESS_EQUAL = "LESS_EQUAL"

    # Literals.
    IDENTIFIER = "IDENTIFIER"
    STRING = "STRING"
    NUMBER = "NUMBER"

    # Keywords.
    AND = "AND"
    CLASS = "CLASS"
    ELSE = "ELSE"
    FALSE = "FALSE"
    FUN = "FUN"
    FOR = "FOR"
    IF = "IF"
    NIL = "NIL"
    OR = "OR"

    PRINT = "PRINT"
    RETURN = "RETURN"
    SUPER = "SUPER"
    THIS = "THIS"
    TRUE = "TRUE"
    VAR = "VAR"
    WHILE = "WHILE"

    EOF = "EOF"


###
# TYPES
###


class Token:
    def __init__(
        self, type: TokenType, lexeme: str, literal: object, line: int
    ) -> None:
        self.type, self.lexeme, self.literal, self.line = (type, lexeme, literal, line)

    def __str__(self) -> str:
        return f"Token({self.type} {self.lexeme} {self.literal})"


###
# Scanner
###


class Scanner:
    def __init__(self, source: str) -> None:
        self.source = source
        self.tokens: list[Token] = []

        self.line = 1
        self.start = 0
        self.current = 0

    def scan_tokens(self) -> list[Token]:
        while not self.is_end:
            self.start = self.current
            log.info(f"{self.start=}")
            self.scan_token()

        eof_token: Token = Token(TokenType.EOF, "", None, self.line)
        self.tokens.append(eof_token)

        return self.tokens

    def scan_token(self):
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
            case _:
                error(self.line, "Fuck you!")

    def add_token(self, **kwargs):
        self.tokens.append(Token(type, None, None, self.line))

    def advance(self):
        self.current += 1
        return self.source[self.current]

    @property
    def is_end(self):
        return self.current >= len(self.source)


###
# Main entrypoint
###


def main():
    log.debug(f"{sys.executable=} {sys.argv[1:]=}")

    if len(sys.argv) > 2:
        print("Usage: python3 -m none [script]")
        return os.EX_USAGE

    if len(sys.argv) == 2:
        return run_file(sys.argv[1])

    return run_prompt()


def execute(source: str) -> int:
    log.info(f"Received {source=}")

    scanner = Scanner(source)
    tokens: list[Token] = scanner.scan_tokens()

    for token in tokens:
        log.info(f"{token=}")


def run_file(file_name: str) -> int:
    log.debug(f"Running from file: {file_name}")

    try:
        with open(file_name, "r") as source_file:
            source: str = source_file.read()
            execute(source)
            return os.EX_DATAERR if (had_error) else os.EX_OK

    except FileNotFoundError as exc:
        log.debug(f"File with {file_name=} is not found: {exc=}")
        return os.EX_NOINPUT


def run_prompt() -> int:
    global had_error

    while True:
        try:
            line = input("=> ")
            execute(line)
            had_error = False
        except KeyboardInterrupt as exc:
            log.debug(f"Received keyboard interrupt(sigint): {exc=}")
            return os.EX_OK


if __name__ == "__main__":
    raise SystemExit(main())
