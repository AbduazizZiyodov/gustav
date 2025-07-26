import contextlib
import typing as t
from dataclasses import dataclass, field
from functools import singledispatchmethod

from gustav import gustav
from gustav.logging import LOG  # noqa: F401
from gustav.token import Token
from gustav.enums import FunctionType, ClassType
from gustav.ast import expression as E, statement as S

__all__ = ("Resolver",)


@t.runtime_checkable
class CanResolveExpression(t.Protocol):
    def resolve(self, expression: E.Expression, depth: int) -> None: ...


class Resolver(E.ExpressionVisitor[None], S.StatementVisitor[None]):
    def __init__(self, interpreter: CanResolveExpression) -> None:
        self.interpreter = interpreter

        self.current_class = ClassType.NONE
        self.current_function = FunctionType.NONE

        self.scope: Scope = Scope()

    ###
    # Statements
    ###

    @t.override
    def visit_block_statement(self, statement: S.Block) -> None:
        with self.scope.enter():
            self.resolve(statement.statements)

    @t.override
    def visit_var_statement(self, statement: S.Var) -> None:
        self.declare(statement.name)

        if statement.initializer is not None:
            self.resolve(statement.initializer)

        self.define(statement.name)

    @t.override
    def visit_class_statement(self, statement: S.Class) -> None:
        enclosing_class: ClassType = self.current_class
        self.current_class = ClassType.CLASS

        self.declare(statement.name)
        self.define(statement.name)

        if (
            statement.superclass is not None
            and statement.name.lexeme == statement.superclass.name.lexeme
        ):
            gustav.error(statement.superclass.name, "A class can't inherit from itself")

        if statement.superclass is not None:
            self.current_class = ClassType.SUBCLASS
            self.resolve(statement.superclass)

        def do_resolve_methods() -> None:
            with self.scope.enter():
                self.scope.peek()["this"] = True

                for method in statement.methods:
                    func_type: FunctionType = (
                        FunctionType.METHOD
                        if method.name.lexeme != "init"
                        else FunctionType.INITIALIZER
                    )

                    self.resolve_function(method, func_type)

        if statement.superclass is not None:
            with self.scope.enter():
                self.scope.peek()["super"] = True
                do_resolve_methods()
        else:
            do_resolve_methods()

        self.current_class = enclosing_class

    @t.override
    def visit_function_statement(self, statement: S.Function) -> None:
        self.declare(statement.name)
        self.define(statement.name)
        self.resolve_function(statement, FunctionType.FUNCTION)

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
    def visit_print_statement(self, statement: S.Print) -> None:
        self.resolve(statement.expression)

    @t.override
    def visit_return_statement(self, statement: S.Return) -> None:
        if self.current_function == FunctionType.NONE:
            gustav.error(statement.keyword, "Can't return from top-level code")

        if statement.value is not None:
            if self.current_function == FunctionType.INITIALIZER:
                gustav.error(
                    statement.keyword, "Can't return a value from an initializer"
                )
            self.resolve(statement.value)

    @t.override
    def visit_while_statement(self, statement: S.While) -> None:
        self.resolve(statement.condition)
        self.resolve(statement.body)

    ###
    # Expressions
    ###
    @t.override
    def visit_get_expression(self, expression: E.Get) -> None:
        self.resolve(expression.object)

    @t.override
    def visit_set_expression(self, expression: E.Set) -> None:
        self.resolve(expression.value)
        self.resolve(expression.object)

    @t.override
    def visit_super_expression(self, expression: E.Super) -> None:
        if self.current_class == ClassType.NONE:
            gustav.error(expression.keyword, "Can't use 'super' outside of a class")

        elif self.current_class != ClassType.SUBCLASS:
            gustav.error(
                expression.keyword, "Can't use 'super' in a class with no superclass"
            )
        else:
            self.resolve_local(expression, expression.keyword)

    @t.override
    def visit_variable_expression(self, expression: E.Variable) -> None:
        if (
            self.scope.has_active_scope()
            and self.scope.peek().get(expression.name.lexeme) is False
        ):
            gustav.error(
                expression.name,
                "Can't read local variable in its own initializer",
            )

        self.resolve_local(expression, expression.name)

    @t.override
    def visit_ternary_expression(self, expression: E.Ternary) -> None:
        self.resolve(expression.condition)
        self.resolve(expression.then_branch)
        self.resolve(expression.else_branch)

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
    def visit_this_expression(self, expression: E.This) -> None:
        if self.current_class == ClassType.NONE:
            gustav.error(expression.keyword, "Can't use 'this' outside of a class")

        self.resolve_local(expression, expression.keyword)

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

    ###
    # Machinery
    ###

    @singledispatchmethod
    def resolve(self, stuff: t.Any) -> None:
        raise NotImplementedError(f"Can't resolve {stuff}")  # pragma: no cover

    @resolve.register(list)
    def _(self, statements: list[S.Statement]) -> None:
        for statement in statements:
            self.resolve(statement)

    @resolve.register
    def _(self, statement: S.Statement) -> None:
        statement.accept(self)

    @resolve.register
    def _(self, expression: E.Expression) -> None:
        expression.accept(self)

    def declare(self, name: Token) -> None:
        if not self.scope.has_active_scope():
            return

        try:
            self.scope.declare(name.lexeme)
        except RuntimeError as e:
            gustav.error(name, str(e))

    def define(self, name: Token) -> None:
        if not self.scope.has_active_scope():
            return

        self.scope.define(name.lexeme)

    def resolve_local(self, expression: E.Expression, name: Token) -> None:
        depth = self.scope.depth_of(name.lexeme)

        if depth is not None:
            self.interpreter.resolve(expression, depth)

    def resolve_function(self, statement: S.Function, type: FunctionType) -> None:
        enclosing_function: FunctionType = self.current_function
        self.current_function = type

        with self.scope.enter():
            for param in statement.params:
                self.declare(param)
                self.define(param)

            self.resolve(statement.body)

        self.current_function = enclosing_function


@dataclass
class Scope:
    _stack: list[dict[str, bool]] = field(default_factory=list)

    @contextlib.contextmanager
    def enter(self) -> t.Generator[None, t.Any, None]:
        self.begin_scope()

        try:
            yield
        finally:
            self.end_scope()

    def begin_scope(self) -> None:
        self._stack.append({})

    def end_scope(self) -> None:
        if not self._stack:
            raise RuntimeError("No active scope")  # pragma: no cover

        self._stack.pop()

    def peek(self) -> dict[str, bool]:
        if not self._stack:
            raise RuntimeError("No active scope")  # pragma: no cover

        return self._stack[-1]

    def define(self, name: str) -> None:
        self.peek()[name] = True

    def declare(self, name: str) -> None:
        scope = self.peek()

        if name in scope:
            raise RuntimeError("Already a variable with this name in this scope")

        scope[name] = False

    def depth_of(self, name: str) -> int | None:
        for i in range(len(self) - 1, -1, -1):
            if name in self._stack[i]:
                return len(self) - 1 - i

        return None

    def has_active_scope(self) -> bool:
        return bool(self._stack)

    def __len__(self) -> int:
        return len(self._stack)
