from none.token import Token


class Scanner:
    def __init__(self, source: str) -> None:
        self.source = source

    def scan_tokens(self) -> list[Token]:
        return []
