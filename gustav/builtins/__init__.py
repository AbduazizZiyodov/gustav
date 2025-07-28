from gustav.builtins import clock, input_, sleep

__all__ = ("BUILTIN_FUNCTIONS",)

BUILTIN_FUNCTIONS = (clock.Clock, sleep.Sleep, input_.Input)
