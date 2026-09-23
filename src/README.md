# Swiq v1 progress

Version 1 is now complete according to the original v1 checklist.

Completed:

* Switcher
* Function overriding
* Function protection
* File writing
* File moving
* File deleting
* File copying
* Command execution
* var math v2
* `&&` and `||` support with short-circuit evaluation
* var math v2 support in `for` loops
* Library imports from ***Swiq***
* `.Split()` for strings
* Function archiving, deletion and restoration
* Function resetting
* `wait(seconds)` built-in
* `stop n;` with process status codes
* Nested functions
* Improved importing, variable creation and file finding

Total number of completed items: ***18***
Total number of uncompleted items: ***0***
Total: ***18***

### New v1 behavior

* Relative imports are resolved from the importing file's directory, and circular imports are rejected.
* Bundled libraries can be imported with `@import "Swiq/name";`, `@import "Swiq:name";`, or a bare library name when a matching file exists in `libraries/`.
* `.Split("delimiter")` returns a normal growable array.
* `wait(seconds)` pauses execution for the requested number of seconds.
* `stop n;` exits the interpreter with status code `n`.
* Function memory-management statements now work with functions as well as variables.
* Function declarations inside function bodies are supported as nested functions and are scoped to the call.
* Relative file operations fall back to the script directory when the path cannot be found from the current working directory.

***The original v1 checklist is complete.***