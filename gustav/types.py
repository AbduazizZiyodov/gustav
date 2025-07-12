import typing as t
from dataclasses import dataclass

from gustav.ast import statement as S
from gustav.environment import Environment


__all__ = ("InterpreterT", "GusCallable")


@t.runtime_checkable
class InterpreterT(t.Protocol):
    globals: Environment

    def execute_block(
        self, statements: list[S.Statement], environment: Environment
    ) -> None:
        pass


@t.runtime_checkable
class GusCallable(t.Protocol):
    def arity(self) -> int:
        pass

    def call(self, interpreter: InterpreterT, arguments: list[t.Any]) -> t.Any:
        pass


@dataclass
class GusClass(GusCallable):
    name: str

    def call(self, interpreter: InterpreterT, arguments: list[t.Any]) -> t.Any:
        return GusClassInstance(self)

    def arity(self) -> int:
        return 0


@dataclass
class GusClassInstance:
    klass: GusClass

    def __repr__(self) -> str:
        return f"{self.klass.name}'s instance"
