import typing as t  # noqa: F401

from .logging import LOG  # noqa: F401
from .types import Token
from .errors import error
from .enums import TokenType
from .exceptions import GusParseError
from .ast import Expression, Binary, Groupping, Literal, Unary

__all__ = ["Parser"]


class Parser:
    """Recursive descent parser"""

    SYNC_BLOCKS: tuple[TokenType, ...] = (
        TokenType.CLASS,
        TokenType.FOR,
        TokenType.FUN,
        TokenType.IF,
        TokenType.PRINT,
        TokenType.RETURN,
        TokenType.VAR,
        TokenType.WHILE,
    )

    def __init__(self, tokens: list[Token]) -> None:
        self.current: int = 0
        self.tokens: list[Token] = tokens

    def parse(self) -> Expression | None:
        try:
            return self.expression()
        except GusParseError:
            return None

    def expression(self) -> Expression:
        return self.equality()

    def equality(self) -> Expression:
        expr: Expression = self.comparision()

        while self.match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL):
            operator: Token = self.previous()
            right: Expression = self.comparision()
            expr = Binary(expr, operator, right)

        return expr

    def comparision(self) -> Expression:
        expr: Expression = self.term()

        while self.match(
            TokenType.GREATER,
            TokenType.GREATER_EQUAL,
            TokenType.LESS,
            TokenType.LESS_EQUAL,
        ):
            operator: Token = self.previous()
            right: Expression = self.term()
            expr = Binary(expr, operator, right)

        return expr

    def term(self) -> Expression:
        expr = self.factor()

        while self.match(TokenType.MINUS, TokenType.PLUS):
            operator: Token = self.previous()
            right: Expression = self.factor()
            expr = Binary(expr, operator, right)

        return expr

    def factor(self) -> Expression:
        expr = self.unary()

        while self.match(TokenType.SLASH, TokenType.STAR):
            operator: Token = self.previous()
            right: Expression = self.unary()
            expr = Binary(expr, operator, right)

        return expr

    def unary(self) -> Expression:
        if self.match(TokenType.BANG, TokenType.MINUS):
            operator: Token = self.previous()
            right: Expression = self.unary()
            return Unary(operator, right)

        return self.primary()

    def primary(self) -> Expression:
        if self.match(TokenType.FALSE):
            return Literal(False)

        if self.match(TokenType.TRUE):
            return Literal(True)

        if self.match(TokenType.NIL):
            return Literal(None)

        if self.match(TokenType.NUMBER, TokenType.STRING):
            return Literal(self.previous().literal)

        if self.match(TokenType.LEFT_PAREN):
            LOG.debug("inside left paren")
            expr: Expression = self.expression()
            LOG.debug(f"{expr=}, needs right paren")
            self.consume(TokenType.RIGHT_PAREN, "Expect ')' after expression")
            return Groupping(expr)

        raise self.error(self.peek(), "Expect expression")

    def consume(self, type: TokenType, message: str) -> Token:
        if self.check(type):
            return self.advance()

        raise self.error(self.peek(), message)

    def error(self, token: Token, message: str) -> GusParseError:
        error(token, message)
        return GusParseError()

    def synchronize(self) -> None:
        self.advance()

        while not self.is_at_end():
            if self.previous().type == TokenType.SEMICOLON:
                return

            if self.peek().type in self.SYNC_BLOCKS:
                return

        self.advance()

    # Utility methods

    def match(self, *token_types: TokenType) -> bool:
        for token_type in token_types:
            if self.check(token_type):
                self.advance()
                return True
        return False

    def check(self, token_type: TokenType) -> bool:
        if self.is_at_end():
            return False

        return self.peek().type == token_type

    def advance(self) -> Token:
        if not self.is_at_end():
            self.current += 1

        return self.previous()

    def peek(self) -> Token:
        return self.tokens[self.current]

    def previous(self) -> Token:
        return self.tokens[self.current - 1]

    def is_at_end(self) -> bool:
        return self.peek().type == TokenType.EOF
