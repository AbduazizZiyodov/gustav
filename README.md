# Gustav [^1]

<p align="center"><i>Too heavy for its own good.</i></p>

<p align="center">
<a href="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml">
  <img src="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml/badge.svg" alt="CI">
</a>
<a href="https://codecov.io/github/AbduazizZiyodov/gustav" > 
 <img src="https://codecov.io/github/AbduazizZiyodov/gustav/branch/master/graph/badge.svg?token=LMJJLRK4OF"/> 
 </a>
</p>

## About

Tree-Walk Interpreter implemented in pure(_modern btw_) CPython for experimenting/learning purposes. Based on amazing book [Crafting Interpreters](https://craftinginterpreters.com) by _Bob Nystrom_ (its cpython implementation of _Lox_).

Interpreter consists of basic components as mentioned in the book: `Scanner => Parser => Resolver => Interpreter`.

Manual/hand-written basic single-pass scanner(a.k.a lexer). Then we have classic top-down, recursive descent parser (`LL(1)` btw). Resolver is used for semantic analysis which "mostly" performs lexical scope resolution before interpretation. So there is no type checking, hoisting or any other fancy features, just lexical scope analyzer. Then, we can see tree-walk interpreter which wires all these components, uses visitor pattern and evaluates AST nodes directly.

## Future Work

Implementing "Bytecode Virtual Machine" variation in separate branch.

## Grammar

Grammar definitions are written using [Wirth Syntax Notation](https://en.wikipedia.org/wiki/Wirth_syntax_notation) instead of the more common `BNF`/`EBNF`. For this project, Wirth notation has no significant drawbacks and is fully compatible with the syntax diagram tool that I saw (see below).

<details>
<summary>See grammar</summary>

> [!TIP]  
> You can paste the grammar into [this site](https://matthijsgroen.github.io/ebnf2railroad/try-yourself.html) to view and explore it as interactive syntax diagrams.
> ![grammar_diagram_example](./assets/grammar_diagram_example.png)

```ebnf
program = { declaration } , "EOF" ;

declaration = class_declaration
            | fun_declaration
            | var_declaration
            | statement ;

class_declaration = "class" , IDENTIFIER , [ "<" , IDENTIFIER ] ,
                    "{" , { function } , "}" ;

fun_declaration = "fun" , function ;

var_declaration = "var" , IDENTIFIER , [ "=" , expression ] , ";" ;

statement = expr_statement
          | for_statement
          | if_statement
          | print_statement
          | return_statement
          | while_statement
          | loop_statement
          | block
          | break_statement
          | continue_statement ;

expr_statement = expression , ";" ;

for_statement = "for" , "(" , ( var_declaration | expr_statement | ";" ) , [ expression ] , ";" , [ expression ] , ")" ,
                    statement ;

if_statement = "if" , "(" , expression , ")" ,
                    statement ,
                [ "else" , statement ] ;

print_statement = "print" , expression , ";" ;

return_statement = "return" , [ expression ] , ";" ;

while_statement = "while" , "(" , expression , ")" ,
                    statement ;

loop_statement = "loop" , statement ;

block = "{" , { declaration } , "}" ;

break_statement = "break" , ";" ;
continue_statement = "continue" , ";" ;

expression = assignment ;

assignment = ( call , "." , IDENTIFIER , "=" , assignment )
           | logic_or ;

logic_or = logic_and , { "or" , logic_and } ;
logic_and = equality , { "and" , equality } ;

equality = comparison , { ( "!=" | "==" ) , comparison } ;
comparison = term , { ( ">" | ">=" | "<" | "<=" ) , term } ;
term = factor , { ( "-" | "+" | "++" | "^" ) , factor } ;
factor = unary , { ( "/" | "*" ) , unary } ;

unary = ( "!" | "-" ) , unary
      | call ;

call = primary , {
    ( "(" , [ arguments ] , ")" | "." , IDENTIFIER )
} , { "|>" , call } ;

primary = "true"
        | "false"
        | "nil"
        | "this"
        | NUMBER
        | STRING
        | IDENTIFIER
        | "(" , expression , ")"
        | "super" , "." , IDENTIFIER
        | lambda_expression ;

ternary = equality , [ "?" , assignment , ":" , assignment ] ;

lambda_expression = ( "lambda" | "λ" ) , "(" , [ parameters ] , ")" , block ;

function = IDENTIFIER , "(" , [ parameters ] , ")" , block ;
parameters = IDENTIFIER , { "," , IDENTIFIER } ;
arguments = expression , { "," , expression } ;

NUMBER = "number" ;
STRING = "string" ;
IDENTIFIER = "id" ;
```

</details>

## "Features" & Usage Guide

...

## Development

Requirements

- `uv`
- `just`

```shell
uv sync --dev
```

If you want to update/extend AST nodes, edit `tools/ast_generator/definitions.py` then run

```
just generate_ast
```

Run checks (formatting, linting & type-checking):

```shell
just check
```

Testing:

```shell
just test
```

[^1]: The name _Gustav_ is inspired by the [Schwerer Gustav](https://en.wikipedia.org/wiki/Schwerer_Gustav), a massive German railway gun built during WW2. The intention is not to glorify(romanticize) war or any political ideology - especially not **Nazism** - but to appreciate the engineering behind it. Current implementation (in cpython) is heavy as this railway gun.
