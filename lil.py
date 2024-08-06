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

DEBUG: bool = os.getenv("DEBUG", "").lower() in ["y", "true", "yes"]

logging.basicConfig(
    level=logging.DEBUG if DEBUG else logging.INFO,
    format="%(asctime)s %(levelname)s: %(message)s",
    datefmt="%Y-%m-%d/%H:%M",
)

try:
    import colorama
    from colorama import Fore, Style

    colorama.init()
    COLORED_OUTPUT: bool = True
except ImportError:
    COLORED_OUTPUT: bool = False
    log.debug("Coloroma not found")


had_error = False

###
# ERROR HANDLING
###


def error(
    line: int,
    message: str,
) -> None:
    report(line, "", message)


def report(line: int, where: str, message: str) -> None:
    global had_error
    if not COLORED_OUTPUT:
        print(f"[line {line}] Error {where}: {message}", file=sys.stderr)
    else:
        print(
            f"{Fore.BLUE}[line {line}]{Style.RESET_ALL} {Fore.RED}Error{Style.RESET_ALL} {where}: {message}",
            file=sys.stderr,
        )

    had_error = True


###
# ENUMS & keywords
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


KEYWORDS: dict[str, str] = {
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

###
# TYPES
###


class Token:
    def __init__(
        self, type: TokenType, lexeme: str, literal: object, line: int
    ) -> None:
        self.type, self.lexeme, self.literal, self.line = (type, lexeme, literal, line)

    def __repr__(self) -> str:
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
                self.comment_or_slash()

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

    def identifier(self):
        while self.peek().isalpha() or self.peek().isnumeric():
            self.advance()

        text = self.source[self.start : self.current]
        type = KEYWORDS.get(text)

        if type is None:
            type = TokenType.IDENTIFIER

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

    def comment_or_slash(self) -> None:
        if self.match_token("/"):
            while self.peek() != "\n" and not self.is_end():
                self.advance()
        else:
            self.add_token(type=TokenType.SLASH)

    def string(self) -> None:
        while self.peek() != '"' and not self.is_end():
            if self.peek() == "\n":
                self.line += 1
            self.advance()

        if self.is_end():
            return error(self.line, "Unterminated string")

        self.advance()
        string_value: str = self.source[self.start + 1 : self.current - 1]
        self.add_token(type=TokenType.STRING, literal=string_value)

    def add_token(self, **kwargs) -> None:
        lexeme = None
        type = kwargs.get("type")

        if (literal := kwargs.get("literal")) is not None:
            lexeme = self.source[self.start : self.current]

        self.tokens.append(
            Token(
                type,
                lexeme,
                literal,
                self.line,
            )
        )

    def peek(self) -> str:
        if self.is_end():
            return "\0"
        return self.source[self.current]

    def peek_next(self):
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


###
# Main entrypoint
###


def main():
    log.debug(f"{sys.executable=} {sys.argv[1:]=}")

    if len(sys.argv) > 2:
        print("Usage: python3 -m lil.py [script]")
        return os.EX_USAGE

    if len(sys.argv) == 2:
        return run_file(sys.argv[1])

    return run_prompt()


def execute(source: str) -> int:
    log.debug(f"Received: {len(source)=} \n{source=}")

    scanner = Scanner(source)
    tokens: list[Token] = scanner.scan_tokens()

    for token in tokens:
        log.debug(f"{token=}")


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
        except (KeyboardInterrupt, EOFError) as exc:
            log.debug(f"Received keyboard interrupt/eof: {exc=}")
            return os.EX_OK


if __name__ == "__main__":
    raise SystemExit(main())
