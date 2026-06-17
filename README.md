# Swiq

<div align="center">
  <img src="assets/Swiq.svg" alt="Swiq Logo" width="250">
</div>


A small programming language, built from scratch in C++.

## Features

- Variables: `set var x = 5;` to declare, `set x = 10;` to reassign
- Arithmetic with correct precedence: `+`, `-`, `*`, `/`
- Strings, with `+` for concatenation
- Booleans: `true`, `false`
- Comparisons: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Conditionals: `if (...) { ... } else { ... }`
- Loops: `while (...) { ... }` and `for (init; condition; post) { ... }`
- Functions with recursion: `func name(params) { ... return expr; }`
- Closures via explicit capture lists: `func name(params)[capturedVar] { ... }`
- Arrays: `[1, 2, 3]`, indexing `arr[0]`, index assignment `set arr[0] = 99;`
- Built-in functions: `len(arr)`, `push(arr, value)`
- Comments: `#`, `//`, and `/* ... */`
- A read-only environment namespace (`Swiq.__ENV__...`), accessible via `./swiq -v`

## Examples

### Basics
```swiq
set var x = 5;
log(x + 2);
```
Output:
```
7
```

### Functions and recursion
```swiq
func factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

log(factorial(5));
```
Output:
```
120
```

### Closures

By default, functions can only see their own parameters — no access to outer
variables. Adding a name to the `[...]` list right after the parameter list
gives that function read **and** write access to that one outer variable.
Anything not listed stays invisible.

```swiq
set var counter = 0;

func increment()[counter] {
    set counter = counter + 1;
    return counter;
}

log(increment());
log(increment());
log(counter);
```
Output:
```
1
2
2
```

### Arrays and for loops
```swiq
set var arr = [10, 20, 30];
push(arr, 40);

set var sum = 0;
for (set var i = 0; i < len(arr); set i = i + 1) {
    set sum = sum + arr[i];
}

log(sum);
```
Output:
```
100
```

## Building

```bash
mkdir build && cd build
cmake ..
make
./swiq ../examples/hello.swiq
```

To see version info:
```bash
./swiq -v
```

## Status

Early days — this is a learning project to understand how programming
languages and compilers work, built one feature at a time: a lexer, a
recursive-descent parser, and a tree-walking interpreter, all written in C++.

Current limitations worth knowing about:
- Functions are isolated by default — no closures except through the
  explicit `[captures]` syntax.
- No floating-point numbers yet — integers only (`long long` under the hood).
- No nested function declarations (functions must be declared at the top
  level of a file).
