# AST nodes definition
# Later, will be used by generator script to create Python representation of these nodes

EXPRESSION_TYPES = {
    "Assign": "name: Token, value: Expression",
    "Binary": "left: Expression, operator: Token, right: Expression",
    "Ternary": "condition: Expression, then_branch: Expression, else_branch: Expression",
    "Call": "callee: Expression, paren: Token, arguments: list[Expression]",
    "Get": "object: Expression, name: Token",
    "Set": "object: Expression, name: Token, value: Expression",
    "This": "keyword: Token",
    "Groupping": "expression: Expression",
    "Literal": "value: object",
    "Logical": "left: Expression, operator: Token, right: Expression",
    "Variable": "name: Token",
    "Unary": "operator: Token, right: Expression",
    "Super": "keyword: Token, method: Token",
}

STATEMENT_TYPES = {
    "Block": "statements: list[Statement]",
    "Class": "name: Token, superclass: E.Variable | None, methods: list['Function']",
    "Expr": "expression: E.Expression",
    "If": "condition: E.Expression, then_branch: Statement, else_branch: Statement | None",
    "Function": "name: Token, params: list[Token], body: list[Statement]",
    "Print": "expression: E.Expression",
    "Return": "keyword: Token, value: E.Expression | None",
    "While": "condition: E.Expression, body: Statement",
    "Var": "name: Token, initializer: E.Expression | None",
}
