import typing as t  # noqa: F401

from .logging import LOG  # noqa: F401
from .types import Token
from .enums import TokenType
from .exceptions import GusParseError
from .ast import (
    Expression,
    Binary,
    Groupping,
    Literal,
    Variable,
    Unary,
    Statement,
    Expr,
    Print,
    Var,
    Assign,
    Block,
)

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

    def __init__(self, tokens: list[Token], gustav_instance: t.Any) -> None:
        self.current: int = 0
        self.tokens: list[Token] = tokens
        self.gustav = gustav_instance

    def parse(self) -> list[Statement]:
        statements: list[Statement] = list()

        while not self.is_at_end():
            if declaration := self.declaration():
                statements.append(declaration)

        return statements

    def declaration(self) -> Statement | Var | None:
        try:
            if self.match(TokenType.VAR):
                return self.var_declaration()

            return self.statement()

        except GusParseError as exc:
            LOG.debug(f"Syncing ... {exc=}")
            self.synchronize()

            return None

    def var_declaration(self) -> Var:
        name: Token = self.consume(TokenType.IDENTIFIER, "Expect variable name.")
        initializer: Expression | None = None

        if self.match(TokenType.EQUAL):
            initializer = self.expression()

        self.consume(TokenType.SEMICOLON, "Expect ';' after variables declaration.")

        return Var(name, initializer)

    def statement(self) -> Statement:
        if self.match(TokenType.PRINT):
            return self.print_statement()

        if self.match(TokenType.LEFT_BRACE):
            return Block(self.block())

        return self.expression_statement()

    def block(self) -> list[Statement]:
        statements: list[Statement] = list()

        while not self.check(TokenType.RIGHT_BRACE) and not self.is_at_end():
            if declaration := self.declaration():
                statements.append(declaration)

        self.consume(TokenType.RIGHT_BRACE, "Expect '}' after block.")
        return statements

    def print_statement(self) -> Print:
        value: Expression = self.expression()
        self.consume(TokenType.SEMICOLON, "Expect ';' after value")

        return Print(value)

    def expression_statement(self) -> Expr:
        expression = self.expression()
        self.consume(TokenType.SEMICOLON, "Expect ';' after value")

        return Expr(expression)

    def expression(self) -> Expression:
        return self.assignment()

    def assignment(self) -> Expression:
        expr: Expression = self.equality()

        if self.match(TokenType.EQUAL):
            equals: Token = self.previous()
            value: Expression = self.assignment()

            if isinstance(expr, Variable):
                name: Token = expr.name
                return Assign(name, value)

            self.error(equals, "Invalid assignment target")

        return expr

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

        while self.match(
            TokenType.MINUS, TokenType.PLUS, TokenType.PLUS_PLUS, TokenType.CARET
        ):
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

        if self.match(TokenType.IDENTIFIER):
            return Variable(self.previous())

        if self.match(TokenType.LEFT_PAREN):
            expr: Expression = self.expression()
            self.consume(TokenType.RIGHT_PAREN, "Expect ')' after expression")
            return Groupping(expr)

        raise self.error(self.peek(), "Expect expression")

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

    def consume(self, type: TokenType, message: str) -> Token:
        if self.check(type):
            return self.advance()

        raise self.error(self.peek(), message)

    def error(self, token: Token, message: str) -> GusParseError:
        self.gustav.error(token, message)
        return GusParseError()

    def peek(self) -> Token:
        return self.tokens[self.current]

    def previous(self) -> Token:
        return self.tokens[self.current - 1]

    def is_at_end(self) -> bool:
        return self.peek().type == TokenType.EOF
