import typing as t
from dataclasses import dataclass

from .enums import TokenType

__all__ = ["Token"]


@dataclass
class Token:
    type: TokenType
    lexeme: str
    literal: t.Any
    line: int

    def __str__(self) -> str:
        return f"{self.type} {self.lexeme} {self.literal}"

    def __repr__(self) -> str:
        return f"Token(type={self.type} lexeme='{self.lexeme}' literal={self.literal})"
