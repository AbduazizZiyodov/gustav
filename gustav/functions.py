import typing as t
from dataclasses import dataclass

from gustav.logging import LOG  # noqa: F401
from gustav.exceptions import GusReturn
from gustav.environment import Environment
from gustav.ast import statement as S, expression as E
from gustav.protocols import GusCallable, CanExecuteBlock

__all__ = "GusFunction", "GusLambda"


@dataclass(slots=True, frozen=True, eq=False)
class GusFunction(GusCallable):
    declaration: S.Function
    closure: Environment
    is_initializer: bool

    @t.override
    def call(self, interpreter: CanExecuteBlock, arguments: list[t.Any]) -> t.Any:
        environment: Environment = Environment(self.closure)

        for i in range(self.arity()):
            environment.define(self.declaration.params[i].lexeme, arguments[i])

        try:
            interpreter.execute_block(self.declaration.body, environment)
        except GusReturn as exc:
            # NOTE (abduazizziyodov): disallow return statement from "init" ?!
            return (
                exc.value if not self.is_initializer else self.closure.get_at(0, "this")
            )

        if self.is_initializer:
            return self.closure.get_at(0, "this")

        return None

    def bind(self, instance: t.Any) -> "GusFunction":
        # NOTE(abduazizziyodov): instance is in GusClassInstance type
        environment: Environment = Environment(self.closure)
        environment.define("this", instance)

        return GusFunction(self.declaration, environment, self.is_initializer)

    @t.override
    def arity(self) -> int:
        return len(self.declaration.params)

    def __repr__(self) -> str:
        return f"<fn {self.declaration.name.lexeme}>"


@dataclass(slots=True, frozen=True, eq=False)
class GusLambda(GusCallable):
    declaration: E.Lambda
    closure: Environment

    @t.override
    def call(self, interpreter: CanExecuteBlock, arguments: list[t.Any]) -> t.Any:
        environment: Environment = Environment(self.closure)

        for i in range(self.arity()):
            environment.define(self.declaration.params[i].lexeme, arguments[i])

        try:
            interpreter.execute_block(self.declaration.body, environment)
        except GusReturn as exc:
            return exc.value

        return None

    @t.override
    def arity(self) -> int:
        return len(self.declaration.params)

    def __repr__(self) -> str:
        return "<λ fn>"
