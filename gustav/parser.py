import typing as t  # noqa: F401

from gustav import gustav
from gustav.logging import LOG  # noqa: F401
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
            if declaration := self.parse_declaration():
                statements.append(declaration)

        return statements

    def parse_declaration(self) -> Statement | S.Var | None:
        try:
            if self.match(TT.FUN):
                return self.parse_function("function")

            if self.match(TT.VAR):
                return self.parse_var_declaration()

            return self.parse_statement()

        except GusParseError:
            self.synchronize()

        return None

    def parse_function(self, kind: str) -> S.Function:
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

        body: list[Statement] = self.parse_block()

        return S.Function(name, parameters, body)

    def parse_var_declaration(self) -> S.Var:
        name: Token = self.consume(TT.IDENTIFIER, "Expect variable name.")
        initializer: Expression | None = None

        if self.match(TT.EQUAL):
            initializer = self.parse_expression()

        self.consume(TT.SEMICOLON, "Expect ';' after variables declaration.")

        return S.Var(name, initializer)

    def parse_statement(self) -> Statement:
        if self.match(TT.PRINT):
            return self.parse_print_statement()

        if self.match(TT.RETURN):
            return self.parse_return_statement()

        if self.match(TT.LEFT_BRACE):
            return S.Block(self.parse_block())

        if self.match(TT.IF):
            return self.parse_if_statement()

        if self.match(TT.WHILE):
            return self.parse_while_statement()

        if self.match(TT.FOR):
            return self.parse_for_statement()

        return self.parse_expression_statement()

    def parse_return_statement(self) -> S.Return:
        keyword: Token = self.get_previous()
        value: Expression | None = None

        if not self.check(TT.SEMICOLON):
            value = self.parse_expression()

        self.consume(TT.SEMICOLON, "Expect ';' after return value")

        return S.Return(keyword, value)

    def parse_for_statement(self) -> Statement:
        """desugaring for loops into while loops"""
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'for'.")

        initializer: S.Var | S.Expr | None

        if self.match(TT.SEMICOLON):
            initializer = None
        elif self.match(TT.VAR):
            initializer = self.parse_var_declaration()
        else:
            initializer = self.parse_expression_statement()

        condition: Expression | None = None

        if not self.check(TT.SEMICOLON):
            condition = self.parse_expression()

        self.consume(TT.SEMICOLON, "Expect ';' after loop condition.")

        increment: Expression | None = None

        if not self.check(TT.RIGHT_PAREN):
            increment = self.parse_expression()

        self.consume(TT.RIGHT_PAREN, "Expect ')' after for clauses.")

        body: Statement = self.parse_statement()

        if increment is not None:
            body = S.Block([body, S.Expr(increment)])

        if condition is None:
            condition = E.Literal(True)

        body = S.While(condition, body)

        if initializer is not None:
            body = S.Block([initializer, body])

        return body

    def parse_while_statement(self) -> S.While:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'while'.")
        condition: Expression = self.parse_expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'.")

        body: Statement = self.parse_statement()

        return S.While(condition, body)

    def parse_if_statement(self) -> S.If:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'if'.")
        condition: Expression = self.parse_expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'.")

        then_branch: Statement = self.parse_statement()
        else_branch: Statement | None = None

        if self.match(TT.ELSE):
            else_branch = self.parse_statement()

        return S.If(condition, then_branch, else_branch)

    def parse_block(self) -> list[Statement]:
        statements: list[Statement] = list()

        while not self.check(TT.RIGHT_BRACE) and not self.is_at_end():
            if declaration := self.parse_declaration():
                statements.append(declaration)

        self.consume(TT.RIGHT_BRACE, "Expect '}' after block.")

        return statements

    def parse_print_statement(self) -> S.Print:
        value: Expression = self.parse_expression()
        self.consume(TT.SEMICOLON, "Expect ';' after value")

        return S.Print(value)

    def parse_expression_statement(self) -> S.Expr:
        expression = self.parse_expression()
        self.consume(TT.SEMICOLON, "Expect ';' after value")

        return S.Expr(expression)

    def parse_expression(self) -> Expression:
        return self.parse_assignment()

    def parse_assignment(self) -> Expression:
        expr: Expression = self.parse_or()

        if self.match(TT.EQUAL):
            equals: Token = self.get_previous()
            value: Expression = self.parse_assignment()

            if isinstance(expr, E.Variable):
                name: Token = expr.name
                return E.Assign(name, value)

            self.error(equals, "Invalid assignment target")

        return expr

    def parse_or(self) -> Expression:  # yes
        expr: Expression = self.parse_and()

        while self.match(TT.OR):
            operator: Token = self.get_previous()
            right: Expression = self.parse_and()
            expr = E.Logical(expr, operator, right)

        return expr

    def parse_and(self) -> Expression:  # yes
        expr: Expression = self.parse_equality()

        while self.match(TT.AND):
            operator: Token = self.get_previous()
            right: Expression = self.parse_equality()
            expr = E.Logical(expr, operator, right)

        return expr

    def parse_equality(self) -> Expression:
        expr: Expression = self.parse_comparision()

        while self.match(TT.BANG_EQUAL, TT.EQUAL_EQUAL):
            operator: Token = self.get_previous()
            right: Expression = self.parse_comparision()
            expr = E.Binary(expr, operator, right)

        return expr

    def parse_comparision(self) -> Expression:
        expr: Expression = self.parse_term()

        while self.match(
            TT.GREATER,
            TT.GREATER_EQUAL,
            TT.LESS,
            TT.LESS_EQUAL,
        ):
            operator: Token = self.get_previous()
            right: Expression = self.parse_term()
            expr = E.Binary(expr, operator, right)

        return expr

    def parse_term(self) -> Expression:
        expr = self.parse_factor()

        while self.match(TT.MINUS, TT.PLUS, TT.PLUS_PLUS, TT.CARET):
            operator: Token = self.get_previous()
            right: Expression = self.parse_factor()
            expr = E.Binary(expr, operator, right)

        return expr

    def parse_factor(self) -> Expression:
        expr = self.parse_unary()

        while self.match(TT.SLASH, TT.STAR):
            operator: Token = self.get_previous()
            right: Expression = self.parse_unary()
            expr = E.Binary(expr, operator, right)

        return expr

    def parse_unary(self) -> Expression:
        if self.match(TT.BANG, TT.MINUS):
            operator: Token = self.get_previous()
            right: Expression = self.parse_unary()
            return E.Unary(operator, right)

        return self.parse_call()

    def parse_call(self, pipe_arg: E.Call | None = None) -> E.Call | Expression:
        expr: Expression = self.parse_primary()

        while True:
            if self.match(TT.LEFT_PAREN):
                expr = self.finish_call(expr, pipe_arg)
            else:
                break

        return expr

    def finish_call(
        self, callee: Expression, pipe_arg: E.Call | None = None
    ) -> E.Call | Expression:
        """Parses arguments, closes call with RIGH_PAREN.

        Then, looks for pipe operator. Ex: g(x) |> f(y)
        Till |>, we have call expr = g(x), pipe encountered.

        Instead of finishing call, we recursively parse_call expr again
        to get f(y). In this moment, parse_call little more different than prev.

        We introduced new pipe_arg param, which will be appended in arguments list of f().
        So, given expression will be interpreted as: f(y,g(x))
        """
        arguments: list[Expression | E.Call] = list()

        if not self.check(TT.RIGHT_PAREN):
            arguments.append(self.parse_expression())

            while self.match(TT.COMMA):
                arguments.append(self.parse_expression())

        if pipe_arg:
            arguments.append(pipe_arg)

        paren: Token = self.consume(TT.RIGHT_PAREN, "Expect ')' after arguments.")

        call_expr: E.Call | Expression

        call_expr = E.Call(callee, paren, arguments)

        if self.match(TT.PIPE):
            call_expr = self.parse_call(call_expr)

        return call_expr

    def parse_primary(self) -> Expression:
        if self.match(TT.FALSE):
            return E.Literal(False)

        if self.match(TT.TRUE):
            return E.Literal(True)

        if self.match(TT.NIL):
            return E.Literal(None)

        if self.match(TT.NUMBER, TT.STRING):
            return E.Literal(self.get_previous().literal)

        if self.match(TT.IDENTIFIER):
            return E.Variable(self.get_previous())

        if self.match(TT.LEFT_PAREN):
            expr: Expression = self.parse_expression()
            self.consume(TT.RIGHT_PAREN, "Expect ')' after expression")
            return E.Groupping(expr)

        raise self.error(self.peek(), "Expect expression")

    def synchronize(self) -> None:
        LOG.info(f"Syncing ... peek={self.peek()}")
        self.advance()

        while not self.is_at_end():
            if self.get_previous().type == TT.SEMICOLON:
                return

            if self.peek().type in self.SYNC_BLOCKS:
                return

            self.advance()

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

        return self.get_previous()

    def consume(self, type: TT, message: str) -> Token:
        if self.check(type):
            return self.advance()

        raise self.error(self.peek(), message)

    def error(self, token: Token, message: str) -> GusParseError:
        gustav.error(token, message)
        return GusParseError()

    def peek(self) -> Token:
        return self.tokens[self.current]

    def get_previous(self) -> Token:
        return self.tokens[self.current - 1]

    def is_at_end(self) -> bool:
        return self.peek().type == TT.EOF
