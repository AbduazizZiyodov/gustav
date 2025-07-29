# Gustav

[![CI](https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml/badge.svg)](https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml) [![codecov](https://codecov.io/github/AbduazizZiyodov/gustav/graph/badge.svg?token=LMJJLRK4OF)](https://codecov.io/github/AbduazizZiyodov/gustav)

> NOTE: readme is written by hand, not LLM generated "crap". I would appreciate if you read it. Thanks.

## About

Interpreter implemented in pure(modern btw) python for [learning](https://craftinginterpreters.com) purposes. I intend to implement features shown in "challenges" & I also have mine too.

Coming to implementation details, its a tree-walk interpreter with custom scanner, recursive decent parser (LL) & resolver for semantic analysis.

## Grammar

> [!INFO] > https://en.wikipedia.org/wiki/Wirth_syntax_notation

```shell
npm install -g ebnf2railroad
```

Checkout interactive [diagram](./assets/.html)

```
ebnf2railroad assets/.grammar
open .html
```

## User Guide

...

## TODOs

> [!NOTE]
> Some of them are the challenges introduced in the book, most of them are proposed by me (to myself).

- [ ] `REPL` support for statements and expressions
- [ ] Disallowing use of _uninitialized_ variables
- [ ] Implement `break` and `continue` statements inside loops
- [ ] Report for unused variable (during semantic analysis)
- [?] Traits
