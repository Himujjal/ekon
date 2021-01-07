# EKON - Ek Object Notation

[ekon.github.io - COMING SOON!](https://ekon.github.io)

EKON is a sane alternative for JSON that is geared towards readability and compressibility. It can be used to write both data-based files and config files.
***Ek*** (pronounced 'A(e)k') is a Sanskrit derived word which when translated to English means **one (1)**. EKON represents that philosophy. **Ek (one)** markup language for all uses.

## Contents

- [Features](#features)
- [Installation](#installation)
- [Syntax and Specification](#syntax-and-specification)
- [Schema Support](#schema-support)
- [Language Support](#language-support)
- [IDE Support for `.ekon` files](#ide-support-for-ekon-files)
- [Contribution and Issues](#contribution-and-issues)
- [License](#license)

## Features

- **Readability:** is kept in mind when designing EKON. So, EKON might feel similar to YAML but without constraining users to tab-based spacing
- **Compressibility** is a major factor that contributed to JSON being the most popular mode of communication among libraries and frameworks. EKON code is roughly ~40% smaller than JSON while maintaining high readibility.
- **Backwards Compatibility with JSON** is a factor that is kept in mind while designing EKON. Rename your `.json` file to `.ekon` and unlock a lot of possibilities.
- **Parser & Stringifier in C** using [zzzJSON](https://github.com/dacez/zzzJSON). Do checkout [zzzJSON](https://github.com/dacez/zzzJSON) written by [dacez](https://github.com/dacez).
- **Full compatibility with all programming languages** made easy by writing the library in C. WASM support boosts the cause further.
- **Schema support** using `*.d.ek` (EkScript definition) or `*.d.ts` (TypeScript definition files) files. [More on this](#schema-support)
- **Optional Commas** not only saves you keystrokes and filesize, but also improves readibility
- **Optional Root Object Curly Braces** also helps improve readibility

## Installation

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

## Syntax and Specification

EKON is fully compatible with [JSON](https://json.org) and [JSON5](https://json5.org). Here is the whole specification of EKON at one glance from [specs.ekon](./specs.ekon).

```javascript
/// specs.d.ek // or specs.d.ts // definition file for the current EKON file.
// { // If the global (root) structure is an object/map, ignore '{' (optional)

// single line comments only
// keyValues can be without quotes

// -- NULL
nullValue: null  // Yeah! commas are finally optional

// -- STRINGS

// Unquoted Strings are simple one word strings & have a few conditions:
//  1. No WhiteSpace characters
//  2. characters:
//      - ':', '{', '}', '[', ']', "'", '"', ',' will be parsed as end of strings
//      - If you wish to use the above in strings, use quoted strings instead.
//          Escape characters will not be supported in strings
//  3. "//" WON'T be parsed as string. It will be parsed as a part of comments
//  4. Reserved words: 'true', 'false', 'null' will be parsed as string
//  5. If its a number, it will be parsed as a number & not as string 
unquotedKey: forSimpleOneWordStrings
unquotedKey2: john_doe@gmail.com // TIP: URLS with ':' has to be under single quotes

// for full support of the above characters, use the following three forms
doubleQuotes: "You can use 'single-quotes' inside" // prefer single-quotes though
singleQuotes: 'You can use "double-quotes" inside'
multilineUseSingleQuotesAsWell: '
This is
a multiline
string
'
'single QuotedKey': 'Prefer single-quotes over double-quotes for keys'
'multiline
keys are
supported': github.com/Himujjal/ekon

// -- NUMBERS
intNumber: 123
floatingPointNumber: -0.12345
positiveSignNumber: +12345.123
hexadecimalNumber: 0xdecaf
leadingDecimalPoint: .123 // == 0.123
trailingDecimalPoint: 123. // == 123.0
binaryNumber: 0b110011
underscoreIntegers: 123_456
underscoreInOctalAndBinaryToo: 0b1001_1001_0000
Decimal: -.123

// -- ARRAYS
arrays: [
    "hello there"
    123
    { key: "value" }, // commas are optional
    [ "another array" ]
]

// -- KEY-VALUE MAPS (A.K.A.: OBJECTS)
objectMap: {
    world: 'No comma rules still apply inside'
    arr: [
        "Hello"
        "World"
    ],
    nestedObj: {
        inNestedObj: CurlyBracesCompulsory
    }
    anotherNumber: 123,   // trailiing commas
}

// -- JSON backwards compatibility
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

// -- Compressed form - yeah! unlike YAML, whitespace is insignificant in EKON files
stringVal:h arrayVal:[1,2,3,4]numVal:-.1 obj:{k:v}multiline:"hi\nthere\n"
// -- Comparison with compressed JSON
"stringVal":"h","arrayVal":[1,2,3,4],"numVal":-0.1,"obj":{"k":"v"},"multiline":"hi\nthere\n"

// } - // As said before. If root structure is an object/map, `{}` is optional.

// Root Array must require `[]` though
```

## Style Guide

Try to adher to these rules while writing proper `EKON` based configs:
These don't matter for minified form. `Formatter` & `Minifier` APIs
will soon be available (TODO!).

1. Prefer unquoted strings as key
2. Prefer unquoted strings as values whenever possible
3. Use single quotes over double quotes whenever possible for values & keys
4. Try your best to avoid commas, looks clean
5. Use 80 as the maximum line length. For under 80 chars lines:
    a. Arrays should be put into single line for under 80 chars lines 
    b. Objects should have put into single line "with commas" for under 80 chars lines
6. For nested data structures, prefer prepending at least 2 whitespace. No rule beyond that.
    EKON is anyways whitespace agnostic.

## Schema Support

EKON supports schema based on a ~~`.d.ek` (EkScript) or~~ `.d.ts` (TypeScript) definition files or directly inside the `.ekon` file.
This schema is extremely used for IDEs and text-editors or for CI/CD. But of course it will have no meaning while parsing.

***Steps for Schema (directly inside the file):***

For this step, inside the [specs.ekon](./specs.ekon) file, write your schema definition inside backticks (`` ` ``).
This first backtick must be the first non-whitespace character in the EKON file. The **root** type is the root Node Type.
Everything inside the backticks support TypeScript based definitions, partially.

*specs.ekon:*
```typescript
`
objectMapType = { world: string, arr: string[], anotherNumber: number };
arraysType = [string, number, {key: string}, string[]];
root = {
    unquotedKey: string,
    singleQuotes: string,
    multilineStrings: string,
    intNumber: number,
    floatingPointNumber: number,
    leadingDecimalPointNumber: number,
    trailingDecimlPointNumber: number,
    positiveSignNumber: number,
    hexadecimalNumber: number,
    arrays: arraysType,
    objectMap: objectType,
    hello: { array: string, "number": number },
    key: number,
    jsonObject: { key: string }   
}
`
//... rest of the .ekon file
```

***Steps for Schema in a `.d.ts` file:***

Let's say the definition of `specs.ekon` file is present as a *type* in [specs.d.ts](./specs.d.ts) file.
Now, inside your `specs.ekon` file in the first line (don't forget the backtick):

```typescript
`root = import('./specs.d.ts').specsEkonSchema`
```

## Language Support

- [ ] C ([ekon.h](./ekon.h)) (Yes it works without Nim compiler)
- [ ] Nim ([This Repo](./.ekon.nim))
- [ ] [Go](./ekon.go)
- [ ] ~~EkScript~~
- [ ] ~~C++~~
- [ ] ~~Javascript/TypeScript~~
    - [ ] ~~node-ekon (WASM + Native) - performant~~
    - [ ] ~~ekon.js (Pure JS) - size optimized~~
- [ ] ~~Python~~
- [ ] ~~Rust~~
    - [ ] ~~ekon-rust~~
    - [ ] ~~serde-rust~~
- [ ] ~~Dart~~
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

If any language is missing please create a pull request/file an issue.


## IDE Support for `ekon` files

- [ ] ~~Language Server~~
- [ ] ~~Vim~~
- [ ] ~~NeoVim (Lua based)~~
- [ ] ~~VS Code~~
- [ ] ~~Visual Studio~~
- [ ] ~~Micro~~
- [ ] ~~Atom~~
- [ ] ~~Sublime~~
- [ ] ~~Emacs~~
- [ ] ~~Kate~~
- [ ] ~~Notepad++~~

# Benchmarks

All benchmark code is to be found [benchmarks](./benchmarks) folder

TODO!

## C API:

TODO!

## Nim API:

TODO!

## ROADMAP & TODOs

- [ ] Nim + C repo: 
    - [ ] Complete the C library
        - [x] Support for Comments
        - [x] Support for single-quote and backtick based strings
        - [x] Support for unquoted strings for both keys and values
        - [x] Support for trailing and preceding decimal numbers
        - [x] Support for positive and negative signs
        - [x] Support for hex numbers
        - [x] Support for optional commas
        - [x] Support for trailing commas
        - [ ] Stringify to EKON
        - [ ] Stringify to JSON and strict JSON parsing support
        - [ ] Beautify support
        - [ ] Minify support
        - [ ] `\r\n` support for windows
    - [ ] WebAssembly Support

## Contribution and Issues

Contributions are always, always welcome!

For ***Contribution***, please refer to [Contributors](./contribution.md#contributors)

For ***Issues***, Please refer to [Issues](./contribution.md#issues).

You can find the same in the [Wiki]() section also.

## [License](./license)

```
MIT License

Copyright (c) 2020 EKON

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
