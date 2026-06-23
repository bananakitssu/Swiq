<div align="center">
  <img src="assets/Swiq.svg" alt="Swiq Logo" width="250">
</div>
---

Swiq is a **programming language** made by *bananakitssu*

The syntax is similar to these languages:
* C++/C#/C
* Python
* Kotlin
* Lua/Luau
* JavaScript/TypeScript

# Documentation

## Downloading Swiq

**(Note: You would need `cmake` and `make` installed)**

To download Swiq onto your device, download the ZIP file, extract it, then go into the extracted folder:

```bash
cd Swiq
```

Then make a new directory called `build` and go into that folder:

```bash
mkdir build && cd build
```

Then run **CMake** and build it with `make`:

```bash
cmake .. && make
```

You now have Swiq as an executable file, to move it run:
```bash
mv ./swiq to/another/directory
```
or to copy it, run:
```bash
cp ./swiq to/another/directory
```

## Swiq commands

There are currently **2** commands in Swiq.

For getting the Swiq version:
```bash
./swiq -v
```

For running a Swiq script:
```bash
./swiq <file>
```

## Variables

Now let's see how we can create/update variables.

### Normal Variables

To create **readable** and **writeable**:

```swiq
set var x = 5;
```

* `set var` creates the variable
* `x` is the variable name
* `5` is the variable value

### Protected Variables

To create protected variables, that are **read-only**:

```swiq
set var<Protected> x = 5;
```

* `set var` creates the variable
* `<Protected>` protects the variable, making it *read-only*
* `x` is the variable name
* `5` is the variable value

### Local/Global variables

If the script would need to be imported to another script, you can choose what variables it can see and what variables it cannot:

```swiq
set local var x = 5;
```

* `set local var` creates the variable but it's local and only avaliable to the current script, not shown when imported
* `x` is the variable name
* `5` is the variable value

To make it **global** *(This is the default)*:

```swiq
set global var x = 5;
```

* `set global var` creates the variable globally
* `x` is the variable name
* `5` is the variable value

### Changing a variable's value

To change a variables value, that do not have `<Protected>` you have to just only use `set`:
```swiq
set x = 10
```

## Math

Swiq supports multiplying (`*`), dividing (`/`), adding (`+`) and substracting (`-`)

Examples:

### Multiplying
```swiq
log(10 * 10);
```
Output:
```
100
```

### Dividing
```swiq
log(6 / 2);
```
Output:
```
3
```

### Adding
```swiq
log(5 + 5);
```
Output:
```
10
```

### Substracting
```swiq
log(10 - 5);
```

**(More information comming soon)**

# Changelog

No published releases

**(Swiq is still under-development)**
