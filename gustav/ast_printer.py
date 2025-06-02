import os
import typing as t

from rich import print as printr

from ._token import Token
from .enums import TokenType
from .ast import Visitor, Expression, Binary, Groupping, Unary, Literal


class AstPrinter(Visitor[str]):
    def print(self, expression: Expression) -> str:
        return expression.accept(self)

    def parenthesize(self, name: str, *expressions: Expression) -> str:
        result = str()

        result += f"({name}"

        for expression in expressions:
            result += f" {expression.accept(self)}"

        result += ")"

        return result

    @t.override
    def visit_binary_expression(self, expression: Binary) -> str:
        return self.parenthesize(
            expression.operator.lexeme, expression.left, expression.right
        )

    @t.override
    def visit_groupping_expression(self, expression: Groupping) -> str:
        return self.parenthesize("group", expression.expression)

    @t.override
    def visit_literal_expression(self, expression: Literal) -> str:
        if expression.value is None:
            return "nil"
        return str(expression.value)

    @t.override
    def visit_unary_expression(self, expression: Unary) -> str:
        return self.parenthesize(expression.operator.lexeme, expression.right)


def main() -> int:
    expression = Binary(
        Unary(Token(TokenType.MINUS, "-", None, 1), Literal(123)),
        Token(TokenType.STAR, "*", None, 1),
        Groupping(Literal(45.67)),
    )

    printr(AstPrinter().print(expression))
    return os.EX_OK


if __name__ == "__main__":
    raise SystemExit(main())
