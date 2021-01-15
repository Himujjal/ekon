# Specifications of EKOwN

TL;DR:
> Strip away commas `,`, strip away the quotes `""` for
 non-spaced keys and values, strip away `{}` from
 root node, put single quotes `''` over double quotes `""`
 everywhere else. Do this with JSON and voila!

> You have EKON!

EKON constitutes of 6 different kinds of data also called as
 Nodes. 4 of them are _scalar_ data and 2 are _compound_ types.
 Each individual type and its value is a node in EKON.

All Nodes in EKON:

1. `null` node (scalar node)
2. `boolean` node (scalar node)
3. `number` node (scalar node)
4. `string` node (scalar node)
5. `array` node (scalar node)
6. `object` node (scalar node)

Optional Nodes:
1. Comments
2. Schema

# Comments
Comments are single line only like Rust written
```js
// hello world
key: value // this is a comment
```

# Scalar Nodes 
 
### 1. `null` Node
 Null node consists of a one single value `null`. Whatever `null`
 means is different from language to language. In JavaScript, a 
 null valued variable means it doesn't contain any data. Consider
 this to be a node without any substantial data.

Use cases of null:
```js
key: null
[ null 1 ]
```

### 2. `boolean` node  
Boolean node can have two sets of values: `true` and `false`.
Again how this is interpreted is different from language to language.
But most languages consider `true` to have a value equivalent to `1` and `false` to have value equivalent to `0`

Use cases of boolean node:
```js
true
false
key: true
key2: false
[true]
```

### 3. `number` node

Number node consists of a numerical value although its limits are
 not set, therefore a number will have the following format represented in string:

```js
// NOTE: there are no ints, floats in EKON. They are all numbers 

// int
let int = ((+-)? (0 | ([1-9][0-9_]*[0-9]))
let intPreZero = ([0-9][0-9_]+[0-9])

// float
let float = (int) (.intPreZero)? ((eE)? (+-)? intPreZero)?

// binary
let binary = (+-)? ((0b) [01][01_]+[01])

// hexadecimal
let hexadecimal = (+-)? ((0x) ([0-9A-Fa-f][0-9A-Fa-f_]+[0-9A-Fa-f]))

// octal
let octal = (+-)? ((0o) ([0-7][0-7_]+[0-7]))

EKONNumber = int | float | binary | hexadecimal | octal
```

NOTE: If any value doesn't fit the criteria for a number, you get a string.

Examples:
```js
// int
1_2 // 12
12_ // ERROR!! not a number. '_' as the last digit wont be allowed
_12 // ERROR!! not a number. '_' as the first digit won't be allowed
+1221313134132431413241 // no limits. EKON has no upper limits. Its upto the implementing language to handle that  
-4_5 //

// float
-0.12

// binary
-0b1001_10001
0b11

// hexadecimal
-0xAFFF
+0x011_12_1231_FF

// octal
0o1273
-0o123_13_133 
```

## Strings

Strings are of two kinds in EKON.
a. Quoted
b. Unquoted.
But both fall under the strings node category. Its just different in how we write them

### Quoted strings

They are simply any characters under the sun that comes in between 
two quotes (`'...'`) except the quotes themselves (unless escaped with `\`)

All unicode are supported. If there is any failure in any unicode, its parsed as simple characters. (see example)

Example:
```js
'hello'
'123 asda rrg'
"string" // double quotes for JSON compatibility

// NOTE '\' are unecessary. just an addition
' multi \
line strings \
are valid \
'
'\u3231' // utf-8
'\U00013429' // utf-8 extended
'\x32' // hex
```

### Unquoted strings
Unquoted strings can have any characters under the sun except:
`'` , `"`, `,` , `:` , `[`, `]`, `{`, `}` and whitespace
 characters like: ` `, `\t`, `\r`, `\n`
 (well consider these whitespace as character codes rather than
 actually writing `\n` )

```js
hello
1230asdsd // anything that's not a valid number is a string
1331.1312sadasd

^1.2.2  
nullable
truer
falsee

unicode\u1344SupportToo! // unicode should be parsed into `\u1344` as unicode yes that ! is also a part
newl\nine // yes literally \n should be parsed as being '\' and 'n'
            // but when the string value is taken make it `\n` character
ta\tb
(1+2) // its a string! but why would you write this?!

tab\:hello // not a string. will be considered as tab\ : hello

github.com/ekon-org/ekon.git // yes. url's without `:` 
```

Okay! I get it. string parsing and number parsing will be a difficult job

# Compound nodes

## Arrays
Arrays have 0 or more scalar compound nodes under its belt. commas
 are optional as delimiters in arrays.
 but do prefer writing them when representing arrays
 that fit in 1 line. ()

```js
[]
[1 2]  // [1, 2]
[hello world this is a string array]
[
    'hello'
    1
    null
    true
    -13
    { key: value }
]
```

## Objects

Objects are a set of key value pairs enclosed under

Keys follow string node rules with which can also have `null`,
 `true`, `false` and any number node. Keys must have at least 1 character.

Keys are followed by `:`, which are then followed by any EKON Node. 

Rule:
```js
{ // optional in case the root node is an object
    ( key : value (,)? )*
} // optional in case of root node being an object
```

## Schema

EkScript supports writing schema using a subset of TypeScript 
Definition. Schema's are really useful for validating a EKON file
which are really helpful in a language server and CI/CD.
Schema are not parsed to the relevant Schema Data Structure always and it is to be done manually. Language bindings can skip writing schema parser.

Schema starts with writing backticks `` ` `` on top of the file
 and end with `` ` `` _at the start of the file_ (i.e. the schema 
 has to be the first non-whitespace thing).

In schema you can write types as follows. The `root` type represents
the type of the root Node of EKON



e.g.
```typescript
`
type anyArrayMember = (string | number | boolean | null | anyArrayMember | Record<string>)[] 
type root = {
    key: string,
    'number': number,
    obj: { hello: world },
    any: anyArrayMember[]
} 
`
key: value
number: 1.2
obj: {
    hello: world
}
any: [
    arraymem
    1.2
    true
    null
    [string]
    { hello: world }
]
```

Inside the backticks, schema will start with a `type` definition.
Multiple types can be written in the schema portion. you can also import type definitions
from a file or pass a type schema to the C API for validation.

To import schema from a file:
```typescript
`import { schema as root } from './type.d.ts'`
```

In case you have a case where you write a certain schema but in your language parser also put 
a schema as string, the schema declared on top will be the tie-breaker.

NOTE: schema is an optional feature. You might consider using it mainly for validation purposes in your language EKON parser. But if you think, its worth it, nothing is stopping you from doing it.

# JSON backwards compatibility

JSON can be easily parsed by EKON and EKON fully supports JSON
 format. But EKON does not enforce strict JSON checking. That's
 the work of a JSON parser

## Conclusion

EKON seems complicated so why prefer it over JSON or say JSON5?

 Thing is EKON is very readable and easy to write when writing
 general EKON files. Plus EKON when minified is ~40% smaller than
 JSON.
 For 95% of the case you will love writing EKON. For the extremely
 rare 5% cases, when confused, you can shift to classic JSON for
 that small ambiguity or confusion. Further, EKON code is highly
 readable.

Good luck! Hope you write lots of EKON and make the world a better place.