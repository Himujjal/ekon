# TODO

## Major:

- [ ] Complete the C library
- [ ] Implement Error handling
- [ ] Implement Util functions
- [ ] Write & test the nim code
- [ ] EKON Language Server
- [ ] EKON IDE Support
- [ ] EKON All Language Support
- [ ] EKON icon
    - [ ] DevIcons
- [ ] EKON RPC repo

## Miscellaneuos. 

- [x] Handle tabbed character in strings
- [x] handle missing values. same as the previous one 
- [ ] Handle unicode support for beautification
    - [ ] Get a stringified version of the DS with unescaped unicodes (EKON & JSON)
- [ ] copyHashmap function using an iterator
    - [ ] also update code in ekonValueCopyFrom to account for the hashmap 
- [x] Add support for underscore in integers (`1_00_000`)
- [x] Add binary number support: [Binary support for numbers](https://stackoverflow.com/a/13107)
- [x] Add support for octal number
- [ ] back to backticks for multiline baby
- [ ] Implement Tree-sitter
    - [ ] Add support for roundtrip preservation of comments, indentation etc etc.
         like [this comment](https://www.reddit.com/r/ProgrammingLanguages/comments/kevu2c/ekon_a_sane_json_alternative_need_strong/gg72hi0?utm_source=share&utm_medium=web2x&context=3)
        - regarding this. add a new file in lsp folder named ekon_liter.h and do the necessary 
    - [ ] Write the Nim bindings already. Use nim-treesitter for this.
    - [ ] Parse the schema using the same
- [ ] Errors to look out for:
    - [x] ekonchecker/fail20.ekon
    - [ ] Handle empty strings as keys - they are errors

- [ ] Write the schema code (clone of EkonValueParseFast) with support for comments


 

