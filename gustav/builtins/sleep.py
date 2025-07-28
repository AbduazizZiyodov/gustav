import time
import typing as t

from gustav.protocols import GusCallable, CanExecuteBlock

__all__ = ("Sleep",)


class Sleep(GusCallable):
    def arity(self) -> int:
        return 1

    def call(self, interpreter: CanExecuteBlock, arguments: list[t.Any]) -> None:
        timeout = arguments[0]
        time.sleep(timeout)

    def __repr__(self) -> str:
        return "<native fn>"
