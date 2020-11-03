# EKON - Ek Object Notation

[ekon.org](https://ekon.org)

EKON is a sane alternative for JSON that geared towards readability and compressibility. It can be used to write both data based files and config files.
***Ek*** (pronounced 'A(e)k') is a Sanskrit derived word which when translated to English means **one (1)**. EKON represents that philosophy. **Ek (one)** markup language for all uses.

# README Contents

- [EKON - Ek Object Notation](#ekon---ek-object-notation)
- [README Contents](#readme-contents)
- [Features](#features)
- [Syntax & Specification](#syntax-&-specification)
- [Schema Support](#schema-support)

# Features:

- **Readability:** is kept in mind when designing EKON. So, EKON might feel similar to YAML but without constraining users to tab-based spacing
- **Compressibility** is a major factor that contributed to JSON being the most popular mode of communication among libraries and frameworks. EKON code is roughly ~40% smaller than JSON while maintaining readibility.
- **Backwards Compatibility with JSON** is a factor that is kept in mind while designing EKON. Rename your `.json` file to `.ekon` and unlock a lot of possibilities.
- **Fastest parser for EKON** is provided thanks to [zzzJSON](https://github.com/dacez/zzzJSON) written in C.
- **Full compatibility with all programming languages** made easy by writing the library in C. WASM support boosts the cause further.
- **Schema support** using `*.d.ek` (EkScript definition) or `*.d.ts` (TypeScript definition files) files. [More on this](#schema-support)
- **Optional Commas** not only saves you keystrokes and filesize, but also improves readibility
- **Optional Root Object Curly Braces** also helps improve readibility

# Syntax & Specification:

Remember EKON is fully compatible with JSON and [JSON5](https://json5.org). Here is the whole specification of EKON at one glance from [specs.ekon](./specs.ekon).

```javascript
/// type.d.ek // definition file for the current EKON file.
// { // If the global (root) structure is an object/map, you can ignore writing '{' (optional)

// single line comments only
// you can skip writing the commas (optional)

// strings
unquotedKey: "Represents a string & you can use 'single-quotes' inside"
singleQuotes: 'A string & you can use "double-quotes" inside'

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
hello: {
    world: "No comma rules still apply inside"
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
    "number": 123
},
"key": 123,
{
    "key": "value"
}

// } - // As said before. If root structure is an object/map, `{}` is optional
```


# Schema Support

EKON supports schema based

# Language Support:

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


# IDE Support for `.ekon` files

- [ ] ~~Language Server~~
- [ ] ~~Vim~~
- [ ] ~~NeoVim (Lua based)~~
- [ ] ~~VS Code~~
- [ ] ~~Micro~~
- [ ] ~~Atom~~
- [ ] ~~Sublime~~
- [ ] ~~Emacs~~

# Nim API:

TODO!

# C API:

TODO!

# ROADMAP:

- [ ] Nim + C repo: 
    - [ ] Write & test the nim code
    - [ ] Write & test the compiled C API
- [ ] EKON Language Server
- [ ] EKON IDE Support
- [ ] EKON All Language Support
- [ ] EKON RPC repo

# Contribution and Issues:

Contributions are always, always welcome!

For ***Contribution***, please refer to [Contributors](./contribution.md#contributors)

For ***Issues***, Please refer to [Issues](./contribution.md#issues).

You can find the same in the [Wiki]() section also.



