# Data Structures that are being used in the following parser.

### This document servers as a reference to the Data Structures being used in the `ekon.h` based file

It helps in building C bindings for other languages

### `EkonType`

1. `EKONTYPEBOOL`: boolean values
2. `EKONTYPEARRAY`: array values
3. `EKONTYPEOBJECT`: an object
4. `EKONTYPESTRING`: a string
5. `EKONTYPENULL`: null values
6. `EKONTYPENUMBER`: a number

### `EkonNode`
This is the Node of the data structure.

```c
struct EkonNode {
    // Type of the Node
    char type;

    // the key of that node and its length
    const char* key;
    uint32_t keyLen;

    // The data structure that holds the value of the node
    union { struct EkonNode *node; const char* str; } value;

    // The length of the str
    uint32_t len;

    struct EkonNode* next; // next node in line
    struct EkonNode* prev; // prev node
    struct EkonNode* father; // parent node
    struct EkonNode* end; // end node of the objects
}
```

### `EkonValue`

This is the data structure that communicates with other libraries

```c
struct EkonValue {
    // The Node that carries the whole data struct
    EkonNode* n;

    // The Allocator that is used to allocate EKON data structure into memory
    // clear this allocator using `ReleaseAllocator(a)`
    EkonAllocator* a;
}
```

### `EkonString`

```c
struct EkonString {
    char* data; // The actual data regarding the string
    uint32_t pos; // position of the ptr for the string 
    uint32_t size; // size of the string
    struct EkonAllocator* a; // EkonAllocator allocates the string in memory
}
```

### `EkonArray`

### The Rules for other languages

1. Expose a `EKON` named class with only static members. No instantiation needed. If the language doesn't support classes, somehow manage a namespace or whatever so that you can call methods \
    in the form of `EKON.parse(....)`

2. The `EKON` class should have the following methods:
    a. `EKON.parse()`: Takes in a string or character array and returns the required `EKON` data structure.
    b. `EKON.stringify()`: Takes in the `EKON` data structure and returns a string or character array
    c. `EKON.minify()`: Takes in a string or character array and returns the minified version of the string 
    d. `EKON.stringify_parse()`: First stringifies the `EKON` data structure and then returns the parsed `EKON` data structure


