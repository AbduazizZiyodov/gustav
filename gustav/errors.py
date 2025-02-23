import sys

from ._logging import log

COLORED_OUTPUT: bool

try:
    import colorama
    from colorama import Fore, Style

    colorama.init()
    COLORED_OUTPUT = True

except ImportError:
    COLORED_OUTPUT = False
    log.debug("Coloroma not found")


had_error = False


def error(
    line: int,
    message: str,
) -> None:
    report(line, "", message)


def report(line: int, where: str, message: str) -> None:
    global had_error

    if not COLORED_OUTPUT:
        print(f"[line {line}] Error {where}: {message}", file=sys.stderr)
    else:
        print(
            f"{Fore.BLUE}[line {line}]{Style.RESET_ALL} {Fore.RED}Error{Style.RESET_ALL} {where}: {message}",
            file=sys.stderr,
        )

    had_error = True
