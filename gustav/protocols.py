import typing as t

from gustav.logging import LOG  # noqa: F401
from gustav.environment import Environment
from gustav.ast import statement as S, expression as E

__all__ = "CanExecuteBlock", "CanResolveExpression", "GusCallable"


@t.runtime_checkable
class CanExecuteBlock(t.Protocol):  # pragma: no cover
    globals: Environment

    def execute_block(
        self,
        statements: list[S.Statement],
        environment: Environment,
    ) -> None: ...


@t.runtime_checkable
class CanResolveExpression(t.Protocol):  # pragma: no cover
    def resolve(self, expression: E.Expression, depth: int) -> None: ...


@t.runtime_checkable
class GusCallable(t.Protocol):  # pragma: no cover
    def arity(self) -> int: ...

    def call(
        self,
        interpreter: CanExecuteBlock,
        arguments: list[t.Any],
    ) -> t.Any: ...
