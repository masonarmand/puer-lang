# TODO

## Core
- [X] basic control flow
    - [X] `if`, `if else`
    - [X] `for`, `while`
- [X] `&&` and `||`
- [ ] Data Types
    - [X] `int`
    - [X] `float`
    - [X] `bool`
    - [ ] `char`
    - [X] `type[]` - dynamic arrays (supporting multiple dimensions)
    - [X] `str`
    - [ ] `long`
    - [ ] `uint`
- [X] Garbage Collection
- [ ] fix grammar and handling of n-dimensional arrays so that functions can return
multi dimensional arrays.
- [X] Make assignment (`=`) work as an expression
- [X] shorthand expressions
    - [X] `++`, `--`
    - [X] `+=`, `-=`, `/=`, `*=`, `%=`
- [ ] some way to convert between data types e.g, `str` -> `int`
- [X] functions with return types and args
- [X] structs/records
- [ ] Ability to initialize a record with comma separated field values surrounded by `{ }`, similar to the array initializer.
- [ ] Strict Record return types instead of just returning `TYPE_REC`.

## stdlib, IO

- [X] An extensible way to make builtin functions (without modifying parser)
- [ ] modules / imports
- [ ] Standard Library Functions
    - [ ] File I/O & Process Control
        - [ ] open(), read(), write(), close(), directory traversal
        - [ ] argv for cmd line args
        - [ ] spawning processes
    - [ ] More functions for strings & arrays

## Other

- [ ] `dict` - Hashmaps similar to python dicts
- [ ] extended control flow (optional)
    - [ ] `do () {} while ()`
    - [ ] `switch` or `match`
- [ ] C bindings
    - [ ] Binding to raylib library
- [ ] Function argument caching
