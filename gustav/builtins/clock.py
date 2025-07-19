import time
import typing as t

from gustav.builtins.utils import define_builtin

__all__ = ("fn",)


def clock(*_: t.Any) -> float:
    return time.perf_counter()


fn = define_builtin("clock", 0, clock)
