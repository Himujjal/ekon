# C API

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
    EkonType type;
    const char* key; // the key of that node and its length
    uint32_t keyLen;
    // The data structure that holds the value of the node
    union { struct EkonNode *node; const char* str; } value;
    uint32_t len; // The length of the str
    struct EkonNode* next; // next node in line
    struct EkonNode* prev; // prev node
    struct EkonNode* father; // parent node
    struct EkonNode* end; // end node of the objects
}
```
# Alternative DS

```c
struct EkonNode {
    EkonType type; // ARRAY, OBJECT, INT, FLOAT, STRING, NULL
    const char* key; // the key
    uint32_t keyLen; // length of the key

    union {
        struct EkonNode* node;
        const char* str; // string value
        float fNum; // store the int and float directly
        int iNum; // store int directly 
    } value; // value of the node if its a compound node
    uint32_t len; // length of str or object or array

    struct EkonNode* next; // next node in line
    struct EkonNode* parent; // parent node

    EkonHashMap* map; // hashmap if object

    // address of (address of this node in the `node's hash map`)
    // (*addrInMap
    struct EkonNode** addrInMap;
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



