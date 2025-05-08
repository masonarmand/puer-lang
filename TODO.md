# TODO
- [X] make `eval_expr` return generic Var type instead of int
- [X] an extensible way to add builtin functions without adding anything to parser or parse tree.
- [ ] Make assignment statements work as expressions for example: `while(var = isTrue())`
- [ ] add in support for more basic types:
    - [ ] `uint` (unsigned integer)
    - [ ] `long`
    - [X] `float`
    - [X] `str`
    - [ ] `char`
    - [ ] `bool`
    - [X] mixed type coersion
- [X] More control flow statements:
    - [X] for loop
    - [X] while loop
- [ ] standard library / builtin functions:
    - [X] `input()`
    - [X] `getch()`
    - [ ] ...
- [ ] Record/struct with members
- [X] Basic procedures
- [X] functions with return types and args
- [ ] `@require` preprocessing
    - Or `include`, idk haven't decided on name or syntax or mechanism for how this would
    work

## other
- [ ] Garbage collection
    - [ ] Reduce memory fragmentation
- [ ] C bindings
    - [ ] Binding to raylib library
- [ ] Other types:
    - [ ] Dynamic Arrays
- [ ] Shorthand expressions ++, --, +=, etc
- [ ] Basic module/namespace system
