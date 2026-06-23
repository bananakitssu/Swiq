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

To create **readable** and **writeable** variables:

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
set x = 10;
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
Output:
```
5
```

## Functions

Now let's see how to create/run functions and run built-in functions:

### Creating a function (Without access to outside variables)

To create a function without access to outside variables is simple:

```swiq
func myFunction (arg1, arg2) {
  // code here
}
```

* `func` creates the function
* `myFunction` is the function name
* `arg1` is the parameter name, which is a variable accessible to the function, it's value depends on what data is sent while calling the function
* `,` is the parameter separater
* `arg2` is another parameter name

### Creating a function (With access to outside variables)

Creating a function with access to outside variables are different than functions without outside variable access:

```swiq
func myFunction (arg1, arg2) [x] {
  // code here
}
```

* `func` creates the function
* `myFunction` is the function name
* `arg1` is the parameter name, which is a variable accessible to the function, it's value depends on what data is sent while calling the function
* `,` is the parameter separater
* `arg2` is another parameter name
* `x` is the outside variable that the function would have access to

### Running a function

Running a function is very easy, you just need to type:

```swiq
myFunction("value1", "value2");
```

* `myFunction` is the function name of what function to call
* `"value1"` is the string to pass to the function for `arg1`
* `"value2"` is the string to pass to the function for `arg2`

### Running built-in functions

Swiq has built-in functions:

Getting a length of an array (Returns a number):
```swiq
len(array);
```
* `array` the array variable name

Adding an item to an array:
```swiq
push(array, 5);
```
* `array` the array variable name
* `5` the item to be added

Creating a size-limited array (`push` doesn't work here):
```swiq
set var array = AllocatedArray(10);
```
* `10` is the array limited size

Printing text to the console:
```swiq
log("hi");
```
* `"hi"` is the text to be printed

Reading a file:
```swiq
set var result = readFile("path/to/file.txt");
```
* `"path/to/file.txt"` the path to the file

## Arrays

Now let's look at arrays:

### Creating a normal array:

```swiq
set var array = [10, 5];
```
* `10` is the first value in the array
* `5` is the second value in the array

### About creating an allocated array:

Creating an allocated array was already stated in the `Functions -> Running built-in functions` section.

Notes:
* When creating an allocated array would fill the array with `null` up to the size of the allocated array
* The array would have the size limit
* Push cannot work, it would throw an error

**(More information comming soon)**

# Changelog

No published releases

> [!NOTE]
> Swiq is still under-development
