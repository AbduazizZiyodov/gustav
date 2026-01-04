## 1.0.0 (2026-01-04)

### Feat

- finish required data structures to work with byte code (chunk), tiny disassembler for debugging, basic logging infra
- disable use of uninit. variables, warning about not used vars, update readme
- implement break/continue statements, separate `for` as a new statement from `while`
- upload test results in codecov
- add ci, update readme & lambda tests
- implement lambda expressions, separate base ast nodes
- update grammar file
- add grammar diagram, update readme
- support loop keyword
- implement custom testing framework
- add support for inheritance, super expression
- this expression support
- class implementation (initial)
- implement ternary(operator) expressions
- semantic analysis, implement resolver
- support for "pipe" operators
- implement function/callables & closures
- update tooling (ast generator), separate expr & statements
- control flow & logical operators
- implement variables, assignments & scope
- update notes
- support for caret operator for "pow", add semicolons on repl if user forgot to add
- implement basic statements, add pipe operator to scanner, improve tooling/cli, extend ast generator
- add yes/no support
- implement string concatenation operator, handle zero division error
- implement interpreter, evaluate expressions
- recursive descent parser

### Fix

- codecov
- undefined property (on binary tree situation), re-structure whole thing, add line numbers to error reporting tool
- all bugs found from tset scenarios, remove unnecessary tests, add coverage plugin to pytest
- is_truthy and stringfy methods & error messages, update some test scenarios
- assignment must return value
- logging levels, add test repr for token (scanner tests)
- disaster in locals dict, introducted by Token (eq, hash stuff) & refactor
- disable nesting multiline comments, check for unterminated block comments
- parsing argument expr
- import at ast generator main
- update old readme

### Refactor

- ast generator
- yes
- exceptions
- builtins
- interpreter, bin op
- renaming parser's methods
- ast generator, incorrect __all__
- cli main, import alias for f-ng token type
- organize modules & imports
- logging and error handling
