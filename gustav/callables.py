import typing as t

from gustav.ast import statement as S
from gustav.exceptions import GusReturn
from gustav.environment import Environment
from gustav.types import GusCallable, InterpreterT

__all__ = "GusFunction", "define_builtin_fn"


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

    @t.override
    def arity(self) -> int:
        return len(self.declaration.params)

    def __repr__(self) -> str:
        return f"<fn {self.declaration.name.lexeme}>"


def define_builtin_fn(
    name: str,
    arity: int,
    body: t.Callable[[t.Any, t.Any], t.Any],
) -> type:
    return type(
        name,
        (GusCallable,),
        {
            "arity": lambda: arity,
            "call": lambda interpreter, arguments: body(interpreter, arguments),
            "__repr__": "<native fn>",
        },
    )
