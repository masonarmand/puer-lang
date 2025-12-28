# Puer
Statically typed procedural C-style scripting language.
Supports functions, data structures, dynamic arrays, and garbage collection.

## Example code:
### Records / Structs
```
rec Object {
    int x;
    int y;
};

def init_obj(int x, int y) -> Object
{
    Object obj;
    obj.x = x;
    obj.y = y;
    return obj;
}

def main()
{
    obj = init_obj(10, 20);
}
```

### Recursion Example:
```
def fibonacci(int n) -> int
{
        if (n <= 1)
                return n;
        return fibonacci(n - 1) + fibonacci(n - 2);
}

int n = 30;
int result = fibonacci(n);
println(result);
```

## Building
To build, simply run `make`.
```
git clone https://github.com/masonarmand/puer-lang.git
cd puer-lang
make
./build/puer
```
