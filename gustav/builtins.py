import time
import typing as t

from gustav.callables import define_builtin_fn

__all__ = ("BUILTINS",)


def clock(*_: t.Any) -> float:
    return time.perf_counter()


def sleep(_: t.Any, arguments: list[t.Any]) -> None:
    return time.sleep(arguments[0])


BUILTINS: tuple[tuple[str, type], ...] = (
    (
        "clock",
        define_builtin_fn(
            "clock",
            0,
            clock,
        ),
    ),
    (
        "sleep",
        define_builtin_fn(
            "sleep",
            1,
            sleep,
        ),
    ),
)
