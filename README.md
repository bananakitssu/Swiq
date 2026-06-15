# Swiq

A small programming language, built from scratch in C++.

Currently supports:
- Variable declaration: `set var x = 5;`
- Logging output: `log(x + 2);`
- Basic addition

## Example

```swiq
set var x = 5;
log(x + 2);
```

Output:
```
7
```

## Building

```bash
mkdir build && cd build
cmake ..
make
./swiq ../examples/hello.swiq
```

## Status

Early days — this is a learning project to understand how programming languages and compilers work, built one feature at a time.
