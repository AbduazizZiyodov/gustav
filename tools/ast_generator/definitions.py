# AST nodes definition
# Later, will be used by generator script to create Python representation of these nodes

EXPRESSION_TYPES = {
    "Assign": "name: Token, value: Expression",
    "Binary": "left: Expression, operator: Token, right: Expression",
    "Ternary": "condition: Expression, then_branch: Expression, else_branch: Expression",
    "Groupping": "expression: Expression",
    "Literal": "value: object",
    "Logical": "left: Expression, operator: Token, right: Expression",
    "Variable": "name: Token",
    "Unary": "operator: Token, right: Expression",
}

STATEMENT_TYPES = {
    "Block": "statements: list[Statement]",
    "Expr": "expression: Expression",
    "If": "condition: Expression, then_branch: Statement, else_branch: Statement | None",
    "Print": "expression: Expression",
    "While": "condition: Expression, body: Statement",
    "Var": "name: Token, initializer: Expression",
    "For": "initializer: Var | Expr | None, condition: Expression | E.Literal, increment: Expr | None, body: Statement",
    "Break": "keyword: Token",
    "Continue": "keyword: Token",
}
