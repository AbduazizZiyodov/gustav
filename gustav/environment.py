import typing as t

from gustav.token import Token
from gustav.logging import LOG  # noqa: F401
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
            return self.enclosing.get(name)  # pragma: no cover

        raise GusRuntimeError(name, f"Undefined variable '{name.lexeme}'")

    def assign(self, name: Token, value: t.Any) -> None:
        if name.lexeme in self.values:
            self.values[name.lexeme] = value
            return

        if self.enclosing is not None:  # pragma: no cover
            self.enclosing.assign(name, value)
            return

        raise GusRuntimeError(name, f"Undefined variable '{name.lexeme}'")

    def get_at(self, distance: int, name: str) -> t.Any:
        # partial coverage
        if (ancestor := self.ancestor(distance)) is not None:  # pragma: no cover
            return ancestor.values.get(name)

    def assign_at(self, distance: int, name: Token, value: t.Any) -> None:
        # partial coverage
        if (ancestor := self.ancestor(distance)) is not None:  # pragma: no cover
            ancestor.values[name.lexeme] = value

    def ancestor(self, distance: int) -> "Environment | None":
        environment: Environment | None = self

        for _ in range(distance):
            environment = environment.enclosing if environment else environment

        return environment
