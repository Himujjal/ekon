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
    - [x] Get a stringified version of the DS with unescaped unicodes (EKON & JSON)
- [x] TOMORROW: Handle duplicate fields
    - [x] Make the hashset allocation in the allocator 
    - [x] Error messages in ekonValueSetKey**..
    - [x] Replace mode when setting the value of a key: check if key is not null, then check if node is object, then set the value of the node to the function's passed value
    - [x] In replace mode, pass a `noDuplicateKey` value to ensure settting duplicate key value is not permitted. 
    - [x] Also make sure to have `noDuplicateKey`. If key is already present return the node pointer. 
    - [ ] copyHashset function
- [x] Add support for underscore in integers (`1_00_000`)
- [x] Add binary number support: [Binary support for numbers](https://stackoverflow.com/a/13107)
- [x] Add support for octal number
- [ ] Add support for indentation preserved multiline string. Refer: []()
- [ ] Implement Tree-sitter
    - [ ] Add support for roundtrip preservation of comments, indentation etc etc.
         like [this comment](https://www.reddit.com/r/ProgrammingLanguages/comments/kevu2c/ekon_a_sane_json_alternative_need_strong/gg72hi0?utm_source=share&utm_medium=web2x&context=3)
- [ ] Provide a tab-based beautify option & optional preserve comments section
- [x] Parsing of Schema with the backticks
- [x] Stringifying to the correct string value
    - [x] EKON
    - [x] JSON
- [ ] Schema generation

