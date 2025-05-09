# TODO

## Core
- [X] basic control flow
    - [X] `if`, `if else`
    - [X] `for`, `while`
- [X] `&&` and `||`
- [X] Create internal generic for type system `Var`
- [ ] Data Types
    - [X] `int`
    - [X] `float`
    - [ ] `bool`
    - [ ] `char`
    - [X] `type[]` - dynamic arrays (supporting multiple dimensions)
    - [X] `str`
    - [ ] `dict` - Hashmaps similar to python dicts
    - [ ] `long`
    - [ ] `uint`
- [ ] Make assignment (`=`) work as an expression
- [ ] shorthand expressions
    - [ ] `++`, `--`
    - [ ] `+=`, `-=`, `/=`, `*=`, `%=`
- [ ] some way to convert between data types e.g, `str` -> `int`
- [X] functions with return types and args
- [ ] structs/records
- [ ] Garbage Collection

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

- [ ] C bindings
    - [ ] Binding to raylib library
- [ ] extended control flow (optional)
    - [ ] `do () {} while ()`
    - [ ] `switch` or `match`
- [ ] Function argument caching
