# Gustav

> NOTE: readme is written by hand, not LLM generated "crap". I would appreciate if you read it. Thanks.

## About

Interpreter implemented in pure(modern btw) python for [learning](https://craftinginterpreters.com) purposes. I intend to implement features shown in "challenges" & I also have mine too.

Coming to implementation details, its a tree-walk interpreter with custom scanner, recursive decent parser (LL) & resolver for semantic analysis.

## Grammar

> [!INFO] 
> https://en.wikipedia.org/wiki/Wirth_syntax_notation

```shell
npm install -g ebnf2railroad
```

Checkout interactive [diagram](./assets/grammar.html)

```
ebnf2railroad assets/.grammar
open .html
```

## User Guide

...

## TODOs

> [!NOTE]
> Some of them are the challenges introduced in the book, most of them are proposed by me (to myself).

- [ ] Better error handling messages
- [x] Scanner support for C-style multiline-comments (e.g. `/* ... */`)
- [?] Comma expressions
- [x] Ternary expressions (operator) support (e.g. `condition ? expr_if_true : expr_if_false` )
- [x] Pipe operator (e.g. `g(x) |> f(y)` <=> `f(y,g(x))`)
- [ ] Support lambdas (a.k.a. anonymous functions)
- [ ] `REPL` support for statements and expressions
- [ ] Disallowing use of _uninitialized_ variables
- [ ] Implement `break` and `continue` statements inside loops
- [ ] Report for unused variable (during semantic analysis)
- [ ] Support for range literals (e.g. `1..5` <=> `[1,2,3,4,5]`)
- [x] `loop` keyword (for infinite loops, `loop { ... }` <=> `while (true) {...}`)
- [?] Traits
- [x] Testing framework & test scenarios/cases
