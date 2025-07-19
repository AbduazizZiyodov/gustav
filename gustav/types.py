import typing as t
from dataclasses import dataclass, field

from gustav.token import Token
from gustav.ast import statement as S
from gustav.environment import Environment
from gustav.exceptions import GusRuntimeError, GusReturn

__all__ = ("GusCallable", "GusFunction", "GusClass", "GusClassInstance")


@t.runtime_checkable
class ImplementsExecuteBlock(t.Protocol):
    globals: Environment

    def execute_block(
        self, statements: list[S.Statement], environment: Environment
    ) -> None:
        pass


@t.runtime_checkable
class GusCallable(t.Protocol):
    def arity(self) -> int:
        pass

    def call(
        self, interpreter: ImplementsExecuteBlock, arguments: list[t.Any]
    ) -> t.Any:
        pass


class GusFunction(GusCallable):
    def __init__(
        self,
        declaration: S.Function,
        closure: Environment,
        is_initializer: bool,
    ) -> None:
        self.closure = closure
        self.declaration = declaration
        self.is_initializer = is_initializer

    @t.override
    def call(
        self, interpreter: ImplementsExecuteBlock, arguments: list[t.Any]
    ) -> t.Any:
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

    def bind(self, instance: "GusClassInstance") -> "GusFunction":
        environment: Environment = Environment(self.closure)
        environment.define("this", instance)

        return GusFunction(self.declaration, environment, self.is_initializer)

    @t.override
    def arity(self) -> int:
        return len(self.declaration.params)

    def __repr__(self) -> str:
        return f"<fn {self.declaration.name.lexeme}>"


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

    def call(
        self, interpreter: ImplementsExecuteBlock, arguments: list[t.Any]
    ) -> t.Any:
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
        return f"Instance<of '{self.klass.name}' id={id(self):x}>"
