import typing as t

from .types import Token
from .logging import LOG
from .enums import TokenType
from .errors import runtime_error
from .exceptions import GusRuntimeError
from .ast import (
    Expression,
    Binary,
    Groupping,
    Literal,
    Variable,
    Unary,
    ExpressionVisitor,
    Statement,
    Expr,
    Print,
    Var,
    StatementVisitor,
)


__all__ = ["Interpreter"]


class Interpreter(ExpressionVisitor[t.Any], StatementVisitor[None]):
    def interpret(self, statements: list[Statement]) -> None:
        try:
            for statement in statements:
                self.execute(statement)
        except GusRuntimeError as exc:
            LOG.error(f"Runtime error occurred: {exc=}")
            runtime_error(exc)

        return None

    def execute(self, statement: Statement) -> None:
        statement.accept(self)

        return None

    # statements
    @t.override
    def visit_expr_statement(self, statement: Expr) -> None:
        self.evaluate(statement.expression)
        return None

    @t.override
    def visit_print_statement(self, statement: Print) -> None:
        value: t.Any = self.evaluate(statement.expression)
        print(self.stringfy(value))

        return None

    @t.override
    def visit_var_statement(self, statement: Var) -> None:
        return None

    # expressions
    @t.override
    def visit_variable_expression(self, expression: Variable) -> None:
        return None

    @t.override
    def visit_literal_expression(self, expression: Literal) -> t.Any:
        return expression.value

    @t.override
    def visit_groupping_expression(self, expression: Groupping) -> t.Any:
        return self.evaluate(expression.expression)

    @t.override
    def visit_unary_expression(self, expression: Unary) -> t.Any:
        right = self.evaluate(expression.right)

        match expression.operator.type:
            case TokenType.BANG:
                return self.is_truthy(right)

            case TokenType.MINUS:
                return -1 * right

    @t.override
    def visit_binary_expression(self, expression: Binary) -> t.Any:
        left = self.evaluate(expression.left)
        right = self.evaluate(expression.right)

        def check_for_number() -> t.NoReturn | None:
            return self.check_number_operands(expression.operator, left, right)

        match expression.operator.type:
            # comparision
            case TokenType.GREATER:
                check_for_number()
                return left > right

            case TokenType.GREATER_EQUAL:
                check_for_number()
                return left >= right

            case TokenType.LESS:
                check_for_number()
                return left < right

            case TokenType.LESS_EQUAL:
                check_for_number()
                return left <= right
            # arithmetic
            case TokenType.MINUS:
                check_for_number()
                return left - right

            case TokenType.PLUS:
                check_for_number()
                return left + right

            case TokenType.PLUS_PLUS:  # concatenation operator
                if isinstance(left, str) and isinstance(right, str):
                    return "".join((left, right))

                raise GusRuntimeError(
                    expression.operator,
                    "Both operands must be string to use concatenation(++) operator",
                )

            case TokenType.STAR:
                check_for_number()
                return left * right

            case TokenType.SLASH:
                check_for_number()

                if right == 0:
                    return float("inf")

                return left / right

            # equality
            case TokenType.BANG_EQUAL:
                return not self.is_equal(left, right)

            case TokenType.EQUAL_EQUAL:
                return self.is_equal(left, right)

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

    def evaluate(self, expression: Expression) -> t.Any:
        return expression.accept(self)

    def stringfy(self, value: t.Any) -> str:
        if value is None:
            return "nihil"

        if value in (True, False):
            return str(value).lower()

        return str(value)
