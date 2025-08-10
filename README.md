<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./assets/logo_dark.svg">
  <source media="(prefers-color-scheme: light)" srcset="./assets/logo_light.svg">
  <img alt="Fallback image description" src="./assets/logo_dark.svg">
</picture>

</div>

<p align="center"><i>Too heavy for its own good.</i></p>

<p align="center">
<a href="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml">
  <img src="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml/badge.svg" alt="CI">
</a>
<a href="https://codecov.io/github/AbduazizZiyodov/gustav" >
 <img src="https://codecov.io/github/AbduazizZiyodov/gustav/branch/master/graph/badge.svg?token=LMJJLRK4OF"/>
 </a>
</p>

## About[^1]

Tree-Walk Interpreter implemented in CPython for experimenting/learning purposes. Based on amazing book [Crafting Interpreters](https://craftinginterpreters.com) by _Bob Nystrom_ (based on _Lox_).

Interpreter consists of basic components as mentioned in the book: `Scanner => Parser => Resolver => Interpreter`.

Manual/hand-written basic single-pass scanner(a.k.a lexer). Then we have classic top-down, recursive descent parser (`LL(1)` btw). Resolver is used for semantic analysis which "mostly" performs lexical scope resolution before interpretation. So there is no type checking, hoisting or any other fancy features, just lexical scope analyzer. Then, we can see tree-walk interpreter which wires all these components, uses visitor pattern and evaluates AST nodes directly.

Most of the challenges(that I think important) are implemented.

## Future Work

Implementing "Bytecode Virtual Machine" variation in separate branch.

## Grammar

Grammar definitions below are written using [Wirth Syntax Notation](https://en.wikipedia.org/wiki/Wirth_syntax_notation) instead of the more common `BNF`/`EBNF`. For this project, Wirth notation has no significant drawbacks and is fully compatible with the syntax diagram tool that I saw (see below).

> [!TIP]
> You can paste the grammar into [this site](https://matthijsgroen.github.io/ebnf2railroad/try-yourself.html) to view and explore it as interactive syntax diagrams.
> ![grammar_diagram_example](./assets/grammar_diagram_example.png)

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
  = expr_statement   | for_statement
  | if_statement     | print_statement
  | return_statement | while_statement
  | loop_statement   | block
  | break_statement  | continue_statement
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

loop_statement = "loop" , statement ;

block = "{" , { declaration } , "}" ;

break_statement = "break" , ";" ;

continue_statement = "continue" , ";" ;

expression = assignment ;

assignment
  = ( call , "." , IDENTIFIER , "=" , assignment )
  | logic_or
  ;

logic_or = logic_and , { "or" , logic_and } ;

logic_and = equality , { "and" , equality } ;

equality = comparison , { ( "!=" | "==" ) , comparison } ;

comparison = term , { ( ">" | ">=" | "<" | "<=" ) , term } ;

term = factor , { ( "-" | "+" | "++" | "^" ) , factor } ;

factor = unary , { ( "/" | "*" ) , unary } ;

unary = ( "!" | "-" ) , unary | call ;

call = primary , {
  ( "(" , [ arguments ] , ")"
  | "." , IDENTIFIER
  ) } , { "|>" , call } ;

primary
  = "true"                     | "false"
  | "nil"                      | "this"
  | NUMBER                     | STRING
  | IDENTIFIER                 | "(" , expression , ")"
  | "super" , "." , IDENTIFIER | lambda_expression
  ;

ternary = equality , [ "?" , assignment , ":" ,
  assignment ] ;

lambda_expression = ( "lambda" | "λ" ) , "(" ,
  [ parameters ] , ")" , block ;

function = IDENTIFIER , "(" , [ parameters ] , ")" , block ;

parameters = IDENTIFIER , { "," , IDENTIFIER } ;

arguments = expression , { "," , expression } ;

NUMBER = "number" ;

STRING = "string" ;

IDENTIFIER = "id" ;
```

</details>

## Syntax, Features & User Guide

> [!NOTE]
> I defined `js` as a language markdown codeblocks to get better syntax highlighting, gustav is syntactically close to JS.
> Apart from this guide below, you can also check `testing/scenarios` folder for more examples & edge-cases.

This language is dynamically typed. It supports the following primitive data types: booleans, numbers, strings, and nil. No arrays/lists, sorry.

### Arithmetic Expressions

The following arithmetic operators are supported: +, -, *, /, ^, and unary - for negation.

```js
21 + 2;    // 23
22 - 3;    // 19
32 * 4;    // 128
12 / 4;    // 3
12 / 5;    // 2.4
-12;       // -12
2^2;     // 4
```

### Comments

```js
// comment
/*
comment
*/
```

Nesting multiple comments are NOT allowed:

```js
/*
comment
  /*
  nested comment
  */
*/

// Raises an error on scanning phase
```

### Comparison and Equality

Supports standard comparison operators: <, >, <=, >=, ==, !=.

```js
12 < 2;                 // false
0 == -0.0;              // true
print "abduaziz" == "abdulaziz"; // false
```

### Logical Operators

Logical operators include and, or, and ! (not).

```js
!true;            // false
!false;           // true
true and false;   // false
true and true;    // true
false or false;   // false
true or false;    // true
```

### Print Statement

Like Python 2, print is a statement, not a function.

```js
print "Gustav!";

{
  print "Gustav: from block";
}
```

### Variables

Use the var statement to declare variables.

```js
var box = "black";
print box; // black
```

Uninitialized variables cannot be used. They do not default to nil; attempting to access one results in an error.

```js
var something;
print something; // Can't use uninitialized variable 'something'.
```

Unused variables will generate a warning (not an error) on `stderr`.

```js
fun spam() {
  var b; 
}
// [WARNING] at line 2: Variable 'b' is not used.
```

### If/Else Statement

```js
var name = "abduaziz";

if (name == "abduaziz") {
  print "yes";
} else {
  print "no";
}

// yes
```

Ternary expression

```js
(1 > 2) ? "gt" : "le";// le
```

### While Loop

`break` and `continue` statements are supported.

```js
var c = 0;
while (c < 3)
  if ((c = c + 1) == 2) continue;
  else print c;

// 1
// 3
```

### For Loop

`break` and `continue` statements are supported. `For` has separate definition as AST node, it won't be desugared into `While` statement like in the book.

```js
for (var j = 0; j < 5; j = j + 1)
  if (j == 3) break;
  else if (j == 1) continue;
  else print j;

// 0
// 2
```

### Loop Statement

Use loop for infinite loops instead of `while (true)`.

```js
loop {
  print "Gustav!";
}
// Prints "Gustav!" infinitely
```

### Functions

Functions are declared using the `fun` keyword.

```js
fun fib(n) {
  return (n < 2) ? n : fib(n - 1) + fib(n - 2);
}

print fib(8); // 21
```

Lambdas are supported and can be defined using `lambda` or `λ`.

```js
var add = λ(x) {
  return λ(y) {
    return x + y;
  };
};

print add(2)(3); // 5
```

Closures are supported.

```js
fun add_pair(a, b) {
  return a + b;
}

fun identity(a) {
  return a;
}

print identity(add_pair)(1, 2); // 3
```

Function piping is supported via the `|>` operator.

```js
fun f(x) { return x^2; }
fun g(y) { return y - 1; }

print f(2) |> g(); // 3

// Using lambdas
print λ(x) { return x^2; }(2) |> λ(y) { return y - 1; }(); // 3

fun square(n) { return n * n; }
fun dec(n) { return n - 1; }

print 4 |> dec() |> square(); // 9
```

### Classes

Below is a basic implementation of a binary search tree (BST) with in-order traversal.

```js
class Tree {
    init(value) {
        this.value = value;
        this.left = nil;
        this.right = nil;
    }

    insert(value) {
        if (value < this.value) {
            if (this.left == nil) {
                this.left = Tree(value);
            } else {
                this.left.insert(value);
            }
        } else {
            if (this.right == nil) {
                this.right = Tree(value);
            } else {
                this.right.insert(value);
            }
        }
    }

    contains(value) {
        if (this.value == value) return true;

        if (value < this.value) {
            if (this.left == nil) return false;
            return this.left.contains(value);
        } else {
            if (this.right == nil) return false;
            return this.right.contains(value);
        }
    }

    print_in_order() {
        if (this.left != nil) this.left.print_in_order();
        print this.value;
        if (this.right != nil) this.right.print_in_order();
    }
}

var tree = Tree(10);

tree.insert(5);
tree.insert(15);
tree.insert(3);
tree.insert(7);

print tree.contains(7);   // true
print tree.contains(12);  // false

tree.print_in_order();    // 3 5 7 10 15
```

**Inheritance**

Multiple inheritance is not supported. Traits may be considered as a future alternative (Feature deserves1 in VM implementation).

```js
class A {
  init(param) {
    this.field = param;
  }

  test() {
    print this.field;
  }
}

class B < A {}

var b = B("value");
b.test(); // value
```

**Super Expression**

Use super to call methods or initializers from the superclass.

```js
class A {
  say() {
    print "A";
  }
}

class B < A {
  test() {
    super.say();
  }

  say() {
    print "B";
  }
}

class C < B {
  say() {
    print "C";
  }
}

C().test(); // A
```

### Built-in Functions

The following built-in functions are provided: `clock`, `input`, and `sleep`.

```js
print clock();                     // returns float value of time in seconds, proxy to time.perfcounter from cpython
var name = input("Enter your name> "); // proxy to builtin input function from cpython
sleep(5);                          // proxy to time.sleep function from cpython
```

## Running

CPython version over `3.13` is recommended. Clone this repository, if you have `uv` run `uv sync` or use your system's "python" with `virtualenv`. Then from `gustav` module, pass your source file as an argument to run interpreter:

```shell
<python> -m gustav <file>
```

You can enable debug flag by setting `DEBUG=yes` as environment variable (it will show entire process, ast nodes, and some debug logs during runtime).

If you don't provide file as argument you will get `REPL` prompt. Which I see as "useless", because there is no support for statement/expressions (now). I wanted to implement this via `curses` library, I though its not worth, instead I wanted to explore other concepts.

## Development

Requirements

- `uv` - <https://docs.astral.sh/uv>
- `just` - <https://just.systems/man/en>

```shell
uv sync --dev
uv run pre-commit install
```

Make your changes as you wish. Use `gustav.logging.LOG` for debug logging and enable debug flag. You can think whether your change introduces new syntax/tokens, if so, make your additions in `scanner.py`. Then you need to work on `parser.py` to construct AST nodes (expressions or statements) to get something meaningful.

To update/extend AST nodes, edit `tools/ast_generator/definitions.py`(format is simple) then run

```shell
just generate_ast
```

which will generate AST files in `gustav/ast` (expressions & statements are separated). After making changes, you can move on to interpreter. Here, you can specify how your AST node should be evaluated (order of operations, checking for runtime errors etc.). Expressions do return value, statements are not.

If you want, you can do semantic analysis in `resolver.py` which is just lexical scope analyzer. But here, you can add type checkers etc.

Run checks which includes formatting, linting & type-checking:

```shell
just check
```

Run tests (with no coverage)

```shell
just test-no-cover
```

With coverage

```shell
just test
open htmlcov/index.html
```

All in one - `just`.

New tests can be added in `scenarios/` folder, most of these are taken from book's [repository](https://github.com/munificent/craftinginterpreters/tree/master/test). They have a simple format:

```js
print "hello world"; // expect: hello world
```

Testing framework looks for `// expect:` blocks, and compares these expectations with stdout & stderr outputs (combined) returned from interpreter.

[^1]: The name _Gustav_ is inspired by the [Schwerer Gustav](https://en.wikipedia.org/wiki/Schwerer_Gustav), a massive German railway gun built during WW2. The intention is not to glorify(romanticize) war or any political ideology - especially not **Nazism** - but to appreciate the engineering behind it. Current implementation (in cpython) is heavy as this railway gun.
