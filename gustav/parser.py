import typing as t  # noqa: F401

from .logging import LOG  # noqa: F401
from gustav import gustav
from .exceptions import GusParseError
from .ast import (
    Expression,
    Binary,
    Groupping,
    Literal,
    Variable,
    Logical,
    Unary,
    Statement,
    Expr,
    Print,
    Var,
    While,
    Assign,
    Block,
    If,
)
from .token import Token, TokenType as TT

__all__ = ("Parser",)


class Parser:
    """Recursive descent parser"""

    SYNC_BLOCKS: tuple[TT, ...] = (
        TT.CLASS,
        TT.FOR,
        TT.FUN,
        TT.IF,
        TT.PRINT,
        TT.RETURN,
        TT.VAR,
        TT.WHILE,
    )

    def __init__(self, tokens: list[Token]) -> None:
        self.current: int = 0
        self.tokens: list[Token] = tokens

    def parse(self) -> list[Statement]:
        statements: list[Statement] = list()

        while not self.is_at_end():
            if declaration := self.declaration():
                statements.append(declaration)

        return statements

    def declaration(self) -> Statement | Var | None:
        try:
            if self.match(TT.VAR):
                return self.var_declaration()

            return self.statement()

        except GusParseError as exc:
            LOG.debug(f"Syncing ... {exc=}")
            self.synchronize()

            return None

    def var_declaration(self) -> Var:
        name: Token = self.consume(TT.IDENTIFIER, "Expect variable name.")
        initializer: Expression | None = None

        if self.match(TT.EQUAL):
            initializer = self.expression()

        self.consume(TT.SEMICOLON, "Expect ';' after variables declaration.")

        return Var(name, initializer)

    def statement(self) -> Statement:
        if self.match(TT.PRINT):
            return self.print_statement()

        if self.match(TT.LEFT_BRACE):
            return Block(self.block())

        if self.match(TT.IF):
            return self.if_statement()

        if self.match(TT.WHILE):
            return self.while_statement()

        if self.match(TT.FOR):
            return self.for_statement()

        return self.expression_statement()

    def for_statement(self) -> Statement:
        """desugaring for loops into while loops"""
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'for'.")

        initializer: Var | Expr | None

        if self.match(TT.SEMICOLON):
            initializer = None
        elif self.match(TT.VAR):
            initializer = self.var_declaration()
        else:
            initializer = self.expression_statement()

        condition: Expression | None = None

        if not self.check(TT.SEMICOLON):
            condition = self.expression()

        self.consume(TT.SEMICOLON, "Expect ';' after loop condition.")

        increment: Expression | None = None

        if not self.check(TT.RIGHT_PAREN):
            increment = self.expression()

        self.consume(TT.RIGHT_PAREN, "Expect ')' after for clauses.")

        body: Statement = self.statement()

        if increment is not None:
            body = Block([body, Expr(increment)])

        if condition is None:
            condition = Literal(True)

        body = While(condition, body)

        if initializer is not None:
            body = Block([initializer, body])

        return body

    def while_statement(self) -> While:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'while'.")
        condition: Expression = self.expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'.")

        body: Statement = self.statement()

        return While(condition, body)

    def if_statement(self) -> If:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'if'.")
        condition: Expression = self.expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'.")

        then_branch: Statement = self.statement()
        else_branch: Statement | None = None

        if self.match(TT.ELSE):
            else_branch = self.statement()

        return If(condition, then_branch, else_branch)

    def block(self) -> list[Statement]:
        statements: list[Statement] = list()

        while not self.check(TT.RIGHT_BRACE) and not self.is_at_end():
            if declaration := self.declaration():
                statements.append(declaration)

        self.consume(TT.RIGHT_BRACE, "Expect '}' after block.")
        return statements

    def print_statement(self) -> Print:
        value: Expression = self.expression()
        self.consume(TT.SEMICOLON, "Expect ';' after value")

        return Print(value)

    def expression_statement(self) -> Expr:
        expression = self.expression()
        self.consume(TT.SEMICOLON, "Expect ';' after value")

        return Expr(expression)

    def expression(self) -> Expression:
        return self.assignment()

    def assignment(self) -> Expression:
        expr: Expression = self.Or()

        if self.match(TT.EQUAL):
            equals: Token = self.previous()
            value: Expression = self.assignment()

            if isinstance(expr, Variable):
                name: Token = expr.name
                return Assign(name, value)

            self.error(equals, "Invalid assignment target")

        return expr

    def Or(self) -> Expression:  # yes
        expr: Expression = self.And()

        while self.match(TT.OR):
            operator: Token = self.previous()
            right: Expression = self.And()
            expr = Logical(expr, operator, right)

        return expr

    def And(self) -> Expression:  # yes
        expr: Expression = self.equality()

        while self.match(TT.AND):
            operator: Token = self.previous()
            right: Expression = self.equality()
            expr = Logical(expr, operator, right)

        return expr

    def equality(self) -> Expression:
        expr: Expression = self.comparision()

        while self.match(TT.BANG_EQUAL, TT.EQUAL_EQUAL):
            operator: Token = self.previous()
            right: Expression = self.comparision()
            expr = Binary(expr, operator, right)

        return expr

    def comparision(self) -> Expression:
        expr: Expression = self.term()

        while self.match(
            TT.GREATER,
            TT.GREATER_EQUAL,
            TT.LESS,
            TT.LESS_EQUAL,
        ):
            operator: Token = self.previous()
            right: Expression = self.term()
            expr = Binary(expr, operator, right)

        return expr

    def term(self) -> Expression:
        expr = self.factor()

        while self.match(TT.MINUS, TT.PLUS, TT.PLUS_PLUS, TT.CARET):
            operator: Token = self.previous()
            right: Expression = self.factor()
            expr = Binary(expr, operator, right)

        return expr

    def factor(self) -> Expression:
        expr = self.unary()

        while self.match(TT.SLASH, TT.STAR):
            operator: Token = self.previous()
            right: Expression = self.unary()
            expr = Binary(expr, operator, right)

        return expr

    def unary(self) -> Expression:
        if self.match(TT.BANG, TT.MINUS):
            operator: Token = self.previous()
            right: Expression = self.unary()
            return Unary(operator, right)

        return self.primary()

    def primary(self) -> Expression:
        if self.match(TT.FALSE):
            return Literal(False)

        if self.match(TT.TRUE):
            return Literal(True)

        if self.match(TT.NIL):
            return Literal(None)

        if self.match(TT.NUMBER, TT.STRING):
            return Literal(self.previous().literal)

        if self.match(TT.IDENTIFIER):
            return Variable(self.previous())

        if self.match(TT.LEFT_PAREN):
            expr: Expression = self.expression()
            self.consume(TT.RIGHT_PAREN, "Expect ')' after expression")
            return Groupping(expr)

        raise self.error(self.peek(), "Expect expression")

    def synchronize(self) -> None:
        self.advance()

        while not self.is_at_end():
            if self.previous().type == TT.SEMICOLON:
                return

            if self.peek().type in self.SYNC_BLOCKS:
                return

            self.advance()

    # Utility methods

    def match(self, *token_types: TT) -> bool:
        for token_type in token_types:
            if self.check(token_type):
                self.advance()
                return True
        return False

    def check(self, token_type: TT) -> bool:
        if self.is_at_end():
            return False

        return self.peek().type == token_type

    def advance(self) -> Token:
        if not self.is_at_end():
            self.current += 1

        return self.previous()

    def consume(self, type: TT, message: str) -> Token:
        if self.check(type):
            return self.advance()

        raise self.error(self.peek(), message)

    def error(self, token: Token, message: str) -> GusParseError:
        gustav.error(token, message)
        return GusParseError()

    def peek(self) -> Token:
        return self.tokens[self.current]

    def previous(self) -> Token:
        return self.tokens[self.current - 1]

    def is_at_end(self) -> bool:
        return self.peek().type == TT.EOF
