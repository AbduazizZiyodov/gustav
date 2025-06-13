import typing as t

from .types import NodeDefinitionMapping

EXPRESSION_BASE_NAME: t.Final[str] = "Expression"
STATEMENT_BASE_NAME: t.Final[str] = "Statement"


EXPRESSION_TYPES: NodeDefinitionMapping = {
    # Expressions
    "Assign": [
        {"type": "Token", "name": "name"},
        {"type": EXPRESSION_BASE_NAME, "name": "value"},
    ],
    "Binary": [
        {"type": EXPRESSION_BASE_NAME, "name": "left"},
        {"type": "Token", "name": "operator"},
        {"type": EXPRESSION_BASE_NAME, "name": "right"},
    ],
    "Groupping": [
        {"type": EXPRESSION_BASE_NAME, "name": "expression"},
    ],
    "Literal": [
        {"type": "object", "name": "value"},
    ],
    "Variable": [
        {"type": "Token", "name": "name"},
    ],
    "Unary": [
        {"type": "Token", "name": "operator"},
        {"type": EXPRESSION_BASE_NAME, "name": "right"},
    ],
}

STATEMENT_TYPES: NodeDefinitionMapping = {
    "Block": [
        {"type": f"list[{STATEMENT_BASE_NAME}]", "name": "statements"},
    ],
    "Expr": [
        {"type": EXPRESSION_BASE_NAME, "name": "expression"},
    ],
    "Print": [
        {"type": EXPRESSION_BASE_NAME, "name": "expression"},
    ],
    "Var": [
        {"type": "Token", "name": "name"},
        {"type": f"{EXPRESSION_BASE_NAME} | None", "name": "initializer"},
    ],
}
