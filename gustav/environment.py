import typing as t

from gustav.token import Token
from gustav.exceptions import GusRuntimeError

__all__ = ("Environment",)


class Environment:
    def __init__(self, enclosing: "Environment | None" = None) -> None:
        self.values: dict[str, t.Any] = dict()
        self.enclosing: "Environment | None" = enclosing

    def define(self, name: str, value: t.Any) -> None:
        self.values[name] = value
        return None

    def get(self, name: Token) -> t.Any | t.NoReturn:
        if name.lexeme in self.values:
            return self.values.get(name.lexeme)

        if self.enclosing is not None:
            return self.enclosing.get(name)

        raise GusRuntimeError(name, f"Undefined variable '{name.lexeme}'")

    def assign(self, name: Token, value: t.Any) -> None:
        if name.lexeme in self.values:
            self.values[name.lexeme] = value
            return

        if self.enclosing is not None:
            self.enclosing.assign(name, value)
            return

        raise GusRuntimeError(name, f"Undefined variable '{name.lexeme}'")

    def get_at(self, distance: int, name: str) -> t.Any:
        ancestor = self.ancestor(distance)

        if ancestor:
            return ancestor.values.get(name)

        return None

    def assign_at(self, distance: int, name: Token, value: t.Any) -> None:
        ancestor = self.ancestor(distance)

        if ancestor:
            ancestor.values[name.lexeme] = value

    def ancestor(self, distance: int) -> "Environment | None":
        environment: Environment | None = self

        for i in range(distance):
            if environment:
                environment = environment.enclosing

        return environment
