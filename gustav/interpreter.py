import typing as t

from .types import Token
from .logging import LOG
from .enums import TokenType
from .errors import runtime_error
from .exceptions import GusRuntimeError
from .ast import Expression, Binary, Groupping, Literal, Unary, Visitor


__all__ = ["Interpreter"]


class Interpreter(Visitor[object]):
    def interpret(self, expression: Expression | None) -> None:
        if not expression:
            return None

        try:
            value = self.evaluate(expression)
        except GusRuntimeError as exc:
            LOG.debug(f"Runtime error occurred: {exc=}")
            runtime_error(exc)
        else:
            LOG.debug("Printing the result of interpretation")
            result: str = self.stringfy(value)
            print(result)

    def stringfy(self, value: t.Any) -> str:
        if value is None:
            return "nihil"

        if value in (True, False):
            return str(value).lower()

        return str(value)

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

        match expression.operator.type:
            # comparision
            case TokenType.GREATER:
                self.check_number_operands(expression.operator, left, right)
                return left > right

            case TokenType.GREATER_EQUAL:
                self.check_number_operands(expression.operator, left, right)
                return left >= right

            case TokenType.LESS:
                self.check_number_operands(expression.operator, left, right)
                return left < right

            case TokenType.LESS_EQUAL:
                self.check_number_operands(expression.operator, left, right)
                return left <= right
            # arithmetic
            case TokenType.MINUS:
                self.check_number_operands(expression.operator, left, right)
                return left - right

            case TokenType.PLUS:
                self.check_number_operands(expression.operator, left, right)
                return left + right

            case TokenType.PLUS_PLUS:  # concatenation operator
                if isinstance(left, str) and isinstance(right, str):
                    return "".join((left, right))

                raise GusRuntimeError(
                    expression.operator,
                    "Both operands must be string to use concatenation(++) operator",
                )

            case TokenType.STAR:
                self.check_number_operands(expression.operator, left, right)
                return left * right

            case TokenType.SLASH:
                self.check_number_operands(expression.operator, left, right)

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
    ) -> None | t.NoReturn:
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
