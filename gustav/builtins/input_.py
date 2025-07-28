import typing as t

from gustav.protocols import GusCallable, CanExecuteBlock

__all__ = ("Input",)


class Input(GusCallable):  # pragma: no cover
    def arity(self) -> int:
        return 1

    def call(self, interpreter: CanExecuteBlock, arguments: list[t.Any]) -> str:
        # TODO(abduazizziyodov): handle keyboard interrupt + mock for testing
        prompt = arguments[0]
        return input(prompt)

    def __repr__(self) -> str:
        return "<native fn>"
