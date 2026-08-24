<div align="center">
    <img alt="logo" src="./.github/assets/logo.svg">
</div>

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: light)" srcset="https://abduaziz.ziyodov.uz/badges/ai-free-dark.svg">
    <source media="(prefers-color-scheme: dark)" srcset="https://abduaziz.ziyodov.uz/badges/ai-free-light.svg">
    <img alt="bruh" src="https://abduaziz.ziyodov.uz/badges/ai-free-light.svg">
  </picture>
  <a href="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml">
    <img src="https://github.com/AbduazizZiyodov/gustav/actions/workflows/ci.yml/badge.svg" alt="CI">
  </a>
</p>

<p align="center"><i>Too heavy for its own good.</i></p>

# About

Experimental programming language with its own VM implemented in C.

# Build

```shell
./build.sh
```

This creates 4 binaries: debug, release, valgrind & valgrind (with GC stress enabled)

> [!NOTE]
> You can also refer to legacy tree walk interpreter [implementation](<https://github.com/AbduazizZiyodov/gustav/tree/legacy/python>) in CPython.

# Usage

Sample program(BST), create file named `test.gus`:

```javascript
class Tree {
    init(value) { this.value = value; this.left = this.right = nil; }

    insert(value) {
        if (value < this.value) {
            if (this.left == nil) { this.left = Tree(value); } 
            else { this.left.insert(value); }
        } else {
            if (this.right == nil) { this.right = Tree(value); }
            else { this.right.insert(value); }
        }
    }
}

fun binary_search(node, value) {
    if (node == nil) return false;
    if (node.value == value) return true;
    if (value < node.value) return binary_search(node.left, value);

    return binary_search(node.right, value);
}

var root = Tree(10);

root.insert(5); root.insert(15); root.insert(3); root.insert(7);

stdout Tree;
stdout root;
stdout binary_search(root, 7);
stdout binary_search(root, 15);
stdout binary_search(root, 12);
stdout binary_search(root, 5);
```

```shell
[gustav] # ./target/gustav_release ./test.gus
Gustav v1.0.0 [ git_branch=master git_commit=fd864dd | Aug  4 2026 03:34:11] [ Clang 19.1.7 (3+b1) ] | optimized RELEASE build for "Linux x86_64"

Tree
Tree<Instance>
true
true
false
true
```

# Testing

> [!NOTE]
> Current state of tests - all passing.
> There are features that should be implemented (they're excluded from test cases):
>
> * Loop Statement
> * Break & Continue Statements
> * Ternary Expressions
> * Lambda functions/expressions
> * Pipe operator
> Since, they were not in original "plan" - they're in TODO (rather than being main feature)

## Integration tests (CPython / pytest)

Install `pytest` via `uv` (or whataver):

```shell
uv tool install pytest
pytest -vv tests
```

## Unit tests (C / Criterion)

Pure C helpers (scanner, value array / equality) are covered with [Criterion](https://github.com/Snaipe/Criterion).

Debian / Ubuntu:

```shell
sudo apt-get install -y libcriterion-dev pkg-config
```

Build and run:

```shell
./build.sh unit-tests
```

Or manually:

```shell
cmake -B builds/build-unit-tests -G Ninja \
  -DCMAKE_C_COMPILER=clang-19 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_UNIT_TESTS=ON \
  -DCLANG_TIDY=OFF \
  -DSANITIZE=OFF
cmake --build builds/build-unit-tests --target test_scanner test_value
ctest --test-dir builds/build-unit-tests --output-on-failure
```

You can also run the binaries directly:

```shell
./builds/build-unit-tests/test_scanner --verbose
./builds/build-unit-tests/test_value --verbose
```
