import typing as t  # noqa: F401

from gustav import gustav
from gustav.logging import LOG, DEBUG  # noqa: F401
from gustav.exceptions import GusParseError
from gustav.token import Token, TokenType as TT
from gustav.ast import Expression, Statement, expression as E, statement as S

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

    def declaration(self) -> Statement | S.Var | None:
        try:
            if self.match(TT.FUN):
                return self.function("function")

            if self.match(TT.VAR):
                return self.var_declaration()

            return self.statement()

        except GusParseError as exc:
            LOG.debug(f"Syncing ... {exc=}")
            self.synchronize()

        return None

    def function(self, kind: str) -> S.Function:
        name: Token = self.consume(TT.IDENTIFIER, f"Expect {kind} name.")

        self.consume(TT.LEFT_PAREN, f"Expect '(' after {kind} name.")

        parameters: list[Token] = list()

        if not self.check(TT.RIGHT_PAREN):
            while True:
                parameters.append(self.consume(TT.IDENTIFIER, "Expect parameter name."))
                if not self.match(TT.COMMA):
                    break

        self.consume(TT.RIGHT_PAREN, "Expect ')' after parameters.")
        self.consume(TT.LEFT_BRACE, f"Expect '{{' {kind}")

        body: list[Statement] = self.block()

        return S.Function(name, parameters, body)

    def var_declaration(self) -> S.Var:
        name: Token = self.consume(TT.IDENTIFIER, "Expect variable name.")
        initializer: Expression | None = None

        if self.match(TT.EQUAL):
            initializer = self.expression()

        self.consume(TT.SEMICOLON, "Expect ';' after variables declaration.")

        return S.Var(name, initializer)

    def statement(self) -> Statement:
        if self.match(TT.PRINT):
            return self.print_statement()

        if self.match(TT.RETURN):
            return self.return_statement()

        if self.match(TT.LEFT_BRACE):
            return S.Block(self.block())

        if self.match(TT.IF):
            return self.if_statement()

        if self.match(TT.WHILE):
            return self.while_statement()

        if self.match(TT.FOR):
            return self.for_statement()

        return self.expression_statement()

    def return_statement(self) -> S.Return:
        keyword: Token = self.previous()
        value: Expression | None = None

        if not self.check(TT.SEMICOLON):
            value = self.expression()

        self.consume(TT.SEMICOLON, "Expect ';' after return value")

        return S.Return(keyword, value)

    def for_statement(self) -> Statement:
        """desugaring for loops into while loops"""
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'for'.")

        initializer: S.Var | S.Expr | None

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
            body = S.Block([body, S.Expr(increment)])

        if condition is None:
            condition = E.Literal(True)

        body = S.While(condition, body)

        if initializer is not None:
            body = S.Block([initializer, body])

        return body

    def while_statement(self) -> S.While:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'while'.")
        condition: Expression = self.expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'.")

        body: Statement = self.statement()

        return S.While(condition, body)

    def if_statement(self) -> S.If:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'if'.")
        condition: Expression = self.expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'.")

        then_branch: Statement = self.statement()
        else_branch: Statement | None = None

        if self.match(TT.ELSE):
            else_branch = self.statement()

        return S.If(condition, then_branch, else_branch)

    def block(self) -> list[Statement]:
        statements: list[Statement] = list()

        while not self.check(TT.RIGHT_BRACE) and not self.is_at_end():
            if declaration := self.declaration():
                statements.append(declaration)

        self.consume(TT.RIGHT_BRACE, "Expect '}' after block.")
        return statements

    def print_statement(self) -> S.Print:
        value: Expression = self.expression()
        self.consume(TT.SEMICOLON, "Expect ';' after value")

        return S.Print(value)

    def expression_statement(self) -> S.Expr:
        expression = self.expression()
        self.consume(TT.SEMICOLON, "Expect ';' after value")

        return S.Expr(expression)

    def expression(self) -> Expression:
        return self.assignment()

    def assignment(self) -> Expression:
        expr: Expression = self.Or()

        if self.match(TT.EQUAL):
            equals: Token = self.previous()
            value: Expression = self.assignment()

            if isinstance(expr, E.Variable):
                name: Token = expr.name
                return E.Assign(name, value)

            self.error(equals, "Invalid assignment target")

        return expr

    def Or(self) -> Expression:  # yes
        expr: Expression = self.And()

        while self.match(TT.OR):
            operator: Token = self.previous()
            right: Expression = self.And()
            expr = E.Logical(expr, operator, right)

        return expr

    def And(self) -> Expression:  # yes
        expr: Expression = self.equality()

        while self.match(TT.AND):
            operator: Token = self.previous()
            right: Expression = self.equality()
            expr = E.Logical(expr, operator, right)

        return expr

    def equality(self) -> Expression:
        expr: Expression = self.comparision()

        while self.match(TT.BANG_EQUAL, TT.EQUAL_EQUAL):
            operator: Token = self.previous()
            right: Expression = self.comparision()
            expr = E.Binary(expr, operator, right)

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
            expr = E.Binary(expr, operator, right)

        return expr

    def term(self) -> Expression:
        expr = self.factor()

        while self.match(TT.MINUS, TT.PLUS, TT.PLUS_PLUS, TT.CARET):
            operator: Token = self.previous()
            right: Expression = self.factor()
            expr = E.Binary(expr, operator, right)

        return expr

    def factor(self) -> Expression:
        expr = self.unary()

        while self.match(TT.SLASH, TT.STAR):
            operator: Token = self.previous()
            right: Expression = self.unary()
            expr = E.Binary(expr, operator, right)

        return expr

    def unary(self) -> Expression:
        if self.match(TT.BANG, TT.MINUS):
            operator: Token = self.previous()
            right: Expression = self.unary()
            return E.Unary(operator, right)

        return self.call()

    def call(self) -> Expression:
        # LOG.info(f"{self.peek()=}")
        expr: Expression = self.primary()

        while True:
            if self.match(TT.LEFT_PAREN):
                LOG.info("Inside function all")
                expr = self.finish_call(expr)
                LOG.info("Function call finished")
            else:
                break

        return expr

    def finish_call(self, callee: Expression) -> Expression:
        arguments: list[Expression] = list()

        if not self.check(TT.RIGHT_PAREN):
            arguments.append(self.expression())
            while self.check(TT.COMMA):
                arguments.append(self.expression())

        paren: Token = self.consume(TT.RIGHT_PAREN, "Expect ')' after arguments.")

        return E.Call(callee, paren, arguments)

    def primary(self) -> Expression:
        if self.match(TT.FALSE):
            return E.Literal(False)

        if self.match(TT.TRUE):
            return E.Literal(True)

        if self.match(TT.NIL):
            return E.Literal(None)

        if self.match(TT.NUMBER, TT.STRING):
            return E.Literal(self.previous().literal)

        if self.match(TT.IDENTIFIER):
            return E.Variable(self.previous())

        if self.match(TT.LEFT_PAREN):
            expr: Expression = self.expression()
            self.consume(TT.RIGHT_PAREN, "Expect ')' after expression")
            return E.Groupping(expr)

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
