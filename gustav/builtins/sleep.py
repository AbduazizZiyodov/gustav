import time
import typing as t

from gustav.builtins.utils import define_builtin

__all__ = ("fn",)


def sleep(_: t.Any, arguments: list[t.Any]) -> None:
    time.sleep(arguments[0])


fn = define_builtin("sleep", 1, sleep)
