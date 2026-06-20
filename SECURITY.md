# Security Policy

## Supported Versions

Because Swiq is under active structural development, security fixes are rolled directly into the main execution engine trunk. We currently only officially support security updates for the latest build.

| Version | Supported          |
| ------- | ------------------ |
| 1.x.x   | :white_check_mark: |
| < 1.0.0 | :x:                |

## Reporting a Vulnerability

We take the safety of the Swiq runtime engine seriously. If you find a security flaw—particularly engine exploits, arbitrary code execution via memory unsafe handlers, or host machine isolation breaches—please do not open a public GitHub issue.

Please report security vulnerabilities by opening a private draft security advisory directly through the repository or by contacting the maintainer via GitHub/development communication channels.

### What to Include in a Report:
1. **Description:** A detailed summary of the vulnerability and its potential impact.
2. **PoC Exploit:** A minimal `.swiq` source script or specialized token stream that reproduces the behavior.
3. **Environment:** Details on your environment (e.g., Termux on Android, Windows, C++ compiler version used to build the interpreter).

You can expect an acknowledgment within 48 hours, followed by updates as a patch is triaged and developed.

## Target Security Focus Areas

When auditing or fuzzing the Swiq implementation, please keep an eye out for these system-specific vectors:
* **Scope Leakage in Captures:** Ensuring that functions executing in isolated environments with explicit `[...]` capture blocks cannot implicitly leak or manipulate out-of-scope pointers or variables.
* **Archive State Bypasses:** Expliting edge cases in the `Parser` or `Interpreter` where an `archive x;` variable state can be evaluated or assigned to without an explicit `restore`.
* **File System Traversal:** Exploits involving the `@import` directive or `readFile()` builtin that allow reading files outside of the intended execution directory.
