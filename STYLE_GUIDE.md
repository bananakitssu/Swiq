# Swiq Style Guide

This document describes the preferred coding style for the Swiq project.

The goal is consistency and readability.

---

# General Principles

Write code that is:

- Easy to read
- Easy to understand
- Easy to maintain

Avoid clever code if a simpler solution exists.

---

# Formatting

## Indentation

Use **4 spaces** or **2 spaces**.

Do not use tabs.

---

## Braces

Opening braces should remain on the same line.

```cpp
if (condition) {
    ...
}
```

Not:

```cpp
if (condition)
{
    ...
}
```

---

## Naming

### Variables

Use:

```cpp
camelCase
```

Example:

```cpp
tokenIndex
currentToken
errorMessage
```

---

### Functions

Use:

```cpp
camelCase
```

Example:

```cpp
parseExpression()
readToken()
throwInterpreterError()
```

---

### Types / Classes

Use:

```cpp
PascalCase
```

Example:

```cpp
Parser
Lexer
Interpreter
Token
```

---

### Constants

Use:

```cpp
UPPER_CASE
```

Example:

```cpp
MAX_STACK_SIZE
DEFAULT_PORT
```

---

# Comments

Write comments explaining **why**, not **what**.

Good:

```cpp
// Skip whitespace before reading the next token.
```

Avoid:

```cpp
// Increment i.
i++;
```

---

# Functions

Functions should ideally perform one task.

If a function becomes difficult to understand, consider splitting it.

---

# Error Messages

Error messages should:

- Clearly describe the problem.
- Mention the token or variable when possible.
- Avoid vague wording.

Good:

```
Expected ')' after expression.
```

Better than:

```
Syntax error.
```

---

# Commits

Use descriptive commit messages.

Examples:

```
Fix parser crash on empty arrays

Improve lexer performance

Add support for protected functions

Update documentation for interfaces
```

Avoid:

```
update

fix

stuff

changes
```

---

# Pull Requests

Keep Pull Requests focused.

A PR should ideally solve one problem or implement one feature.

---

Thank you for helping keep the Swiq codebase clean and consistent!
