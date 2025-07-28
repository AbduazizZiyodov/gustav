import time

from gustav.protocols import GusCallable, CanExecuteBlock

__all__ = ("Clock",)


class Clock(GusCallable):
    def arity(self) -> int:
        return 0

    def call(self, interpreter: CanExecuteBlock, arguments: list[str]) -> float:
        return time.perf_counter()

    def __repr__(self) -> str:
        return "<native fn>"
