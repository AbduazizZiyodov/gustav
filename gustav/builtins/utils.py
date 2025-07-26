import typing as t

from gustav.types import GusCallable

__all__ = ("define_builtin",)


def define_builtin(
    name: str,
    arity: int,
    body: t.Callable[[t.Any, t.Any], t.Any],
) -> t.Any:
    return type(
        name,
        (GusCallable,),
        {
            "arity": lambda self: arity,
            "call": lambda self, interpreter, arguments: body(interpreter, arguments),
            "__str__": lambda self: "<native fn>",
            "__repr__": lambda self: "<native fn>",
        },
    )()
