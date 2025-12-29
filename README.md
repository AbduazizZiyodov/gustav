<div align="center">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./assets/logo_dark.svg">
  <source media="(prefers-color-scheme: light)" srcset="./assets/logo_light.svg">
  <img alt="Fallback image description" src="./assets/logo_light.svg">
</picture>

</div>

<p align="center"><i>Too heavy for its own good.</i></p>

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://abduaziz.ziyodov.uz/badges/ai-free-dark.svg">
    <source media="(prefers-color-scheme: light)" srcset="https://abduaziz.ziyodov.uz/badges/ai-free-light.svg">
    <img alt="bruh" src="https://abduaziz.ziyodov.uz/badges/ai-free-dark.svg">
  </picture>
<a href="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml">
  <img src="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml/badge.svg" alt="CI">
</a>
<a href="https://codecov.io/github/AbduazizZiyodov/gustav" >
 <img src="https://codecov.io/github/AbduazizZiyodov/gustav/branch/master/graph/badge.svg?token=LMJJLRK4OF" alt="codecov"/>
 </a>
</p>

## About[^1]

Tree-Walk Interpreter implemented in cpython for experimenting/learning purposes. Based on amazing book [Crafting Interpreters](https://craftinginterpreters.com) by _Bob Nystrom_ (based on _Lox_).

Interpreter consists of basic components as mentioned in the book:

`Scanner => Parser => Resolver => Interpreter`

* Basic single-pass scanner(a.k.a lexer) and we have classic top-down recursive descent parser (`LL(1)`).
* Resolver is used for semantic analysis which "mostly" performs lexical scope resolution before interpretation. So there is no type checking, hoisting or any other fancy features, just lexical scope analyzer.
* Then, we can see tree-walk interpreter which wires all these components, evaluates AST nodes directly(visitor pattern).

Most of the challenges that I think important to me were implemented. Take a look at source code, maybe you'll learn something. You can even open an issue in case of questions & problems that you've found.

## Future Work

Implementing "Bytecode Virtual Machine" variation in separate branch(`vm` branch).

## Syntax, Features & User Guide

> [!NOTE]
> Apart from this guide below, you can also check `testing/scenarios` folder for more examples.

Language is dynamically typed. It supports the following primitive data types: booleans, numbers, strings, and nil. No arrays/lists (sorry).

### Arithmetic Expressions

The following arithmetic operators are supported: `+`, `-`, `*`, `/`, `^` (negating is also supported).

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

Nesting multiple multiline comments are NOT allowed:

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

Supports standard comparison operators: `<`, `>`, `<=`, `>=`, `==`, `!=`.

```js
12 < 2;                 // false
0 == -0.0;              // true
print "abduaziz" == "abdulaziz"; // false
```

### Logical Operators

Logical operators include `and`, `or`, and `!` (not). `yes/no` maps into `true/false`.

```js
!true;            // false
!false;           // true
true and false;   // false
true and true;    // true
false or false;   // false
true or false;    // true
yes == true;      // true
```

### Print Statement

Like cpython 2, print is a statement, not a function.

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

Uninitialized variables **cannot be** used. They do not default to nil; attempting to access one results in an error.

```js
var something;
print something; // Can't use uninitialized variable 'something'.
```

Unused variables will generate a warning message (not an error) on `stderr`.

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

> [!NOTE]
> Both `break` and `continue` statements are supported in `for`, `while` & `loop` statements.

```js
var c = 0;
while (c < 3)
  if ((c = c + 1) == 2) continue;
  else print c;

// 1
// 3
```

### For Loop

```js
for (var j = 0; j < 5; j = j + 1)
  if (j == 3) break;
  else if (j == 1) continue;
  else print j;

// 0
// 2
```

Design note: `For` has separate definition as AST node, it won't be desugared into `While` statement like shown in the book.

### Loop Statement

Use `loop` for defining infinite loops instead of `while (true)`.

```js
var name;
loop {
  name = input("Are you male or female?> ");

  if (name == "email") {
    print "nice!";
    break;
  } else {
    print "nope";
  }
}

// Are you male or female?> male
// nope
// Are you male or female?> gigachad
// nope
// Are you male or female?> abduaziz
// nope
// Are you male or female?> email
// nice!

// https://youtu.be/kHCEMzsulN4?t=7
```

### Functions

Functions are declared using the `fun` keyword.

```js
fun fib(n) {
  return (n < 2) ? n : fib(n - 1) + fib(n - 2);
}

print fib(8); // 21
```

Lambdas are supported(closures too) and can be defined using `lambda` or `λ`.

```js
var add = λ(x) { // lambda(x) {} is also fine
  return λ(y) {
    return x + y;
  };
};

print add(2)(3); // 5
```

```js
fun fibonacci() {
    var a = 0; var b = 1;

    return λ() {
        var old_a = a;
        a = b; b = old_a + b;
        return old_a;
    };
}

var fibo = fibonacci();

for (
    var iter = 0;
    iter <= 21;
    iter = iter + 1
)
    print fibo();

// 0
// 1
// 1
// ...
// 4181
// 6765
// 10946
```

Function piping is supported via the `|>` operator.

```js
fun f(x) { return x^2; }
fun g(y) { return y - 1; }

print f(2) |> g(); // 3

// using lambdas
print λ(x) { return x^2; }(2) |> λ(y) { return y - 1; }(); // 3

fun square(n) { return n * n; }
fun dec(n) { return n - 1; }

print 4 |> dec() |> square(); // 9
```

However, there is one known bug :)

### Classes

Below is a basic implementation of BST.

```js
class Tree {
    init(value) {
        this.value = value;
        this.left = this.right = nil;
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
}

fun binary_search(node, value) {
    if (node == nil)
        return false;

    if (node.value == value)
        return true;

    if (value < node.value)
        return binary_search(node.left, value);

    return binary_search(node.right, value);
}

var root = Tree(10);

root.insert(5); root.insert(15); root.insert(3); root.insert(7);

print binary_search(root, 7);   // true
print binary_search(root, 15);  // true
print binary_search(root, 12);  // false
print binary_search(root, 5);   // true
```

#### Inheritance

Multiple inheritance is **not** supported. Traits may be considered as a future alternative (this feature deserves in VM implementation). Use `<` for inheriting.

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

#### Super Expression

Use `super` to call methods or initializers from the superclass.

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
print clock();                         // returns float value of time in seconds, proxy to time.perfcounter from cpython
var name = input("Enter your name> "); // proxy to builtin input function from cpython
sleep(5);                              // time.sleep(...)
```

## Running

CPython version over `3.13` is recommended (that is what I use(d)). Clone this repository, if you have `uv` run `uv sync` or use your system's "python" with `virtualenv`. Then from `gustav` module, pass your source file as an argument to run interpreter:

```shell
<python> -m gustav <file>
```

You can enable debug flag by setting `DEBUG=yes` as environment variable (it will show entire process, ast nodes, and some debug logs during runtime).

If you don't provide file as argument you will get `REPL` prompt. Which I see as "useless", because there is no support for statement/expressions yet. I wanted to implement this via `curses` library, Instead, I wanted to explore other concepts. There will be time for CLI app development.

## Development

Requirements

* `uv` - <https://docs.astral.sh/uv>
* `just` - <https://just.systems/man/en> (optional)

```shell
uv sync --dev
uv run pre-commit install
```

Make your changes as you wish. Use `gustav.logging.LOG` for debug logging and enable debug flag.

* If change introduces new syntax/tokens - make your additions in `scanner.py` first.
* Then you need to work on `parser.py` to construct AST nodes to produce something meaningful for evaluating.
* If you want, you can implement semantic analysis in `resolver.py` which has only lexical scope analyzer.
* In `interpreter.py` you can define evaluation steps of nodes that you've defined.

> [!NOTE]
> To update/extend AST nodes, edit `tools/ast_generator/definitions.py`(format is simple) then run
>
> ```shell
> just generate_ast
>  ```
>
> which will generate AST files in `gustav/ast` (expressions & statements are separated).
> Run checks which includes formatting, linting & type-checking:
>
> ```shell
> just check
> ```

Run tests (with no coverage)

```shell
just test-no-cover
```

With coverage

```shell
just test
open htmlcov/index.html
```

New tests can be added in `scenarios/` folder, most of these are taken from book's [repository](https://github.com/munificent/craftinginterpreters/tree/master/test). They have a simple format:

```js
print "hello world"; // expect: hello world
```

Testing framework looks for `// expect:` blocks, and compares these expectations with stdout & stderr outputs (combined) returned from interpreter.

## Grammar

Grammar definitions below are written using [Wirth Syntax Notation](https://en.wikipedia.org/wiki/Wirth_syntax_notation) instead of the more common `BNF`/`EBNF`. For this project, Wirth notation has no significant drawbacks and is fully compatible with the syntax diagram tool that I saw (See below. If you know other tools, let me know).

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

`<end></end>`

[^1]: The name _Gustav_ is inspired by the [Schwerer Gustav](https://en.wikipedia.org/wiki/Schwerer_Gustav), a massive German railway gun built during WW2. The intention is not to glorify(romanticize) war or any political ideology - especially not **Nazism** - but to appreciate the engineering behind it. Current implementation (in cpython) is heavy as this railway gun.
