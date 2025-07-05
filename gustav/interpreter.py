import sys
import typing as t

from gustav.logging import LOG  # noqa: F401
from gustav import gustav
from gustav.ast import (
    Expression,
    Statement,
    expression as E,
    statement as S,
)
from gustav.builtins import BUILTINS
from gustav.environment import Environment
from gustav.token import Token
from gustav.types import TokenType as TT
from gustav.callables import GusCallable, GusFunction
from gustav.exceptions import GusRuntimeError, GusReturn

__all__ = ("Interpreter",)


class Interpreter(E.ExpressionVisitor[t.Any], S.StatementVisitor[None]):
    def __init__(self) -> None:
        self.globals: Environment = Environment()
        self.locals: dict[Expression, int] = dict()
        self.environment: Environment = self.globals

        self.init_builtins()

    def init_builtins(self) -> None:
        for fn_name, callable in BUILTINS:
            self.globals.define(fn_name, callable)

    def interpret(self, statements: list[Statement]) -> None:
        try:
            for statement in statements:
                self.execute(statement)

        except GusRuntimeError as exc:
            gustav.runtime_error(exc)

        return None

    def execute(self, statement: Statement) -> None:
        statement.accept(self)

    def resolve(self, expression: E.Expression, depth: int) -> None:
        self.locals[expression] = depth

    def look_up_variable(self, name: Token, expr: Expression) -> t.Any:
        distance: int | None = self.locals.get(expr)

        if distance is not None:
            return self.environment.get_at(distance, name.lexeme)

        return self.globals.get(name)

    def evaluate(self, expression: Expression) -> t.Any:
        return expression.accept(self)

    def execute_block(
        self, statements: list[Statement], environment: Environment
    ) -> None:
        previous_environment: Environment = self.environment

        try:
            self.environment = environment

            for statement in statements:
                self.execute(statement)

        finally:
            self.environment = previous_environment

    ###
    # Statements
    ###

    @t.override
    def visit_return_statement(self, statement: S.Return) -> t.Never:
        value: t.Any = None

        if (val := statement.value) is not None:
            value = self.evaluate(val)

        raise GusReturn(value)

    @t.override
    def visit_function_statement(self, statement: S.Function) -> None:
        func: GusFunction = GusFunction(statement, self.environment)
        self.environment.define(statement.name.lexeme, func)
        return None

    @t.override
    def visit_while_statement(self, statement: S.While) -> None:
        while self.is_truthy(self.evaluate(statement.condition)):
            self.execute(statement.body)

    @t.override
    def visit_logical_expression(self, expression: E.Logical) -> t.Any:
        left: t.Any = self.evaluate(expression.left)

        if expression.operator.type == TT.OR:
            if self.is_truthy(left):
                return left
        else:
            if not self.is_truthy(left):
                return left

        return self.evaluate(expression.right)

    @t.override
    def visit_if_statement(self, statement: S.If) -> None:
        if self.is_truthy(self.evaluate(statement.condition)):
            self.execute(statement.then_branch)

        elif statement.else_branch is not None:
            self.execute(statement.else_branch)

        return None

    @t.override
    def visit_expr_statement(self, statement: S.Expr) -> None:
        self.evaluate(statement.expression)
        return None

    @t.override
    def visit_print_statement(self, statement: S.Print) -> None:
        value: t.Any = self.evaluate(statement.expression)
        line: str = self.stringfy(value) + "\n"
        sys.stdout.write(line)

        return None

    @t.override
    def visit_var_statement(self, statement: S.Var) -> None:
        value: t.Any = None

        if statement.initializer is not None:
            value = self.evaluate(statement.initializer)

        self.environment.define(statement.name.lexeme, value)

    @t.override
    def visit_block_statement(self, statement: S.Block) -> None:
        self.execute_block(
            statement.statements,
            Environment(self.environment),
        )

        return None

    ###
    # Expressions
    ###
    @t.override
    def visit_call_expression(self, expression: E.Call) -> t.Any:
        callee: t.Any = self.evaluate(expression.callee)

        arguments: list[t.Any] = list(map(self.evaluate, expression.arguments))

        if not isinstance(callee, GusCallable):
            raise GusRuntimeError(
                expression.paren, "Can only call functions and classes."
            )

        func: GusCallable = callee

        if (current_arity := len(arguments)) != (required_arity := func.arity()):
            raise GusRuntimeError(
                expression.paren,
                f"Expected {required_arity} arguments but got {current_arity}",
            )

        return func.call(self, arguments)

    @t.override
    def visit_assign_expression(self, expression: E.Assign) -> t.Any:
        value: t.Any = self.evaluate(expression.value)

        distance: int | None = self.locals.get(expression)

        if distance is not None:
            self.environment.assign_at(distance, expression.name, value)
        else:
            self.globals.assign(expression.name, value)

        return value

    @t.override
    def visit_variable_expression(self, expression: E.Variable) -> t.Any:
        return self.look_up_variable(expression.name, expression)

    @t.override
    def visit_literal_expression(self, expression: E.Literal) -> t.Any:
        return expression.value

    @t.override
    def visit_groupping_expression(self, expression: E.Groupping) -> t.Any:
        return self.evaluate(expression.expression)

    @t.override
    def visit_unary_expression(self, expression: E.Unary) -> t.Any:
        right = self.evaluate(expression.right)

        match expression.operator.type:
            case TT.BANG:
                return not self.is_truthy(right)

            case TT.MINUS:
                return -1 * right

    @t.override
    def visit_binary_expression(self, expression: E.Binary) -> t.Any:
        left = self.evaluate(expression.left)
        right = self.evaluate(expression.right)

        def check_for_number() -> t.NoReturn | None:
            return self.check_number_operands(expression.operator, left, right)

        if expression.operator.type in (
            TT.GREATER,
            TT.GREATER_EQUAL,
            TT.LESS,
            TT.LESS_EQUAL,
            TT.MINUS,
            TT.PLUS,
            TT.STAR,
            TT.SLASH,
        ):
            check_for_number()

        match expression.operator.type:
            case TT.GREATER:
                return left > right

            case TT.GREATER_EQUAL:
                return left >= right

            case TT.LESS:
                return left < right

            case TT.LESS_EQUAL:
                return left <= right

            case TT.MINUS:
                return left - right

            case TT.PLUS:
                return left + right

            case TT.PLUS_PLUS:  # concatenation operator
                return "".join((str(left), str(right)))

            case TT.STAR:
                return left * right

            case TT.SLASH:
                if right == 0:
                    return float("inf")  # yes

                return left / right

            case TT.BANG_EQUAL:
                return not self.is_equal(left, right)

            case TT.EQUAL_EQUAL:
                return self.is_equal(left, right)

            case TT.CARET:
                return pow(left, right)

    ###
    # Utility
    ###

    def check_number_operands(
        self, operator: Token, *operands: t.Any
    ) -> t.NoReturn | None:
        if all(
            isinstance(operand, int) or isinstance(operand, float)
            for operand in operands
        ):
            return None

        raise GusRuntimeError(operator, "Operands must be a number")

    def is_equal(self, a: t.Any, b: t.Any) -> bool:
        return bool(a == b)

    def is_truthy(self, value: t.Any) -> bool:
        if value in (None, False):
            return False

        return True

    def stringfy(self, value: t.Any) -> str:
        if value is None:
            return "nil"

        if value in (True, False):
            return str(value).lower()

        return str(value)
