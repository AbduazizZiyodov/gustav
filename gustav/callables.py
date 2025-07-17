import typing as t

from gustav.types import GusCallable

__all__ = ("define_builtin_fn",)


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
