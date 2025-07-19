from gustav.builtins import clock, input_, sleep

__all__ = ("BUILTIN_FUNCTIONS",)

BUILTIN_FUNCTIONS = (clock.fn, sleep.fn, input_.fn)
