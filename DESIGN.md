# Puer Language Design

Puer is a statically typed scripting language. The syntax is similar to C,
but with more of a scripting-style flavor. I wanted to make this because
most scripting languages have dynamic types which I personally am not a
fan of. I also think C style syntax is very elegant and am not a fan of
whitespace-based syntax.

```
@require "file.puer"

def Point
{
    int x;
    int y;
}


def add(int a, int b) -> int
{
    return a + b;
}


// functions with no return value return void by default.
def main() 
{
    print("Hello World");

    for (int i = 0; i < 5; i++) {
        print(i);
    }

    Point p;
    p.x = 10;
    p.y = 20;

    print(p.x);
}

main();
```

## Features

### Memory & Garbage Collection

All variables will be heap allocated.

If I have enough time I will implement a basic mark and sweep garbage
collector. This method is quite slow, but it will handle circular
references and will make the language feel more scripting-like, since the
programmer will not have to worry about memory management.

Mark and sweep works like this:
- A graph of all heap allocated objects is created during runtime.
- GC starts from the roots of the graph
- Follows references from the roots to find what is still reachable
    - depth-first search
- Marks every unreachable object.
- The graph is then traversed a second time, this time each marked object
is freed from memory.

Problems and additional notes:
- This GC method halts program execution until the GC is finished. My
interpreter will probably already be super slow, so this will make it
slower.
- This method results in memory fragmentation - additional steps can be
taken to reduce fragmentation.

### C style syntax

The language ignores whitespace and uses braces and semicolons similar to
C.

### Functions and Procedures

The language supports functions. Defined using `def` keyword:
```
def functionName(type arg1, type arg2) -> return_value
{
}
```
Example:
```
def factorial(int n) -> int
{
    int fact;
    for (int i = 0; i < n; i++) {
        fact += i;
    }
    return fact;
}
```

Although I generally prefer the C style functions where the return type
is written before the function name, I've noticed this looks messy with
data types with long names. This is why I've chosen to put the return
type at the end of the function.

### @require preprocessing
You can 'include' files as a preprocessing step, like in C. The interpreter will avoid circular inclusions, so a file will only be included once.
```
math.puer
---------
def sum(int a, int b) -> int
{
    return a + b;
}
```
```
main.puer
---------
@require math.puer

int n = sum(10, 20); // Calls sum from math.puer
print(n);
```

### Structs / records

Declare struct using keyword `def`. Every instance of a struct is a
pointer to the struct (like in scripting languages such as python).
This means if the struct is passed into a function as a parameter, the 
function will be able to directly edit its members and the struct won't
be copied.
```
def Point
{
    int x;
    int y;
}


def setZero(Point p)
{
    p.x = 0;
    p.y = 0;
}


Point p;
setZero(p);

print(p.x); // zero
```

## Extended Features

If I somehow have extra time I may consider implementing these features:
- Ability to make language bindings to C libraries:
    - Goal is to make a raylib binding so that I could make a game with
    2d graphics in puer.
- Basic module system to mimick namespaces so functions defined in `Module math {}` will be called like this `Math.sum`.

# Implementation Notes

## Scalable Parse Tree
Instead of each Node having a fixed number of node references, we can
just use an array of children nodes, with the makeNode() function having
a dynamic number of args.

```C
typedef struct Node {
    NodeType type; /* enum */
    struct Node** children;
    int n_children;
} Node;
```

Then makeNode() could be more flexible. maybe something like this:
```C
Node* makeNode(int type, int n_children, ...)
{
    Node* n = malloc(sizeof(n));
    n->type = type;
    n->n_children = n_children;
    n->children = malloc(sizeof(Node*) * n_children);

    va_list args;
    va_start(args, n_children);
    for (int i = 0; i < n_children; i++) {
        node->children[i] = va_arg(args, Node*);
    }
    va_end(args);
    
    node->value = NULL;
    return node;
}
```

## No casting longs

I did some research and found out about %union in yacc. This allows you
to specify the data types of each token or nonterminal.

example:
```C
%union
{
    int val;
    struct Node* node;
}

%type <node> expr stmt cond
%type <val> number
```
So then using makeNode from earlier (with Node* instead of long):
```C
stmt: IF cond stmt ELSE stmt
{
     $$ = makeNode(IFELSE, 3, $2, $3, $5);
}
```

## Values
static type variables using union & enum. Example:
```C
typedef struct Value {
    ValueType type;
    union {
        int i;
        float f;
        void* ptr;
    } data;
    bool const; /* maybe support for constants? */
} Value;
```
