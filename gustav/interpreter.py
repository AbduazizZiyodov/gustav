import typing as t

from .logging import LOG  # noqa: F401
from gustav import gustav
from .exceptions import GusRuntimeError
from .token import Token, TokenType as TT
from .ast import (
    Expression,
    Binary,
    Groupping,
    Literal,
    Variable,
    Logical,
    Unary,
    Assign,
    ExpressionVisitor,
    Statement,
    Expr,
    While,
    Print,
    Var,
    Block,
    If,
    StatementVisitor,
)
from .environment import Environment

__all__ = ["Interpreter"]


class Interpreter(ExpressionVisitor[t.Any], StatementVisitor[None]):
    def __init__(self) -> None:
        super().__init__()
        self.environment: Environment = Environment()

    def interpret(self, statements: list[Statement]) -> None:
        try:
            for statement in statements:
                self.execute(statement)

        except GusRuntimeError as exc:
            gustav.runtime_error(exc)

        return None

    def execute(self, statement: Statement) -> None:
        statement.accept(self)
        return None

    @t.override
    def visit_while_statement(self, statement: While) -> None:
        while self.is_truthy(self.evaluate(statement.condition)):
            self.execute(statement.body)

    @t.override
    def visit_logical_expression(self, expression: Logical) -> t.Any:
        left: t.Any = self.evaluate(expression.left)

        if expression.operator.type == TT.OR:
            if self.is_truthy(left):
                return left
        else:
            if not self.is_truthy(left):
                return left

        return self.evaluate(expression.right)

    @t.override
    def visit_if_statement(self, statement: If) -> None:
        if self.is_truthy(self.evaluate(statement.condition)):
            self.execute(statement.then_branch)

        elif statement.else_branch is not None:
            self.execute(statement.else_branch)

        return None

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
        value: t.Any = None

        if statement.initializer is not None:
            value = self.evaluate(statement.initializer)

        self.environment.define(statement.name.lexeme, value)

    @t.override
    def visit_block_statement(self, statement: Block) -> None:
        self.execute_block(
            statement.statements,
            Environment(self.environment),
        )

        return None

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

    # expressions
    @t.override
    def visit_assign_expression(self, expression: Assign) -> t.Any:
        value: t.Any = self.evaluate(expression.value)
        self.environment.assign(expression.name, value)

        return value

    @t.override
    def visit_variable_expression(self, expression: Variable) -> t.Any:
        return self.environment.get(expression.name)

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
            case TT.BANG:
                return not self.is_truthy(right)

            case TT.MINUS:
                return -1 * right

    @t.override
    def visit_binary_expression(self, expression: Binary) -> t.Any:
        left = self.evaluate(expression.left)
        right = self.evaluate(expression.right)

        def check_for_number() -> t.NoReturn | None:
            return self.check_number_operands(expression.operator, left, right)

        match expression.operator.type:
            case TT.GREATER:
                check_for_number()
                return left > right

            case TT.GREATER_EQUAL:
                check_for_number()
                return left >= right

            case TT.LESS:
                check_for_number()
                return left < right

            case TT.LESS_EQUAL:
                check_for_number()
                return left <= right

            case TT.MINUS:
                check_for_number()
                return left - right

            case TT.PLUS:
                check_for_number()
                return left + right

            case TT.PLUS_PLUS:  # concatenation operator
                if isinstance(left, str) and isinstance(right, str):
                    return "".join((left, right))

                raise GusRuntimeError(
                    expression.operator,
                    "Both operands must be string to use concatenation(++) operator",
                )

            case TT.STAR:
                check_for_number()
                return left * right

            case TT.SLASH:
                check_for_number()

                if right == 0:
                    return float("inf")

                return left / right

            # equality
            case TT.BANG_EQUAL:
                return not self.is_equal(left, right)

            case TT.EQUAL_EQUAL:
                return self.is_equal(left, right)

            case TT.CARET:
                return pow(left, right)

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
