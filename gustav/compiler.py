import math
import sys
import typing as t
from dataclasses import dataclass

from gustav.ast import Expression, Statement
from gustav.ast import expression as E
from gustav.ast import statement as S
from gustav.errors import GusParseError, GusRuntimeError, runtime_error
from gustav.token import Token
from gustav.token import TokenType as TT


@dataclass(slots=True, frozen=True)
class Compiler(E.ExpressionVisitor[t.Any], S.StatementVisitor[None]):
    """This compiler's job is to do AST -> SSA."""

    statements: list[Statement]

    def do(self) -> None:
        try:
            for statement in self.statements:
                self.execute(statement)

        except GusRuntimeError as exc:
            runtime_error(exc)

    ###
    # Statements
    ###

    @t.override
    def visit_break_statement(self, statement: S.Break) -> t.Never:
        raise GusParseError()

    @t.override
    def visit_continue_statement(self, statement: S.Continue) -> None: ...

    @t.override
    def visit_while_statement(self, statement: S.While) -> None: ...

    @t.override
    def visit_for_statement(self, statement: S.For) -> None: ...

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

    @t.override
    def visit_expr_statement(self, statement: S.Expr) -> None:
        self.evaluate(statement.expression)

    @t.override
    def visit_print_statement(self, statement: S.Print) -> None:
        value: t.Any = self.evaluate(statement.expression)
        line: str = self.stringify(value) + "\n"

        sys.stdout.write(line)

    @t.override
    def visit_var_statement(self, statement: S.Var) -> None:
        self.evaluate(statement.initializer)
        # value = self.evaluate(statement.initializer)
        # self.environment.define(statement.name.lexeme, value)

    @t.override
    def visit_block_statement(self, statement: S.Block) -> None: ...

    ###
    # Expressions
    ###

    @t.override
    def visit_ternary_expression(self, expression: E.Ternary) -> t.Any:
        condition_eval = self.evaluate(expression.condition)

        if self.is_truthy(condition_eval):
            return self.evaluate(expression.then_branch)

        return self.evaluate(expression.else_branch)

    @t.override
    def visit_assign_expression(self, expression: E.Assign) -> t.Any:
        # value: t.Any = self.evaluate(expression.value)

        # distance: int | None = self.locals.get(expression)

        # if distance is not None:
        #     self.environment.assign_at(distance, expression.name, value)
        # else:
        #     self.globals.assign(expression.name, value)

        # return value
        ...

    @t.override
    def visit_variable_expression(self, expression: E.Variable) -> t.Any:
        # value = self.look_up_variable(expression.name, expression)

        # if value is UNINITIALIZED:
        #     raise GusRuntimeError(
        #         expression.name,
        #         f"Can't use uninitialized variable '{expression.name.lexeme}'",
        #     )

        # return value
        ...

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

            # partial coverage
            case TT.MINUS:  # pragma: no cover
                self.check_number_operand(expression.operator, right)
                return -1 * right

    NUMERIC_OPERATORS: t.ClassVar[tuple[TT, ...]] = (
        TT.GREATER,
        TT.GREATER_EQUAL,
        TT.LESS,
        TT.LESS_EQUAL,
        TT.MINUS,
        TT.STAR,
        TT.SLASH,
    )

    @t.override
    def visit_binary_expression(self, expression: E.Binary) -> t.Any:
        left = self.evaluate(expression.left)
        right = self.evaluate(expression.right)

        if expression.operator.type in Compiler.NUMERIC_OPERATORS:
            self.check_number_operands(expression.operator, left, right)

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
                if any(
                    isinstance(left, T) and isinstance(right, T) for T in (int, float)
                ):
                    return left + right

                raise GusRuntimeError(
                    expression.operator, "Operands must be two numbers or two strings"
                )

            case TT.STAR:
                return left * right

            case TT.SLASH:
                if right == 0:
                    # NOTE(abduazizziyodov): can be inf
                    return float("nan")

                return left / right

            case TT.BANG_EQUAL:
                return not self.is_equal(left, right)

            case TT.EQUAL_EQUAL:
                return self.is_equal(left, right)

            case _:  # pragma: no cover
                # should not be possible, anyway
                raise GusRuntimeError(expression.operator, "Unknown binary operator.")

    ###
    # Machinery
    ###

    def execute(self, statement: Statement) -> None:
        statement.accept(self)

    def evaluate(self, expression: Expression) -> t.Any:
        return expression.accept(self)

    def check_number_operand(self, operator: Token, operand: t.Any) -> None:
        if isinstance(operand, (int, float)):
            return

        raise GusRuntimeError(operator, "Operand must be a number")

    def check_number_operands(self, operator: Token, *operands: t.Any) -> None:
        if all(isinstance(operand, (int, float)) for operand in operands):
            return

        raise GusRuntimeError(operator, "Operands must be numbers")

    def is_equal(self, a: t.Any, b: t.Any) -> bool:
        if type(a) is type(b):
            return bool(a == b)
        return False

    def is_truthy(self, value: t.Any) -> bool:
        return not (value is None or value is False)

    def stringify(self, value: t.Any) -> str:
        if value is None:
            return "nil"

        if isinstance(value, bool):
            return str(value).lower()

        if isinstance(value, float):
            # NOTE(abduazizziyodov): special to handle -0.0
            if value == 0.0 and math.copysign(1.0, value) < 0:
                return "-0"

            if value.is_integer():
                return str(int(value))
            return str(value)

        return str(value)
