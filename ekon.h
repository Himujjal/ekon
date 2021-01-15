/**
 * ============= ekon.h ===============
 * EKON API
 * LICENSE: MIT
 * Contents:
 * 1. Includes & Macros - line: ~(7 - 23)
 * 2. Type Definitions and Declarations: ~(21 - 50)
 * 2. EkonAPI Functions: ~(21 - 50)
 * ====================================
 */
#ifndef EKON_H
#define EKON_H

// ----------------------------------------------------------
// 1. INCLUDES & MACROS
// ----------------------------------------------------------
// ------ INCLUDES ------
#include <stdbool.h> // import bool, true, false
#include <stdint.h>  // import uint32_t

// ------ Type Macros Rust like --------
#define i8 int8_t
#define i32 int
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define f64 double

#define EkonOption uint16_t
// ----------------------------------------------------------
// 1. Type Definitions and Declarations
// ----------------------------------------------------------
struct _EkonNode;
struct hashmap_element_s;
struct hashmap_s;

// Type of EkonNode
enum _EkonType {
  EKON_TYPE_BOOL,
  EKON_TYPE_ARRAY,
  EKON_TYPE_OBJECT,
  EKON_TYPE_STRING,
  EKON_TYPE_NULL,
  EKON_TYPE_NUMBER,
};
typedef enum _EkonType EkonType;

// options for a particular node
typedef enum {
  // key string options
  EKON_IS_KEY_SPACED = 1 << 0,
  EKON_IS_KEY_MULTILINED = 1 << 1,
  EKON_IS_KEY_ESCAPABLE = 1 << 2,
  // value string options
  EKON_IS_STR_SPACED = 1 << 3,
  EKON_IS_STR_MULTILINED = 1 << 4,
  EKON_IS_STR_ESCAPABLE = 1 << 5,
  // value number type options
  EKON_IS_NUM_BINARY = 1 << 6,
  EKON_IS_NUM_OCTAL = 1 << 7,
  EKON_IS_NUM_DECIMAL = 1 << 8,
  EKON_IS_NUM_HEXADECIMAL = 1 << 9,
  EKON_IS_NUM_FLOAT = 1 << 10,
  EKON_IS_NUM_INT = 1 << 11
} EKON_NODE_OPTIONS;

// Node for allocator
struct _EkonANode {
  char *data;
  u32 size;
  u32 pos;
  struct _EkonANode *next;
};
typedef struct _EkonANode EkonANode;

// Memory Allocator (!!)
struct _EkonAllocator {
  EkonANode *root;
  EkonANode *end;
};
typedef struct _EkonAllocator EkonAllocator;

// Hashmap-Item
struct hashmap_element_s {
  const char *key;
  u32 keyLen;
  bool inUse;
  struct _EkonNode *value;
};
typedef struct hashmap_element_s EkonHashmapItem;

// Hash-Map
struct hashmap_s {
  u32 tableSize;         // max size
  u32 size;              // current size
  EkonHashmapItem *data; // array of HashmapItem
};
typedef struct hashmap_s EkonHashmap;

// EKON Node
struct _EkonNode {
  EkonType ekonType;
  uint8_t option;
  const char *key;
  u32 keyLen;

  EkonHashmap *keymap; // duplicate prevention & faster retrieval of objects

  // pointer to (pointer to the current node in `keyTable`)
  EkonHashmapItem *hashItem;

  struct _EkonNode **node;
  union {
    struct _EkonNode *node;
    const char *str;
  } value;
  u32 len; // string length
  struct _EkonNode *next;
  struct _EkonNode *prev;
  struct _EkonNode *father;
  struct _EkonNode *end;
};
typedef struct _EkonNode EkonNode;

// EKON Value - stores Node and Allocator
struct _EkonValue {
  EkonAllocator *a;
  EkonNode *n;
};
typedef struct _EkonValue EkonValue;

// TODO: Shift this and beautify to lsp/schema
// TODO: Make this an Enum
typedef struct EkonBeautifyOptions {
  bool unEscapeString;
  bool asJSON;
  bool preserveComments;
} EkonBeautifyOptions;

// Ekon String
struct _EkonString {
  char *data;
  u32 pos;
  u32 size;
  EkonAllocator *a;
};
typedef struct _EkonString EkonString;

static const u32 ekonDelta = 2;
static const u32 ekonAllocatorInitMemSize = 1024 * 4;
static const u32 ekonStringInitMemSize = 1024;
static const u32 ekonStringCacheInitMemSize = 128;

// --------------------------------------------------
// 3. EKON APIs
// --------------------------------------------------

/**
 * @brief Initializes memory allocator
 * @return EkonAllocator is a memory allocator
 * */
EkonAllocator *ekonAllocatorNew();

/**
 * @brief Allocates new memory of size
 * @param a memory allocator
 * @param size how much size of chunk do you allocate
 * @return memory address
 * */
char *ekonAllocatorAlloc(EkonAllocator *a, u32 size);

/**
 * @brief Release Allocator
 * @param rootAlloc Allocator to release
 * */
void ekonAllocatorRelease(EkonAllocator *rootAlloc);

/**
 * @brief Initialize Value using allocator
 * @param alloc EkonAllocator used for allocation
 * @return EkonValue with null node
 * */
EkonValue *ekonValueNew(EkonAllocator *alloc);

/**
 * @brief: error generator as string
 * @param outMessage  pointer to the character array
 *                    message will be of format: "<line>:<pos>:<char>"
 *                    NOTE: have to manually free this pointer using free(s)
 * @param s           start of the original string. NOTE: s[0] won't give you
 *                    start of the error point
 * @param index       index in `s` where the error occured
 * @return false
 * */
bool ekonParseError(char **outMessage, const char *s, const u32 index);

/**
 * @brief: error generator as string
 * @param outMessage  pointer to the character array
 *                    message will be of format: "<line>:<pos>:<char>"
 *                    NOTE: have to manually free this pointer using free(s)
 * @param s           start of the original string. NOTE: s[0] won't give you
 *                    start of the error point
 * @param keyLen      size of the keyLen
 * @param index       index in `s` where the error occured
 * @return false
 * */
bool ekonDuplicateKeyError(char **outMessage, const char *s, const u32 index,
                           const u32 keyLen);

/**
 * @brief The parser for Ekon String
 * @param v EkonValue where the parsed whole node is stored
 * @param s EKON Source code string
 * @param outErrMess the pointer to errMessage char-array
 * @param outSchema the pointer to the schema char-array.
 *                  if `schema == (char**)1;`, then the memory allocation in
 *                  outSchema won't happen.
 * @return true for success, false for failure
 * */
bool ekonValueParseFast(EkonValue *v, const char *s, char **outErrMess,
                        char **outSchema);

/**
 * @brief             The parser for Ekon String but with known length
 *                      Prefer this over ekonValueParseFast
 * @param v           EkonValue where the parsed whole node is stored
 * @param s           EKON Source code string
 * @param len         source code string length
 * @param outErrMess  the pointer to errMessage char-array
 * @param outSchema   the pointer to the schema char-array. if schema != NULL,
 *                    then the memory allocation will not happen in schema.
 *                    this is a performance measure.
 * @return            true for success, false for failure
 * */
bool ekonValueParseLen(EkonValue *v, const char *s, u32 len, char **outErrMess,
                       char **outSchema);

/**
 * @brief             Parser that calculates the length and then
 *                    ekonValueParseLen
 * @param v           EkonValue where the parsed whole node is stored
 * @param s           EKON Source code string
 * @param len         source code string length
 * @param outErrMess  the pointer to errMessage char-array
 * @param outSchema   the pointer to the schema char-array. if schema != NULL,
 *                    then the memory allocation will not happen in schema.
 *                    this is a performance measure.
 * @return            true for success, false for failure
 * */
bool ekonValueParse(EkonValue *v, const char *s, char **outErrMess,
                    char **outSchema);

/**
 * @brief                 Stringify to pure JSON
 * @param v               The EkonValue to stringify
 * @param unEscapeString  should string be unescaped on string generation
 * @return                generated JSON String
 * */
const char *ekonValueStringifyToJSON(const EkonValue *v, bool unEscapeString);

/**
 * @brief General stringifier - also acts as a minifier
 * @param v               The EkonValue to stringify
 * @param unEscapeString  should string be unescaped on string generation
 * @return                generated minified EKON String
 */
const char *ekonValueStringify(const EkonValue *v, bool unEscapeString);

/**
 * TODO: move this to lsp folder
 * @brief Generate Beautified Source to Source compiler
 * @param src         The string to beautify
 * @param outErrMess  err
 * @param options     Check EkonBeautifyOptions
 * @returns           beautified string
 * */
const char *ekonBeautify(const char *src, char **err,
                         EkonBeautifyOptions options);

/**
 * @brief Get string from an EkonValue wrapped node
 * @param v       EkonValue
 * @param outLen  stores the length of the string
 * @return        the output string
 * */
const char *ekonValueGetStrFast(const EkonValue *v, u32 *outLen);

/**
 * @brief Get string by copying the string value
 * @param v       EkonValue
 * @param option  pointer to option. this will be updated
 * @return        the newly allocated string
 * */
const char *ekonValueGetStr(EkonValue *v, EkonOption *outOption);

/**
 * @brief Get an unescaped string from the string
 * @param v       EkonValue whose node is a string
 * @return        Returns a newly allocated unescaped string
 * */
const char *ekonValueGetUnEspaceStr(EkonValue *v);

/**
 * @brief Get a number string fast. just the pointer to the old number
 * @param v       EkonValue whose node is a number
 * @return        pointer to the str containing the number
 * */
const char *ekonValueGetNumFast(const EkonValue *v, u32 *outLen);

/**
 * @brief Get a new copy of the str in EkonValue v
 * @param v       EkonValue whose node is a number
 * @return        a newly allocated string containing the number
 * */
const char *ekonValueGetNumStr(EkonValue *v);

/**
 * @brief Get num (double) directly stored in d
 * @param v       EkonValue to where the number lies
 * @param d       pointer to the f64 where number is stored
 * @return        success/failure
 * */
const bool ekonValueGetNum(EkonValue *v, f64 *d);

/**
 * @brief Get a (double) directly stored in d
 * @param v       EkonValue to where the number lies
 * @param d       pointer to the f64 where number is stored
 * @return        success/failure
 * */
const bool ekonValueGetf64(EkonValue *v, f64 *d);

/**
 * @brief Get an int directly stored in i
 * @param v       EkonValue to where the number lies
 * @param i       pointer to the int where number is stored
 * @return        success/failure
 * */
const bool ekonValueGetInt(EkonValue *v, int *i);

/**
 * @brief Get an long directly stored in l
 * @param v       EkonValue to where the number lies
 * @param l       pointer to the long where number is stored
 * @return        success/failure
 * */
const bool ekonValueGetLong(EkonValue *v, long *l);

/**
 * @brief Get an long long directly stored in ll
 * @param v       EkonValue to where the number lies
 * @param ll      pointer to the long where number is stored
 * @return        success/failure
 * */
const bool ekonValueGetLongLong(EkonValue *v, long long *ll);

/**
 * @brief Get a bool directly stored in outBool
 * @param v         EkonValue to where the bool lies
 * @param outBool   pointer to the bool where bool value will be stored
 * @return          success/failure
 * */
const bool ekonValueGetBool(const EkonValue *v, bool *outBool);

/**
 * @brief confirm if the value is a `null` node
 * @param v         EkonValue
 * @return          success/failure meaning is null node or not
 * */
bool ekonValueIsNull(const EkonValue *v);

/**
 * @brief Get the key off a node. copied value. not the original
 * @param v           value whose key to get
 * @param outOption   option will be stored
 * @param outKeyLen   keylen is stored
 * @return            pointer to the newly allocated key
 * */
const char *ekonValueGetKey(EkonValue *v, uint8_t *outOption, u32 *outKeyLen);

/**
 * @brief Get Unescaped key from a node
 * @param v       value from which to get the key from
 * @param outLen  pointer where the length is stored
 * @return        character string pointer
 * */
const char *ekonValueGetUnEspacedKey(EkonValue *v, u32 *outLen);

/**
 * @brief Get the pointer to the key string. (does not copy)
 * @param v       EkonValue where the node is a string
 * @param outLen  stores the length of the string
 * @return        returns the pointer to the string
 * */
const char *ekonValueGetKeyFast(const EkonValue *v, u32 *outLen);

/**
 * @brief Get a value from an object with a key without len
 * @param v       EkonValue where the node is present
 * @param key     string character key
 * @return        EkonValue with the node as the inner value
 * */
EkonValue *ekonValueObjGet(const EkonValue *v, const char *key);

/**
 * @brief Get a value from an object with a key with known len
 * @param v       EkonValue where the node is present
 * @param key     string character key
 * @param keyLen  key length
 * @return        EkonValue with the node as the inner value
 * */
EkonValue *ekonValueObjGetLen(const EkonValue *v, const char *key, u32 keyLen);

/**
 * @brief Get the type of a node in the value v
 * @param v       The value containing the inner node
 * @return        EkonType of the inner node. if error, returns 0
 * */
const EkonType ekonValueType(const EkonValue *v);

/**
 * @brief Get the len of the node in the EkonValue
 * @param v       The value containing the node
 * @return        the length of the EkonNode
 */
u32 ekonValueSize(const EkonValue *v);

/**
 * @brief Get a EkonValue member of an array given an index
 * @param v       The EkonValue whose node is an array
 * @param index   The index of the value to get
 * @return        The Value in the given index of the aray
 * */
EkonValue *ekonValueArrayGet(const EkonValue *v, u32 index);

/**
 * @brief Get the start of a array/object
 * @param v       The EkonValue whose node is an array/object
 * @return        EkonValue having its node as the first member of array/object
 * */
EkonValue *ekonValueBegin(const EkonValue *v);

/**
 * @brief Get the next value of the node
 * @param v       The EkonValue whose node's next value to get
 * @return        Ekonvalue with node which is next to v
 * */
EkonValue *ekonValueNext(const EkonValue *v);

/**
 * @brief Copy value from srcV to desV
 * @param desV?       The des value to copy to. note. memory has to be allocated
 *                      beforehand
 * @param srcV        The src value from whom to copy
 * @return            success/failure
 * */
bool ekonValueCopyFrom(EkonValue *desV, const EkonValue *srcV);

/**
 * @brief Copy value and return a new copy. allocation is handled internally
 * @param a     The allocator that will be used for the copy. srcV->a can also
 *              be used
 * @param v     srcValue to copy from
 * @return      EkonValue that is the copy of srcV
 * */
EkonValue *ekonValueMove(EkonAllocator *a, const EkonValue *srcV);

/*
 * @brief Move a value out of its father (object/array)
 * @param v EkonValue whose node is to be removed (NOTE: This is not the value
 * containing the object/array whose node is to be removed)
 * */
bool ekonValueMoveOutOfArrObj(EkonValue *v);

/**
 * @brief Set a Value containing a node to Null node
 * @param v       v->n will be replaced as a `null` node
 * @return        success/failure
 * */
bool ekonValueSetNull(EkonValue *v);

/**
 * @brief Set a Value containing a node to bool node
 * @param v       v->n will be replaced as a `bool` node
 * @param b       true / false depending on what to set
 * @return        success/failure
 * */
bool ekonValueSetBool(EkonValue *v, bool b);

/**
 * @brief Set a Value containing a node to number node (pointer only)
 * @param v       v->n will be replaced as a `num` node
 * @param num     string containing the number
 * @return        success/failure
 * */
bool ekonValueSetNumStrFast(EkonValue *v, const char *num);

/**
 * @brief Set a Value containing a node to number node along with the length of
 *                  the string - prefer this one over the above (pointer only)
 * @param v       v->n will be replaced as a `num` node
 * @param num     string containing the number
 * @param len     length of the string with the number e.g.'1.23'
 * @return        success/failure
 * */
bool ekonValueSetNumStrLenFast(EkonValue *v, const char *num, u32 len);

/**
 * @brief Set a Value containing a node to number node (copies the string)
 * @param v       v->n will be replaced as a `num` node
 * @param num     string containing the number
 * @return        success/failure
 * */
bool ekonValueSetNumStr(EkonValue *v, const char *num);

/**
 * @brief Set a Value containing a node to number node along with the length of
 *                  the string [prefer this one over the above (copies string)]
 * @param v       v->n will be replaced as a `num` node
 * @param num     string containing the number
 * @param len     length of the string with the number e.g.'1.23'
 * @return        success/failure
 * */
bool ekonValueSetNumStrLen(EkonValue *v, const char *num, u32 len);

/**
 * @brief set number from a double
 * @param v   EkonValue whose number is to be set
 * @param d   f64 value that is to set
 * @return    success/failure
 * */
bool ekonValueSetNum(EkonValue *v, const f64 d);

/**
 * @brief set number from a double
 * @param v   EkonValue whose number is to be set
 * @param d   f64 value that is to set
 * @return    success/failure
 * */
bool ekonValueSetf64(EkonValue *v, const f64 d);

/**
 * @brief set number from a double
 * @param v   EkonValue whose number is to be set
 * @param n   int value that is to set
 * @return    success/failure
 * */
bool ekonValueSetInt(EkonValue *v, const int n);

/**
 * @brief set number from a double
 * @param v   EkonValue whose number is to be set
 * @param l   long value that is to set
 * @return    success/failure
 * */
bool ekonValueSetLong(EkonValue *v, const long l);

/**
 * @brief set number from a double
 * @param v   EkonValue whose number is to be set
 * @param ll   long long value that is to set
 * @return    success/failure
 * */
bool ekonValueSetLongLong(EkonValue *v, const long long ll);

/**
 * @brief set string with an option (no copy)
 * @param v       whose number is to be set
 * @param str     string value that is to set
 * @return        success/failure
 * */
bool ekonValueSetStrFast(EkonValue *v, const char *str);

/**
 * @brief set string with an option (no copy)
 * @param v       whose number is to be set
 * @param str     string value that is to set
 * @param option  string options to follow
 * @return        success/failure
 * */
bool ekonValueSetStrLenFast(EkonValue *v, const char *str, u32 len);

/**
 * @brief set string with an option (copy)
 * @param v       whose number is to be set
 * @param str     string value that is to set
 * @return        success/failure
 * */
bool ekonValueSetStr(EkonValue *v, const char *str);

/**
 * @brief set string with an option with length (copy)
 * @param v       whose number is to be set
 * @param str     string value that is to set
 * @param len     length of the string
 * @return        success/failure
 * */
bool ekonValueSetStrLen(EkonValue *v, const char *str, u32 len);

/**
 * @brief set string with escape
 * @param v     EkonValue that will set string to its node
 * @param str   unescaped str to be escaped
 * @return      success/failure
 * */
bool ekonValueSetStrEscape(EkonValue *v, const char *str);

/**
 * @brief set string with escape in a string with length
 * @param v     EkonValue whose node is to be updated
 * @param str   unescaped str to be escaped
 * @param len   length of the unescaped str
 * */
bool ekonValueSetStrLenEscape(EkonValue *v, const char *str, u32 len);

/**
 * @brief set key fast
 * @param v     EkonValue whose node
 * @param key   String to set as key
 * @return      success/failure
 * */
bool ekonValueSetKeyFast(EkonValue *v, const char *key);

/**
 * @brief set key with length fast
 * @param v     EkonValue whose node to be updated
 * @param key   string to set as key in the node
 * @param len   length of the string to be set
 * @return
 * */
bool ekonValueSetKeyLenFast(EkonValue *v, const char *key, u32 len);

/**
 * @brief set key
 * @param v     EkonValue whose node to be updated
 * @param key   character array with key
 * @return      success/failure
 * */
bool ekonValueSetKey(EkonValue *v, const char *key);

/**
 * @brief set key with length
 * @param v     EkonValue whose node to be updated
 * @param key   string to be put in the node
 * @param len   length of the string in question
 * @return      success/failure
 * */
bool ekonValueSetKeyLen(EkonValue *v, const char *key, u32 len);

/**
 * @brief set key but escaping it first
 * @param v     EKonValue
 * @param key   key to be escaped and set in the v space
 * @return      success/failure
 * */
bool ekonValueSetKeyEscape(EkonValue *v, const char *key);

/**
 * @brief set key with length but escaping it first
 * @param v     EKonValue
 * @param key   key to be escaped and set in the v space
 * @param len   length of the key in question
 * @return      success/failure
 * */
bool ekonValueSetKeyLenEscape(EkonValue *v, const char *key, u32 len);

/**
 * @brief set array to a node
 * @param v     EKonValue whose node is to be set as array
 * @return      success/failure
 * */
bool ekonValueSetArray(EkonValue *v);

/**
 * @brief set object type to a node
 * @param v     EKonValue whose node is to be set as object node
 * @return      success/failure
 * */
bool ekonValueSetObj(EkonValue *v);

/**
 * @brief Sets a value from srcV to desV pointer based (not copied)
 *          but the srcV->n = 0 at the end
 * @param desV  EkonValue whose node is to be updated
 * @param srcV  EkonValue whose node is to be referenced for updating desV
 * @return      success/failure
 * */
bool ekonValueSetFast(EkonValue *desV, EkonValue *srcV);

/**
 * @brief Copies a value from srcV to desV
 * @param desV    EkonValue whose node is to be updated
 * @param srcV    EkonValue whose node is to be copties
 * @return        success/failure
 * */
bool ekonValueSet(EkonValue *desV, const EkonValue *srcV);

/**
 * @brief update a value in objV with childV as member (without allocation)
 * @param objV    EkonValue whose node is to be updated
 * @param childV    EkonValue whose node is to be copied without allocation
 * @return        sucess/failure
 * */
bool ekonValueObjAddFast(EkonValue *objV, EkonValue *childV);

/**
 * @brief update a value in objV with childV as member (allocation based)
 * @param objV      EkonValue whose node is to be updated
 * @param childV    EkonValue whose node is to be copied
 * @return          sucess/failure
 * */
bool ekonValueObjAdd(EkonValue *objV, const EkonValue *childV);

/**
 * @brief update a value in arrV with childV as member (without allocation)
 * @param arrV      EkonValue whose node is to be updated
 * @param childV    EkonValue whose node is to be copied without allocation
 * @return          sucess/failure
 * */
bool ekonValueArrayAddFast(EkonValue *arrV, EkonValue *childV);

/**
 * @brief update a value in arrV with childV as member
 * @param arrV      EkonValue whose node is to be updated
 * @param childV    EkonValue whose node is to be copied without allocation
 * @return          success/failure
 * */
bool ekonValueArrayAdd(EkonValue *v, const EkonValue *vv);

/*
 * @brief delete a member in arrV
 * @param arrV      EkonValue whose node is to be updated
 * @param index     index of the array to be deleted
 * @return          success/failure
 * */
bool ekonValueArrayDel(EkonValue *arrV, u32 index);

/*
 * @brief delete a member in objV
 * @param objV      EkonValue whose node is to be updated
 * @param key       key of the node that is to be removed
 * @return          success/failure
 * */
bool ekonValueObjDel(EkonValue *objV, const char *key);

#endif
