import typing as t
from dataclasses import dataclass, field

from gustav.token import Token
from gustav.ast import statement as S
from gustav.environment import Environment
from gustav.exceptions import GusRuntimeError, GusReturn

__all__ = ("InterpreterT", "GusCallable", "GusFunction", "GusClass", "GusClassInstance")


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


class GusFunction(GusCallable):
    def __init__(self, declaration: S.Function, closure: Environment) -> None:
        self.closure = closure
        self.declaration = declaration

    @t.override
    def call(self, interpreter: InterpreterT, arguments: list[t.Any]) -> t.Any:
        environment: Environment = Environment(self.closure)

        for i in range(self.arity()):
            environment.define(self.declaration.params[i].lexeme, arguments[i])

        try:
            interpreter.execute_block(self.declaration.body, environment)
        except GusReturn as exc:
            return exc.value

        return None

    def bind(self, instance: "GusClassInstance") -> "GusFunction":
        environment: Environment = Environment(self.closure)
        environment.define("this", instance)
        return GusFunction(self.declaration, environment)

    @t.override
    def arity(self) -> int:
        return len(self.declaration.params)

    def __repr__(self) -> str:
        return f"<fn {self.declaration.name.lexeme}>"


@dataclass
class GusClass(GusCallable):
    name: str
    methods: dict[str, GusFunction]

    def find_method(self, name: str) -> GusFunction | None:
        return self.methods.get(name)

    def call(self, interpreter: InterpreterT, arguments: list[t.Any]) -> t.Any:
        return GusClassInstance(self)

    def arity(self) -> int:
        return 0


@dataclass
class GusClassInstance:
    klass: GusClass
    fields: dict[str, t.Any] = field(default_factory=dict)

    def get(self, name: Token) -> GusFunction | t.Any:
        if val := self.fields.get(name.lexeme):
            return val

        method: GusFunction | None

        if (method := self.klass.find_method(name.lexeme)) is not None:
            return method.bind(self)

        raise GusRuntimeError(name, f"Undefined property '{name.lexeme}'.")

    def set(self, name: Token, value: t.Any) -> None:
        self.fields[name.lexeme] = value

    def __repr__(self) -> str:
        return f"Instance<of '{self.klass.name}' {id(self)}>"
