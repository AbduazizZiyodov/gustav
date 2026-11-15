import typing as t
from dataclasses import dataclass

from gustav.enums import TokenType as TT


@dataclass(frozen=True, slots=True, eq=False)
class Token:
    type: TT
    lexeme: str
    literal: t.Any
    line: int
