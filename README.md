# Gustav

[![CI](https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml/badge.svg)](https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml) [![codecov](https://codecov.io/github/AbduazizZiyodov/gustav/graph/badge.svg?token=LMJJLRK4OF)](https://codecov.io/github/AbduazizZiyodov/gustav)

> NOTE: readme is written by hand, not LLM generated "crap". I would appreciate if you read it. Thanks.

## About

Interpreter implemented in pure(modern btw) python for [learning](https://craftinginterpreters.com) purposes. I intend to implement features shown in "challenges" & I also have mine too.

Coming to implementation details, its a tree-walk interpreter with custom scanner, recursive decent parser (LL) & resolver for semantic analysis.

## Grammar

> [!TIP]
> Its written in [Wirth Syntax Notation](https://en.wikipedia.org/wiki/Wirth_syntax_notation) instead of standard `EBNF`.
> Understandable notation, also you can paste it directly into [this website](https://matthijsgroen.github.io/ebnf2railroad/try-yourself.html) to explore it interactively.

<details>
<summary>See grammar</summary>

```ebnf
program = { declaration } , "EOF" ;

declaration
= class_declaration
| fun_declaration
| var_declaration
| statement
;

class_declaration = "class" , IDENTIFIER ,
[ "<" , IDENTIFIER ] , "{" , { function } , "}" ;

fun_declaration = "fun" , function ;

var_declaration = "var" , IDENTIFIER , [ "=" ,
expression ] , ";" ;

statement
= expr_statement | for_statement | if_statement
| print_statement | return_statement | while_statement
| block
;

expr_statement = expression , ";" ;

for_statement = "for" , "(" , ( var_declaration | expr_statement | ";" ) ,
[ expression ] , ";" , [ expression ] , ")" ,
statement ;

if_statement = "if" , "(" , expression , ")" ,
statement , [ "else" , statement ] ;

print_statement = "print" , expression , ";" ;

return_statement = "return" , [ expression ] , ";" ;

while_statement = "while" , "(" , expression , ")" ,
statement ;

block = "{" , { declaration } , "}" ;

expression = assignment | assignment , "|>" , call ;

assignment
= [ call , "." ] , IDENTIFIER , "=" , assignment
| logic_or
;

logic_or = logic_and , { "or" , logic_and } ;

logic_and = equality , { "and" , equality } ;

equality = comparison , { ( "!=" | "==" ) , comparison } ;

comparison = term , { ( ">" | ">=" | "<" | "<=" ) , term } ;

term = factor , { ( "-" | "+" ) , factor } ;

factor = unary , { ( "/" | "\*" ) , unary } ;

unary = ( "!" | "-" ) , unary | call ;

call = primary , {
( "(" , [ arguments ] , ")"
| "." , IDENTIFIER
) } ;

primary
= "true" | "false"
| "nil" | "this"
| NUMBER | STRING
| IDENTIFIER | "(" , expression , ")"
| "super" , "." , IDENTIFIER | lambda_expression
;

lambda_expression = ( "lambda" | "λ" ) , "(" ,
[ parameters ] , ")" , block ;

function = IDENTIFIER , "(" , [ parameters ] , ")" , block ;

parameters = IDENTIFIER , { "," , IDENTIFIER } ;

arguments = expression , { "," , expression } ;

(_ Lexical rules _)

NUMBER = "number" ;

STRING = "string" ;

IDENTIFIER = "id" ;
```

</details>

## Guide

...

## TODOs

> [!NOTE]
> Some of them are the challenges introduced in the book, most of them are proposed by me (to myself).

- [ ] `REPL` support for statements and expressions
- [ ] Disallowing use of _uninitialized_ variables
- [ ] Implement `break` and `continue` statements inside loops
- [ ] Report for unused variable (during semantic analysis)
- [?] Traits
