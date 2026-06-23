---
name: "Feature Proposal"
about: Propose a new builtin function, modifier keyword, or syntax grammar rule for Swiq
title: "[PROPOSAL]: "
labels: enhancement
assignees: ''
---

## Is your feature request related to a problem?
A clear description of what the limitation is (e.g., "I want a native way to do X without chaining string methods").

## Proposed Syntax Architecture
Show us how this feature would look in real `.swiq` script files:
`swiq
// Show your syntax design here
`

## Core Engine Implementation Ideas
How do you visualize this changing the C++ codebase?
- What token(s) need to be added to the Lexer?
- How should the Parser represent this in the AST?
- What does the Interpreter do with the state/variables during `evaluate()`?

## Alternatives Considered
Any alternative approaches or workarounds you've thought of.
