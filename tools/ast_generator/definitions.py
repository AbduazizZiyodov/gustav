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
    "Call": [
        {"type": EXPRESSION_BASE_NAME, "name": "callee"},
        {"type": "Token", "name": "paren"},
        {"type": f"list[{EXPRESSION_BASE_NAME}]", "name": "arguments"},
    ],
    "Groupping": [
        {"type": EXPRESSION_BASE_NAME, "name": "expression"},
    ],
    "Literal": [
        {"type": "object", "name": "value"},
    ],
    "Logical": [
        {"type": EXPRESSION_BASE_NAME, "name": "left"},
        {"type": "Token", "name": "operator"},
        {"type": EXPRESSION_BASE_NAME, "name": "right"},
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
    "If": [
        {"type": EXPRESSION_BASE_NAME, "name": "condition"},
        {"type": STATEMENT_BASE_NAME, "name": "then_branch"},
        {"type": f"{STATEMENT_BASE_NAME} | None", "name": "else_branch"},
    ],
    "Function": [
        {"type": "Token", "name": "name"},
        {"type": "list[Token]", "name": "params"},
        {"type": f"list[{STATEMENT_BASE_NAME}]", "name": "body"},
    ],
    "Print": [
        {"type": EXPRESSION_BASE_NAME, "name": "expression"},
    ],
    "Return": [
        {"type": "Token", "name": "keyword"},
        {"type": f"{EXPRESSION_BASE_NAME} | None", "name": "value"},
    ],
    "While": [
        {"type": EXPRESSION_BASE_NAME, "name": "condition"},
        {"type": STATEMENT_BASE_NAME, "name": "body"},
    ],
    "Var": [
        {"type": "Token", "name": "name"},
        {"type": f"{EXPRESSION_BASE_NAME} | None", "name": "initializer"},
    ],
}
