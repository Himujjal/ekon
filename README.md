# EKON - Ek Object Notation

[ekon.github.io - COMING SOON!](https://ekon.github.io)

EKON is a sane alternative for JSON that is geared towards readability and compressibility. It can be used to write both data-based files and config files.
***Ek*** (pronounced 'A(e)k') is a Sanskrit derived word which when translated to English means **one (1)**. EKON represents that philosophy. **Ek (one)** markup language for all uses.

## README Contents

- [EKON - Ek Object Notation](#ekon---ek-object-notation)
- [README Contents](#readme-contents)
- [Features](#features)
- [Installation](#installation)
- [Syntax & Specification](#syntax-&-specification)
- [Schema Support](#schema-support)
- [Language Support](#language-support)
- [IDE Support for `.ekon` files](#ide-support)
- [Contribution and Issues](#contribution-and-issues)
- [License](#license)

## Features

- **Readability:** is kept in mind when designing EKON. So, EKON might feel similar to YAML but without constraining users to tab-based spacing
- **Compressibility** is a major factor that contributed to JSON being the most popular mode of communication among libraries and frameworks. EKON code is roughly ~40% smaller than JSON while maintaining high readibility.
- **Backwards Compatibility with JSON** is a factor that is kept in mind while designing EKON. Rename your `.json` file to `.ekon` and unlock a lot of possibilities.
- **A very fast parser for EKON** is provided thanks to [zzzJSON](https://github.com/dacez/zzzJSON) written in C.
- **Full compatibility with all programming languages** made easy by writing the library in C. WASM support boosts the cause further.
- **Schema support** using `*.d.ek` (EkScript definition) or `*.d.ts` (TypeScript definition files) files. [More on this](#schema-support)
- **Optional Commas** not only saves you keystrokes and filesize, but also improves readibility
- **Optional Root Object Curly Braces** also helps improve readibility

## Installation:

For Windows {Through Scoop only (To avoid multiple binaries)}:
```sh
TODO!
```
For Linux:
```sh
TODO!
```
For MAC:
```sh
TODO!
```
To compile from source, check: [Compiling from Source](#compiling-repo)

## Syntax & Specification:

EKON is fully compatible with [JSON](https://json.org) and [JSON5](https://json5.org). Here is the whole specification of EKON at one glance from [specs.ekon](./specs.ekon).

```javascript
/// specs.d.ek // or specs.d.ts // definition file for the current EKON file.
// { // If the global (root) structure is an object/map, you can ignore writing '{' (optional)

// single line comments only

// -- null
nullValue: null  // Yeah! commas are finally optional

// -- strings

// Unquoted Strings are simple one word strings & have a few conditions:
//  1. No ':', '{', '}', "'", '"', '[',  ']', ',' or whitespace characters
//  2. Reserved words: 'true', 'false', 'null' won't be parsed as string
//  3. If its a number, it will be parsed as a number & not as string 
unquoted: forSimpleOneWordStrings

// for full support of the above characters, use the following three forms
doubleQuotes: "You can use 'single-quotes' inside"
singleQuotes: 'You can use "double-quotes" inside'
multilineWithBacktickQuote: `
This is
a multiline
string
`
'singleQuotedKey': 'Although, it is encouraged to use unquoted keys'

// numbers
intNumber: 123
floatingPointNumber: -0.12345
leadingDecimalPointNumber: .12345
trailingDecimlPointNumber: 12345.
positiveSignNumber: +12345.123
hexadecimalNumber: 0xdecaf

// Arrays
arrays: [
    "hello there"
    123
    { key: "value" }, // commas are optional
    [ "another array" ]
]

// key-value maps/objects
objectMap: {
    world: 'No comma rules still apply inside'
    arr: [
        "Hello"
        "World"
    ],
    anotherNumber: 123,   // trailiing commas
}

// JSON backwards compatibility
"hello": {
    "array": [
        "hello"
    ],
    "numberVal": 123
},
"key": 123,
"jsonObject": {
    "key": "value"
}

// Compressed form - yeah! unlike YAML, whitespace is insignificant in EKON files
stringVal:h arrayVal:[1,2,3,4]numVal:1.1 obj:{k:v}multiline:"hi\nthere\n"

// } - // As said before. If root structure is an object/map, `{}` is optional.

// Root Array must require `[]` though
```

## Style Guide:

Try to adher to these rules while writing proper `EKON` based configs:
These don't matter for minified form. `Formatter` & `Minifier` APIs
will soon be available (TODO!).

1. Use unquoted strings as key
2. Use unquoted strings as values whenever possible
3. Use single quotes over double quotes whenever possible for values
4. Try your best to avoid commas, looks clean
5. Use 80 as the maximum line length. For under 80 chars lines:
    a. Arrays should be put into single line for under 80 chars lines 
    b. Objects should have put into single line with commas for under 80 chars lines
6. For nested data structures, prepend at least 2 whitespace. No rule beyond that.
    EKON is anyways whitespace agnostic.




## Schema Support

EKON supports schema based on a `.d.ek` (EkScript) or `.d.ts` (TypeScript) definition files. Simply write the type
of the `.ekon` file as `type ekonSpecs = ..` in the global namespace. The type name should start with `ekon` followed by the name of the file.

The `.d.ek` or `.d.ts` follow the TypeScript type definition specification.

For manual type declaration location, add `/// file: specs.d.ek | ekonSpecs` on the first line of the `.ekon` file.

If you want to have the type declaration in the `.ekon` file, declare the definition under triple slash comments (`///`) until your declaration
is not complete. Specification for this is the TypeScript type specs:

If the root object 
```typescript
/// root = {
/// key: string,
/// ...
/// }
```
or

if the root object/map is an array:
```typescript
/// [string, { key: number }, string[] ]
```

Example:

```typescript
// specs.d.ek

type ekonSpecs = {
    unquotedKey: string,
    singleQuotes: string,
    multilineStrings: string,
    intNumber: number,
    floatingPointNumber: number,
    leadingDecimalPointNumber: number,
    trailingDecimlPointNumber: number,
    positiveSignNumber: number,
    hexadecimalNumber: number,
    arrays: [ string, number, { key: string }, string[]],
    objectMap: { world: string, arr: string[], anotherNumber: number }
    hello: { array: string, "number": number },
    key: number,
    jsonObject: { key: string }   
}
```

## Language Support:

- [ ] C ([This Repo - **c_api** folder](./c)) (Yes it works without Nim compiler)
- [ ] Nim ([This Repo](./))
- [ ] ~~EkScript~~
- [ ] ~~C++~~
- [ ] ~~Javascript/TypeScript~~
- [ ] ~~Rust~~ 
- [ ] ~~Go~~
- [ ] ~~Python~~
- [ ] ~~PHP~~
- [ ] ~~Ruby~~
- [ ] ~~Lua~~
- [ ] ~~Java~~
- [ ] ~~Kotlin~~
- [ ] ~~Scala~~
- [ ] ~~C#~~
- [ ] ~~F#~~
- [ ] ~~Crystal~~
- [ ] ~~Haskell~~
- [ ] ~~Erlang~~ 
- [ ] ~~Elixir~~


## IDE Support for `.ekon` files

- [ ] ~~Language Server~~
- [ ] ~~Vim~~
- [ ] ~~NeoVim (Lua based)~~
- [ ] ~~VS Code~~
- [ ] ~~Micro~~
- [ ] ~~Atom~~
- [ ] ~~Sublime~~
- [ ] ~~Emacs~~

# Benchmarks

All benchmark code is to be found [benchmarks](./benchmarks) folder

TODO!

## C API:

TODO!

## Nim API:

TODO!

## ROADMAP:

- [ ] Nim + C repo: 
    - [ ] Write & test the nim code
- [ ] EKON Language Server
- [ ] EKON IDE Support
- [ ] EKON All Language Support
- [ ] EKON RPC repo

## Contribution and Issues:

Contributions are always, always welcome!

For ***Contribution***, please refer to [Contributors](./contribution.md#contributors)

For ***Issues***, Please refer to [Issues](./contribution.md#issues).

You can find the same in the [Wiki]() section also.

## [License](./license)

```
MIT License

Copyright (c) 2020 EK

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```




