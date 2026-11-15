import typing as t  # noqa: F401

from functools import wraps

from dataclasses import dataclass, field

from gustav import gustav
from gustav.logging import LOG  # noqa: F401
from gustav.token import Token
from gustav.enums import TokenType as TT
from gustav.exceptions import GusParseError
from gustav.ast import expression as E, statement as S

__all__ = ("Parser", "UNINITIALIZED")


UNINITIALIZED = sentinel("UNINITIALIZED")

P = t.ParamSpec("P")
T = t.TypeVar("T")


def control_loop_depth(
    f: t.Callable[t.Concatenate["Parser", P], T],
) -> t.Callable[t.Concatenate["Parser", P], T]:
    @wraps(f)
    def wrapper(parser: "Parser", *args: P.args, **kwargs: P.kwargs) -> T:
        try:
            parser.loop_depth += 1
            return f(parser, *args, **kwargs)
        finally:
            parser.loop_depth -= 1

    return t.cast(t.Callable[t.Concatenate["Parser", P], T], wrapper)


@dataclass(slots=True)
class Parser:
    """Recursive descent parser"""

    tokens: list[Token]
    current: int = field(default=0)
    loop_depth: int = field(default=0)

    def parse(self) -> list[S.Statement]:
        statements: list[S.Statement] = list()

        while not self.is_end:
            if declaration := self.parse_declaration():
                statements.append(declaration)

        return statements

    def parse_declaration(self) -> S.Statement | None:
        try:
            if self.match(TT.VAR):
                return self.parse_var_declaration()

            return self.parse_statement()

        except GusParseError:
            self.synchronize()

        return None

    def parse_var_declaration(self) -> S.Var:
        name: Token = self.consume(TT.IDENTIFIER, "Expect variable name")
        initializer: E.Expression = E.Literal(UNINITIALIZED)

        if self.match(TT.EQUAL):
            initializer = self.parse_expression()

        self.consume(TT.SEMICOLON, "Expect ';' after variables declaration")

        return S.Var(name, initializer)

    def parse_statement(self) -> S.Statement:
        if self.match(TT.BREAK):
            return self.parse_break_statement()

        if self.match(TT.CONTINUE):
            return self.parse_continue_statement()

        if self.match(TT.PRINT):
            return self.parse_print_statement()

        if self.match(TT.LEFT_BRACE):
            return S.Block(self.parse_block())

        if self.match(TT.IF):
            return self.parse_if_statement()

        if self.match(TT.WHILE):
            return self.parse_while_statement()

        if self.match(TT.LOOP):
            return self.parse_loop_statement()

        if self.match(TT.FOR):
            return self.parse_for_statement()

        return self.parse_expression_statement()

    def parse_break_statement(self) -> S.Break:
        self.consume(TT.SEMICOLON, "Expect ';' after break statement")
        return S.Break(self.get_previous())

    def parse_continue_statement(self) -> S.Continue:
        self.consume(TT.SEMICOLON, "Expect ';' after continue statement")
        return S.Continue(self.get_previous())

    def parse_return_statement(self) -> S.Return:
        keyword: Token = self.get_previous()
        value: E.Expression | None = None

        if not self.check(TT.SEMICOLON):
            value = self.parse_expression()

        self.consume(TT.SEMICOLON, "Expect ';' after return value")

        return S.Return(keyword, value)

    @control_loop_depth
    def parse_for_statement(self) -> S.For:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'for'")

        initializer: S.Var | S.Expr | None

        if self.match(TT.SEMICOLON):
            initializer = None
        elif self.match(TT.VAR):
            initializer = self.parse_var_declaration()
        else:
            initializer = self.parse_expression_statement()

        condition: E.Expression | None = None

        if not self.check(TT.SEMICOLON):
            condition = self.parse_expression()

        self.consume(TT.SEMICOLON, "Expect ';' after loop condition")

        increment: S.Expr | None = None

        if not self.check(TT.RIGHT_PAREN):
            # partial coverage
            if (
                increment_expr := self.parse_expression()
            ) is not None:  # pragma: no cover
                increment = S.Expr(increment_expr)

        self.consume(TT.RIGHT_PAREN, "Expect ')' after for clauses")

        body: S.Statement = self.parse_statement()

        if condition is None:
            condition = E.Literal(True)

        return S.For(initializer, condition, increment, body)

    @control_loop_depth
    def parse_loop_statement(self) -> S.While:
        """Same as while loop, but infinite.

        `loop {...}` (actually its `loop <statement>`) is equivalent to `while (true) {...}`

        This method desugars loop statement into while
        with only one addition - setting condition to true (literal)
        """
        condition = E.Literal(value=True)
        body: S.Statement = self.parse_statement()

        return S.While(condition, body=body)

    @control_loop_depth
    def parse_while_statement(self) -> S.While:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'while'")
        condition: E.Expression = self.parse_expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'")

        body: S.Statement = self.parse_statement()

        return S.While(condition, body)

    def parse_if_statement(self) -> S.If:
        self.consume(TT.LEFT_PAREN, "Expect '(' after 'if'")
        condition: E.Expression = self.parse_expression()
        self.consume(TT.RIGHT_PAREN, "Expect ')' after 'condition'")

        then_branch: S.Statement = self.parse_statement()
        else_branch: S.Statement | None = None

        if self.match(TT.ELSE):
            else_branch = self.parse_statement()

        return S.If(condition, then_branch, else_branch)

    def parse_block(self) -> list[S.Statement]:
        statements: list[S.Statement] = list()

        while not self.check(TT.RIGHT_BRACE) and not self.is_end:
            if declaration := self.parse_declaration():
                statements.append(declaration)

        self.consume(TT.RIGHT_BRACE, "Expect '}' after block")

        return statements

    def parse_print_statement(self) -> S.Print:
        value: E.Expression = self.parse_expression()
        self.consume(TT.SEMICOLON, "Expect ';' after value")

        return S.Print(value)

    def parse_expression_statement(self) -> S.Expr:
        expression = self.parse_expression()
        self.consume(TT.SEMICOLON, "Expect ';' after expression")

        return S.Expr(expression)

    def parse_expression(self) -> E.Expression:
        assignment_expr = self.parse_assignment()
        return assignment_expr

    def parse_assignment(self) -> E.Expression:
        expr: E.Expression = self.parse_or()

        if self.match(TT.EQUAL):
            equals: Token = self.get_previous()
            value: E.Expression = self.parse_assignment()

            if isinstance(expr, E.Variable):
                name: Token = expr.name
                return E.Assign(name, value)

            self.error(equals, "Invalid assignment target")

        return expr

    def parse_or(self) -> E.Expression:
        expr: E.Expression = self.parse_and()

        while self.match(TT.OR):
            operator: Token = self.get_previous()
            right: E.Expression = self.parse_and()
            expr = E.Logical(expr, operator, right)

        return expr

    def parse_and(self) -> E.Expression:
        expr: E.Expression = self.parse_equality()

        while self.match(TT.AND):
            operator: Token = self.get_previous()
            right: E.Expression = self.parse_equality()
            expr = E.Logical(expr, operator, right)

        return expr

    def parse_equality(self) -> E.Expression:
        expr: E.Expression = self.parse_comparision()

        while self.match(TT.BANG_EQUAL, TT.EQUAL_EQUAL):
            operator: Token = self.get_previous()
            right: E.Expression = self.parse_comparision()
            expr = E.Binary(expr, operator, right)

        while self.match(TT.QUESTION_MARK):
            # allows nesting: a ? (b ? c : d) : e
            expr = self.parse_ternary(expr)

        return expr

    def parse_ternary(self, condition: E.Expression) -> E.Ternary:
        true_expr: E.Expression = self.parse_assignment()
        self.consume(TT.COLON, "Expect ':' after '?' in ternary expr")
        false_expr: E.Expression = self.parse_assignment()

        return E.Ternary(
            condition,
            true_expr,
            false_expr,
        )

    def parse_comparision(self) -> E.Expression:
        expr: E.Expression = self.parse_term()

        while self.match(
            TT.GREATER,
            TT.GREATER_EQUAL,
            TT.LESS,
            TT.LESS_EQUAL,
        ):
            operator: Token = self.get_previous()
            right: E.Expression = self.parse_term()
            expr = E.Binary(expr, operator, right)

        return expr

    def parse_term(self) -> E.Expression:
        expr = self.parse_factor()

        while self.match(TT.MINUS, TT.PLUS):
            operator: Token = self.get_previous()
            right: E.Expression = self.parse_factor()
            expr = E.Binary(expr, operator, right)

        return expr

    def parse_factor(self) -> E.Expression:
        expr = self.parse_unary()

        while self.match(TT.SLASH, TT.STAR):
            operator: Token = self.get_previous()
            right: E.Expression = self.parse_unary()
            expr = E.Binary(expr, operator, right)

        return expr

    def parse_unary(self) -> E.Expression:
        if self.match(TT.BANG, TT.MINUS):
            operator: Token = self.get_previous()
            right: E.Expression = self.parse_unary()
            return E.Unary(operator, right)

        return self.parse_primary()

    def parse_primary(self) -> E.Expression:
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
            expr: E.Expression = self.parse_expression()
            self.consume(TT.RIGHT_PAREN, "Expect ')' after expression")

            return E.Groupping(expr)

        raise self.error(self.peek(), "Expect expression")


    ###
    # Machinery
    ###
    SYNC_BLOCKS: t.ClassVar[tuple[TT, ...]] = (
        TT.FOR,
        TT.IF,
        TT.PRINT,
        TT.VAR,
        TT.WHILE,
    )

    def synchronize(self) -> None:
        LOG.debug(f"Syncing ... peek={self.peek()}")

        self.advance()

        while not self.is_end:
            if self.get_previous().type == TT.SEMICOLON:
                return

            if self.peek().type in Parser.SYNC_BLOCKS:
                return  # pragma: no cover

            self.advance()

    def match(self, *token_types: TT) -> bool:
        for token_type in token_types:
            if self.check(token_type):
                self.advance()
                return True
        return False

    def check(self, token_type: TT) -> bool:
        if self.is_end:
            return False

        return self.peek().type == token_type

    def advance(self) -> Token:
        if not self.is_end:
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

    @property
    def is_end(self) -> bool:
        return self.peek().type == TT.EOF
