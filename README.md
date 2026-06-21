# Swiq

<div align="center">
  <img src="assets/Swiq.svg" alt="Swiq Logo" width="250">
</div>

A small programming language, built from scratch in C++.

## Core Architectural Concepts

### Type vs. Interface Contracts
Data models are defined using either `type` or `interface` structures. Both enforce explicit base types (`Number`, `String`, `Boolean`, or `Array`) and numeric modifiers like `<integerOnly>` or `<floatOnly>` inside curly braces using field colon definitions:
* **`type` Definitions:** Flexible layout structures. Any field left omitted when creating an object instance automatically defaults to a raw `null` (`std::monostate`).
* **`interface` Contracts:** Strict data boundaries. Every single field must either define a default value within the signature or be explicitly supplied at initialization. Omitting a field with no default throws an engine runtime error.

### State-Machine Lifecycles
Swiq lets you manipulate runtime identifiers using explicit statement operators:
* `archive x;` — Strips a variable out of the active runtime execution environment and places it into an isolated background state map. It is blocked from all reads and mutations.
* `restore x;` — Revives an archived variable, hydrating its signature back into the live active scope.
* `reset x;` — Instantly rolls a variable back to the initial baseline value it held when it was declared.
* `delete x;` — Completely purges a variable's memory allocation and identifier tracking from the scope.

## Features

- Variables: `set var x = 5;` to declare, `set x = 10;` to reassign
- Advanced State Operators: `reset x;`, `delete x;`, `archive x;`, and `restore x;` to control variable lifecycles natively
- Hard Type Isolation: Integers (`long long`) and Floats (`double`) are strictly separated (e.g., `5 == 5.0` evaluates to `false`)
- Strict Tagged Objects: `{ field: value }<TypeName>` syntax validated against custom structures
- File System Modules: Compile-time code composition using the `@import "filename.swiq";` directive
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

### State Archiving
```swiq
set var securityToken = 0x7F;

archive securityToken;
// log(securityToken); // ❌ Throws runtime exception: variable is archived.

restore securityToken;
log(securityToken); // Output: 0x7F
```

### Interfaces and Layout Constraints
```swiq
set interface NetworkConfig = {
     port: Number = 8080,
     bitrate: Number<integerOnly>
};

// Allocate a typed object validated against the contract rules
set var primarySocket = { bitrate: 9600 }<NetworkConfig>;
log(primarySocket.port); // Output: 8080
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
- No nested function declarations (functions must be declared at the top
  level of a file).

## Repository Compliance Standards
Swiq is configured according to professional open-source workflow standards. See our internal files for development guidelines:
* **`CONTRIBUTING.md`** — Outlines our core Abstract Syntax Tree layout conventions and token processing architectures.
* **`SECURITY.md`** — Outlines scope execution isolation parameters and security auditing focuses.
* **`CODE_OF_CONDUCT.md`** — Details community contribution rules and etiquette standards.
