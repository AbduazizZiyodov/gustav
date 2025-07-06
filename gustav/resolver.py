import contextlib
import typing as t
from collections import deque
from functools import singledispatchmethod

from gustav import gustav
from gustav.token import Token
from gustav.types import FunctionType
from gustav.ast import expression as E, statement as S

__all__ = ("Resolver",)


class Resolver(E.ExpressionVisitor[None], S.StatementVisitor[None]):
    def __init__(self, interpreter: t.Any) -> None:  # TODO: interpreter prot.
        self.interpreter = interpreter
        self.current_function = FunctionType.NONE
        self.scopes: deque[dict[str, bool]] = deque()

    @singledispatchmethod
    def resolve(self, _: t.Any) -> None:
        raise NotImplementedError("Can't resolve this stuff")

    # resolve statements
    @resolve.register(list)
    def _(self, statements: list[S.Statement]) -> None:
        for statement in statements:
            self.resolve(statement)

    # resolve single statement
    @resolve.register
    def _(self, statement: S.Statement) -> None:
        statement.accept(self)

    # resolve expression
    @resolve.register
    def _(self, expression: E.Expression) -> None:
        expression.accept(self)

    def declare(self, name: Token) -> None:
        if not self.scopes:
            return

        scope: dict[str, bool] = self.scopes[-1]

        if scope.get(name.lexeme) is not None:
            gustav.error(name, "Already a variable with this name in this scope.")

        scope[name.lexeme] = False

    def define(self, name: Token) -> None:
        if not self.scopes:
            return

        peek: dict[str, bool] = self.scopes[-1]
        peek[name.lexeme] = True

    @t.override
    def visit_block_statement(self, statement: S.Block) -> None:
        with self.scope():
            self.resolve(statement.statements)

    @t.override
    def visit_var_statement(self, statement: S.Var) -> None:
        self.declare(statement.name)

        if statement.initializer is not None:
            self.resolve(statement.initializer)

        self.define(statement.name)

    @t.override
    def visit_function_statement(self, statement: S.Function) -> None:
        self.declare(statement.name)
        self.define(statement.name)
        self.resolve_function(statement, FunctionType.FUNCTION)

    @t.override
    def visit_variable_expression(self, expression: E.Variable) -> None:
        if self.scopes and self.scopes[-1].get(expression.name.lexeme) is False:
            gustav.error(
                expression.name,
                "Can't read local variable in its own initializer.",
            )

        self.resolve_local(expression, expression.name)

    @t.override
    def visit_expr_statement(self, statement: S.Expr) -> None:
        self.resolve(statement.expression)

    @t.override
    def visit_if_statement(self, statement: S.If) -> None:
        self.resolve(statement.condition)
        self.resolve(statement.then_branch)

        if statement.else_branch is not None:
            self.resolve(statement.else_branch)

    @t.override
    def visit_ternary_expression(self, expression: E.Ternary) -> None:
        self.resolve(expression.condition)
        self.resolve(expression.then_branch)
        self.resolve(expression.else_branch)

    @t.override
    def visit_print_statement(self, statement: S.Print) -> None:
        self.resolve(statement.expression)

    @t.override
    def visit_return_statement(self, statement: S.Return) -> None:
        if self.current_function == FunctionType.NONE:
            gustav.error(statement.keyword, "Can't return from top-level code.")

        if statement.value is not None:
            self.resolve(statement.value)

    @t.override
    def visit_while_statement(self, statement: S.While) -> None:
        self.resolve(statement.condition)
        self.resolve(statement.body)

    @t.override
    def visit_binary_expression(self, expression: E.Binary) -> None:
        self.resolve(expression.left)
        self.resolve(expression.right)

    @t.override
    def visit_call_expression(self, expression: E.Call) -> None:
        self.resolve(expression.callee)

        for argument in expression.arguments:
            self.resolve(argument)

    @t.override
    def visit_groupping_expression(self, expression: E.Groupping) -> None:
        self.resolve(expression.expression)

    @t.override
    def visit_logical_expression(self, expression: E.Logical) -> None:
        self.resolve(expression.left)
        self.resolve(expression.right)

    @t.override
    def visit_unary_expression(self, expression: E.Unary) -> None:
        self.resolve(expression.right)

    @t.override
    def visit_literal_expression(self, _: t.Any) -> None:
        return None

    @t.override
    def visit_assign_expression(self, expression: E.Assign) -> None:
        self.resolve(expression.value)
        self.resolve_local(expression, expression.name)

    def resolve_local(self, expression: E.Expression, name: Token) -> None:
        for i in reversed(range(length := len(self.scopes))):
            if self.scopes[i].get(name.lexeme) is not None:
                self.interpreter.resolve(expression, length - 1 - i)
                break

    def resolve_function(self, statement: S.Function, type: FunctionType) -> None:
        enclosing_function = self.current_function
        self.current_function = type

        with self.scope():
            for param in statement.params:
                self.declare(param)
                self.define(param)

            self.resolve(statement.body)

        self.current_function = enclosing_function

    @contextlib.contextmanager
    def scope(self) -> t.Generator[None, t.Any, None]:
        self.begin_scope()
        try:
            yield
        finally:
            self.end_scope()

    def begin_scope(self) -> None:
        self.scopes.append(dict())

    def end_scope(self) -> None:
        if not self.scopes:
            raise RuntimeError("end_scope() called with no active scope")

        self.scopes.pop()
