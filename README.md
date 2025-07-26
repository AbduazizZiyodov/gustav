# Gustav

> NOTE: readme is written by hand, not LLM generated "crap". I would appreciate if you read it. Thanks.

## About

Interpreter implemented in pure(modern btw) python for [learning](https://craftinginterpreters.com) purposes. I intend to implement features shown in "challenges" & I also have mine too.

Coming to implementation details, its a tree-walk interpreter with custom scanner, recursive decent parser (LL) & resolver for semantic analysis.

## Grammar

...

## User Guide

...

## TODOs

> [!NOTE]
> List of features should added | work to be done that I assume. Some interesting challenges(to me) included also.

- [ ] Better error handling (messages)
- [x] Scanner support for C-style multiline-comments (e.g. `/* ... */`)
- [ ] Support for comma expressions
- [x] Ternary expressions (operator) support (e.g. `condition ? expr_if_true : expr_if_false` )
- [x] Pipe operator (e.g. `g(x) |> f(y)` equivalent to `f(y,g(x))`)
- [ ] Support lambdas (anonymous functions)
- [ ] REPL support for statements & expressions
- [ ] Disallowing use of uninitialized variables
- [ ] Support for `break` and `continue` statements in loops
- [x] Dividing by zero, should return infinity
- [ ] Report for unused variable
- [ ] Support for range literals
- [x] `loop` keyword, for infinite loops
- [x] Tests
