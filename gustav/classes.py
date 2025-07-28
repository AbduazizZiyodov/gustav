import typing as t
from dataclasses import dataclass, field

from gustav.logging import LOG  # noqa: F401
from gustav.token import Token
from gustav.functions import GusFunction
from gustav.exceptions import GusRuntimeError
from gustav.protocols import GusCallable, CanExecuteBlock

__all__ = "GusClass", "GusClassInstance"


@dataclass
class GusClass(GusCallable):
    name: str
    superclass: "GusClass"
    methods: dict[str, GusFunction]

    def find_method(self, name: str) -> GusFunction | None:
        if name in self.methods:
            return self.methods.get(name)

        if self.superclass is not None:
            return self.superclass.find_method(name)

    def call(self, interpreter: CanExecuteBlock, arguments: list[t.Any]) -> t.Any:
        # Creates instance of class: var instance = Klass()

        instance = GusClassInstance(self)

        if self.initializer is not None:
            self.initializer.bind(instance).call(interpreter, arguments)

        return instance

    @property
    def initializer(self) -> GusFunction | None:
        return self.find_method("init")

    def arity(self) -> int:
        return 0 if self.initializer is None else self.initializer.arity()

    def __repr__(self) -> str:
        return self.name


@dataclass
class GusClassInstance:
    klass: GusClass
    fields: dict[str, t.Any] = field(default_factory=dict)

    def get(self, name: Token) -> GusFunction | t.Any:
        if name.lexeme in self.fields:
            return self.fields.get(name.lexeme)

        method: GusFunction | None

        if (method := self.klass.find_method(name.lexeme)) is not None:
            return method.bind(self)

        raise GusRuntimeError(name, f"Undefined property '{name.lexeme}'")

    def set(self, name: Token, value: t.Any) -> None:
        self.fields[name.lexeme] = value

    def __repr__(self) -> str:
        return f"{self.klass.name} instance"
