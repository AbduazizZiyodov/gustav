import contextlib
import typing as t
from functools import singledispatchmethod

from gustav import gustav
from gustav.logging import LOG  # noqa: F401
from gustav.token import Token
from gustav.protocols import CanResolveExpression
from gustav.ast import expression as E, statement as S
from gustav.enums import FunctionType, ClassType, VariableState

__all__ = ("Resolver",)


class Resolver(E.ExpressionVisitor[None], S.StatementVisitor[None]):
    def __init__(self, interpreter: CanResolveExpression) -> None:
        self.interpreter = interpreter

        self.current_class = ClassType.NONE
        self.current_function = FunctionType.NONE

        self.in_loop: bool = False

        self.scopes: list[dict[str, tuple[Token, VariableState]]] = []

    ###
    # Statements
    ###

    @t.override
    def visit_block_statement(self, statement: S.Block) -> None:
        with self.enter_scope():
            self.resolve(statement.statements)

    @t.override
    def visit_var_statement(self, statement: S.Var) -> None:
        self.declare(statement.name)

        if statement.initializer is not None:
            self.resolve(statement.initializer)

        self.define(statement.name)

    @t.override
    def visit_break_statement(self, statement: S.Break) -> None:
        if not self.in_loop:
            gustav.error(statement.keyword, "Can't use 'break' outside of a loop")

    @t.override
    def visit_continue_statement(self, statement: S.Continue) -> None:
        if not self.in_loop:
            gustav.error(statement.keyword, "Can't use 'continue' outside of a loop")

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
            with self.enter_scope():
                self.peek()["this"] = (statement.name, VariableState.USED)

                for method in statement.methods:
                    func_type: FunctionType = (
                        FunctionType.METHOD
                        if method.name.lexeme != "init"
                        else FunctionType.INITIALIZER
                    )

                    self.resolve_function(method, func_type)

        if statement.superclass is not None:
            with self.enter_scope():
                self.peek()["super"] = (statement.name, VariableState.USED)
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
        with self.within_loop():
            self.resolve(statement.condition)
            self.resolve(statement.body)

    @t.override
    def visit_for_statement(self, statement: S.For) -> None:
        with self.within_loop():
            with self.enter_scope():
                if statement.initializer:
                    self.resolve(statement.initializer)

                # partial coverage
                if statement.condition:  # pragma: no cover
                    self.resolve(statement.condition)

                if statement.increment:
                    self.resolve(statement.increment)

                self.resolve(statement.body)

    ###
    # Expressions
    ###

    @t.override
    def visit_lambda_expression(self, expr: E.Lambda) -> None:
        self.resolve_function(expr, FunctionType.LAMBDA)

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
            self.resolve_local(expression, expression.keyword, True)

    @t.override
    def visit_variable_expression(self, expression: E.Variable) -> None:
        if (
            self.has_active_scope()
            and expression.name.lexeme in self.peek()
            and (scope_value := self.peek().get(expression.name.lexeme)) is not None
            and scope_value[1] == VariableState.DECLARED
        ):
            gustav.error(
                expression.name,
                "Can't read local variable in its own initializer",
            )
        LOG.debug(f"Resolving variable expr {expression.name.lexeme=}")
        self.resolve_local(expression, expression.name, True)

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

        self.resolve_local(expression, expression.keyword, True)

    @t.override
    def visit_unary_expression(self, expression: E.Unary) -> None:
        self.resolve(expression.right)

    @t.override
    def visit_literal_expression(self, _: t.Any) -> None:
        return None

    @t.override
    def visit_assign_expression(self, expression: E.Assign) -> None:
        self.resolve(expression.value)
        self.resolve_local(expression, expression.name, False)

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

    def resolve_local(
        self, expression: E.Expression, name: Token, mark_as_used: bool
    ) -> None:
        for i in range(len(self.scopes) - 1, -1, -1):
            if name.lexeme not in self.scopes[i]:
                continue

            depth = len(self.scopes) - 1 - i

            self.interpreter.resolve(expression, depth)

            if mark_as_used:
                self.scopes[i][name.lexeme] = (name, VariableState.USED)

            return

    def resolve_function(
        self, statement: S.Function | E.Lambda, type: FunctionType
    ) -> None:
        enclosing_loop = self.in_loop

        # NOTE(abduazizziyodov): for (...) { lambda() { break; }(); }
        self.in_loop = False

        enclosing_function: FunctionType = self.current_function
        self.current_function = type

        with self.enter_scope():
            for param in statement.params:
                self.declare(param)
                self.define(param)

            self.resolve(statement.body)

        self.current_function = enclosing_function

        self.in_loop = enclosing_loop

    @contextlib.contextmanager
    def within_loop(self) -> t.Generator[None, t.Any, None]:
        outer = self.in_loop
        self.in_loop = True

        try:
            yield
        finally:
            self.in_loop = outer

    ###
    # Scope related machinery
    ###

    @contextlib.contextmanager
    def enter_scope(self) -> t.Generator[None, t.Any, None]:
        self.begin_scope()
        LOG.debug("Entering scope")
        try:
            yield
        finally:
            self.end_scope()

    def begin_scope(self) -> None:
        self.scopes.append({})

    def end_scope(self) -> None:
        scope = self.scopes.pop()

        for _, value in scope.items():
            name, status = value
            if status == VariableState.DEFINED:
                gustav.warning(name, f"Variable '{name.lexeme}' is not used")

    def peek(self) -> dict[str, tuple[Token, VariableState]]:
        return self.scopes[-1]

    def define(self, name: Token) -> None:
        if not self.has_active_scope():
            return

        self.peek()[name.lexeme] = (name, VariableState.DEFINED)

    def declare(self, name: Token) -> None:
        if not self.has_active_scope():
            return

        scope = self.peek()

        if name.lexeme in scope:
            gustav.error(name, "Already a variable with this name in this scope")

        scope[name.lexeme] = (name, VariableState.DECLARED)

    def has_active_scope(self) -> bool:
        return bool(self.scopes)
