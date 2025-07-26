import typing as t

from gustav.builtins.utils import define_builtin

__all__ = ("fn",)


def _input(_: t.Any, arguments: list[t.Any]) -> str:  # pragma: no cover
    # TODO(abduazizziyodov): handle keyboard interrupt + mock for testing
    prompt = arguments[0]
    return input(prompt)


fn = define_builtin("input", 1, _input)
