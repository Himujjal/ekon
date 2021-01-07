#ifndef EKON_H
#define EKON_H

#include <stdbool.h> // import bool, true, false
#include <stdint.h>  // import uint32_t
#include <stdio.h>   // import snprintf
#include <stdlib.h>  // import atof, atoi, atol, atoll, malloc, free
#include <string.h>  // import memcpy

#include "hashtable.h"

static const bool ekonTrue = true;
static const bool ekonFalse = false;

// enumerate types with numbers
enum _EkonType {
  EKON_TYPE_BOOL,
  EKON_TYPE_ARRAY,
  EKON_TYPE_OBJECT,
  EKON_TYPE_STRING,
  EKON_TYPE_NULL,
  EKON_TYPE_NUMBER,
};
typedef enum _EkonType EkonType;

// The primary json node
struct EkonNode {
  EkonType ekonType;
  uint8_t option;
  const char *key;
  uint32_t keyLen;
  EkonHashTable *keyTable; // duplication prevention
  union {
    struct EkonNode *node;
    const char *str;
  } value;
  uint32_t len; // string length
  struct EkonNode *next;
  struct EkonNode *prev;
  struct EkonNode *father;
  struct EkonNode *end;
};

typedef enum {
  EKON_IS_KEY_SPACED = 1 << 0,
  EKON_IS_KEY_MULTILINED = 1 << 1,
  EKON_IS_KEY_ESCAPABLE = 1 << 5,
  EKON_IS_STR_SPACED = 1 << 2,
  EKON_IS_STR_MULTILINED = 1 << 3,
  EKON_IS_STR_ESCAPABLE = 1 << 4
} EKON_NODE_OPTIONS;

// ekon value
struct EkonValue {
  struct EkonNode *n;
  struct EkonAllocator *a;
};

struct EkonAllocator;
struct EkonValue;
typedef struct EkonBeautifyOptions {
  bool unEscapeString;
  bool asJSON;
  bool preserveComments;
} EkonBeautifyOptions;

struct EkonString {
  char *data;
  uint32_t pos;
  uint32_t size;
  struct EkonAllocator *a;
};

static const uint32_t ekonDelta = 2;
static const uint32_t ekonAllocatorInitMemSize = 1024 * 4;
static const uint32_t ekonStringInitMemSize = 1024;
static const uint32_t ekonStringCacheInitMemSize = 128;

// Node for allocator
struct EkonANode {
  char *data;
  uint32_t size;
  uint32_t pos;
  struct EkonANode *next;
};

// Allocator
struct EkonAllocator {
  struct EkonANode *root;
  struct EkonANode *end;
};
#ifndef EKON_MEMORY_NODE
#define EKON_MEMORY_NODE 1
#endif

#if EKON_MEMORY_NODE == 1

static inline void *ekonNew(uint32_t size) { return malloc(size); }
static inline void ekonFree(void *pointer) { free(pointer); }

#elif EKON_MEMORY_NODE == 2

static uint32_t ekonAllocMemorySize = 0, ekonAllocMemoryCount = 0,
                ekonFreeMemoryCount = 0;
void *ekonNew(uint32_t size) {
  return ekonAllocMemorySize += size, ekonAllocMemoryCount += 1, malloc(size);
}
void ekonFree(void *ptr) { freeMemoryCount += 1, free(ptr); }

#elif EKON_MEMORY_NODE == 3
#endif

#define EKON_LIKELY(x) __builtin_expect(x, 1)
#define EKON_UNLIKELY(x) __builtin_expect(x, 0)

#if EKON_EXPECT_MODE == 1
// https://www.ibm.com/support/knowledgecenter/SSGH2K_12.1.0/com.ibm.xlc121.aix.doc/compiler_ref/bif_builtin_expect.html
// https://stackoverflow.com/questions/7346929/what-is-the-advantage-of-gccs-builtin-expect-in-if-else-statements
#define EKON_LIKELY(x) __builtin_expect(x, 1)
#define EKON_UNLIKELY(x) __builtin_expect(x, 0)

#elif EKON_EXPECT_MODE == 2
// msvc
#define EKON_LIKELY(x) x
#define EKON_UNLIKELY(x) x

#elif EKON_EXPECT_MODE == 3
// Other compilers maybe
#endif

// ...
// Long API
struct EkonAllocator *ekonAllocatorNew();
char *ekonAllocatorAlloc(struct EkonAllocator *a, uint32_t size);
void ekonAllocatorRelease(struct EkonAllocator *rootAlloc);
struct EkonValue *ekonValueNew(struct EkonAllocator *alloc);

bool ekonError(char **message, struct EkonAllocator *a, const char *s,
               uint32_t index);
bool ekonDuplicateKeyError(char **message, struct EkonAllocator *a,
                           const char *s, uint32_t index, uint32_t keyLen);

/**
 * v - EkonValue
 * s - Source code string
 * errMessage - the pointer to errMessage char-array
 * schema - the pointer to the schema char-array. if schema != NULL,
 *          then the memory allocation will not happen in schema.
 *          this is a performance measure.
 * */
bool ekonValueParseFast(struct EkonValue *v, const char *s, char **errMessage,
                        char **schema);

bool ekonValueParseLen(struct EkonValue *v, const char *s, uint32_t len,
                       char **err, char **schema);
bool ekonValueParse(struct EkonValue *v, const char *s, char **err,
                    char **schema);

const char *ekonValueStringifyToJSON(const struct EkonValue *v,
                                     bool unEscapeString);
const char *ekonValueStringify(const struct EkonValue *v, bool unEscapeString);
const char *ekonBeautify(const char *src, char **err,
                         EkonBeautifyOptions options);

const char *ekonValueGetStrFast(const struct EkonValue *v, uint32_t *len);
const char *ekonValueGetUnEspaceStr(struct EkonValue *v);
const char *ekonValueGetStr(struct EkonValue *v, uint8_t *option);

const char *ekonValueGetNumFast(const struct EkonValue *v, uint32_t *len);
const char *ekonValueGetNumStr(struct EkonValue *v);
const double *ekonValueGetNum(struct EkonValue *v);
const double *ekonValueGetDouble(struct EkonValue *v);
const int *ekonValueGetInt(struct EkonValue *v);
const long *ekonValueGetLong(struct EkonValue *v);
const long long *ekonValueGetLongLong(struct EkonValue *v);

const bool *ekonValueGetBool(const struct EkonValue *v);

bool ekonValueIsNull(const struct EkonValue *v);

const char *ekonValueGetKey(struct EkonValue *v, uint8_t *option,
                            uint32_t *keyLength);
const char *ekonValueGetUnEspaceKey(struct EkonValue *v);
const char *ekonValueGetKeyFast(const struct EkonValue *v, uint32_t *len);

struct EkonValue *ekonValueObjGet(const struct EkonValue *v, const char *key);
struct EkonValue *ekonValueObjGetLen(const struct EkonValue *v, const char *key,
                                     uint32_t len);

const EkonType *ekonValueType(const struct EkonValue *v);
uint32_t ekonValueSize(const struct EkonValue *v);

struct EkonValue *ekonValueArrayGet(const struct EkonValue *v, uint32_t index);

struct EkonValue *ekonValueBegin(const struct EkonValue *v);
struct EkonValue *ekonValueNext(const struct EkonValue *v);

bool ekonValueCopyFrom(struct EkonValue *v, const struct EkonValue *vv);

struct EkonValue *ekonValueCopy(const struct EkonValue *v);
bool ekonValueMove(struct EkonValue *v);

bool ekonValueSetNull(struct EkonValue *v);

bool ekonValueSetBool(struct EkonValue *v, bool b);

bool ekonValueSetNumStrFast(struct EkonValue *v, const char *num);
bool ekonValueSetNumStrLenFast(struct EkonValue *v, const char *num,
                               uint32_t len);
bool ekonValueSetNumStr(struct EkonValue *v, const char *num);
bool ekonValueSetNumStrLen(struct EkonValue *v, const char *num, uint32_t len);
bool ekonValueSetNum(struct EkonValue *v, const double d);
bool ekonValueSetDouble(struct EkonValue *v, const double d);
bool ekonValueSetInt(struct EkonValue *v, const int n);
bool ekonValueSetLong(struct EkonValue *v, const long d);
bool ekonValueSetLongLong(struct EkonValue *v, const long long d);

bool ekonValueSetStrFast(struct EkonValue *v, const char *str, uint8_t option);
bool ekonValueSetStrLenFast(struct EkonValue *v, const char *str, uint32_t len);
bool ekonValueSetStr(struct EkonValue *v, const char *str);
bool ekonValueSetStrLen(struct EkonValue *v, const char *str, uint32_t len);
bool ekonValueSetStrEscape(struct EkonValue *v, const char *str,
                           uint8_t option);
bool ekonValueSetStrLenEscape(struct EkonValue *v, const char *str,
                              uint32_t len, uint8_t option);

bool ekonValueSetKeyFast(struct EkonValue *v, const char *key, uint8_t option,
                         bool replace);
bool ekonValueSetKeyLenFast(struct EkonValue *v, const char *key, uint32_t len,
                            bool replce);
bool ekonValueSetKey(struct EkonValue *v, const char *key, bool replace);
bool ekonValueSetKeyLen(struct EkonValue *v, const char *key, uint32_t len,
                        bool replace);
bool ekonValueSetKeyEscape(struct EkonValue *v, const char *key, uint8_t option,
                           bool replace);
bool ekonValueSetKeyLenEscape(struct EkonValue *v, const char *key,
                              uint32_t len, uint8_t option, bool replace);

bool ekonValueSetArray(struct EkonValue *v);

bool ekonValueSetObj(struct EkonValue *v);

bool ekonValueSetFast(struct EkonValue *v, struct EkonValue *vv);
bool ekonValueSet(struct EkonValue *v, const struct EkonValue *vv);

bool ekonValueObjAddFast(struct EkonValue *v, struct EkonValue *vv);
bool ekonValueObjAdd(struct EkonValue *v, const struct EkonValue *vv);

bool ekonValueArrayAddFast(struct EkonValue *v, struct EkonValue *vv);
bool ekonValueArrayAdd(struct EkonValue *v, const struct EkonValue *vv);

bool ekonValueArrayDel(struct EkonValue *v, uint32_t index);

bool ekonValueObjDel(struct EkonValue *v, const char *key);

bool ekonSkin(const char c);
bool ekonConsumeComment(const char *s, uint32_t *index);

#ifndef EKON_SHORT_API
#define EKON_SHORT_API 1
#endif
#if EKON_SHORT_API == 1

void debPrintStr(const char *s, uint32_t index, uint32_t size);

// Allocator and Value
typedef struct EkonAllocator Allocator;
typedef struct EkonValue Value;

// API declarations
static inline Allocator *NewAllocator() { return ekonAllocatorNew(); }
static inline void ReleaseAllocator(Allocator *rootAlloc) {
  ekonAllocatorRelease(rootAlloc);
}
static inline Value *NewValue(Allocator *alloc) { return ekonValueNew(alloc); }
static inline bool ParseFast(Value *v, const char *s, char **errMes,
                             char **schema) {
  return ekonValueParseFast(v, s, errMes, schema);
}
static inline bool ParseLen(Value *v, const char *s, uint32_t len, char **err,
                            char **schema) {
  return ekonValueParseLen(v, s, len, err, schema);
}
static inline bool Parse(Value *v, const char *s, char **err, char **schema) {
  return ekonValueParse(v, s, err, schema);
}
static inline const char *Stringify(const Value *v, bool unEscapeString) {
  return ekonValueStringify(v, unEscapeString);
}
static inline const char *StringifyToJSON(const Value *v, bool unEscapeString) {
  return ekonValueStringifyToJSON(v, unEscapeString);
}
static inline const char *GetStrFast(const Value *v, uint32_t *len) {
  return ekonValueGetStrFast(v, len);
}
static inline const char *GetUnEscapeStr(Value *v) {
  return ekonValueGetUnEspaceStr(v);
}
static inline const char *GetStr(Value *v, uint8_t *option) {
  return ekonValueGetStr(v, option);
}
static inline const char *GetNumFast(const Value *v, uint32_t *len) {
  return ekonValueGetNumFast(v, len);
}
static inline const char *GetNumStr(Value *v) { return ekonValueGetNumStr(v); }
static inline const double *GetNum(Value *v) { return ekonValueGetNum(v); }
static inline const double *GetDouble(Value *v) {
  return ekonValueGetDouble(v);
}
static inline const int *GetInt(Value *v) { return ekonValueGetInt(v); }
static inline const long *GetLong(Value *v) { return ekonValueGetLong(v); }
static inline const long long *GetLongLong(Value *v) {
  return ekonValueGetLongLong(v);
}
static inline const bool *GetBool(const Value *v) {
  return ekonValueGetBool(v);
}
static inline bool IsNull(const Value *v) { return ekonValueIsNull(v); }
static inline const char *GetKey(Value *v, uint8_t *option, uint32_t *keyLen) {
  return ekonValueGetKey(v, option, keyLen);
}
static inline const char *GetUnEscapeKey(Value *v) {
  return ekonValueGetUnEspaceKey(v);
}
static inline const char *GetKeyFast(Value *v, uint32_t *len) {
  return ekonValueGetKeyFast(v, len);
}
static inline const Value *ObjGet(const Value *v, const char *key) {
  return ekonValueObjGet(v, key);
}
static inline const Value *ObjGetLen(const Value *v, const char *key,
                                     uint32_t len) {
  return ekonValueObjGetLen(v, key, len);
}
static inline const EkonType *Type(const Value *v) { return ekonValueType(v); }
static inline uint32_t Size(const Value *v) { return ekonValueSize(v); }
static inline Value *ArrayGet(const Value *v, uint32_t index) {
  return ekonValueArrayGet(v, index);
};
static inline Value *Begin(Value *v) { return ekonValueBegin(v); }
static inline Value *Next(Value *v) { return ekonValueNext(v); }
static inline Value *Copy(Value *v) { return ekonValueCopy(v); }
static inline bool Move(Value *v) { return ekonValueMove(v); }
static inline bool SetNull(Value *v) { return ekonValueSetNull(v); }
static inline bool SetBool(Value *v, bool b) { return ekonValueSetBool(v, b); }
static inline bool SetNumStrFast(Value *v, const char *num) {
  return ekonValueSetNumStrFast(v, num);
}
static inline bool SetNumStrLenFast(Value *v, const char *num, uint32_t len) {
  return ekonValueSetNumStrLenFast(v, num, len);
}
static inline bool SetNumStr(Value *v, const char *num) {
  return ekonValueSetNumStr(v, num);
}
static inline bool SetNumStrLen(Value *v, const char *num, uint32_t len) {
  return ekonValueSetStrLen(v, num, len);
}
static inline bool SetNum(Value *v, const double d) {
  return ekonValueSetNum(v, d);
}
static inline bool SetDouble(Value *v, const double d) {
  return ekonValueSetDouble(v, d);
}
static inline bool SetInt(Value *v, const int d) {
  return ekonValueSetInt(v, d);
}
static inline bool SetLong(Value *v, const long d) {
  return ekonValueSetLong(v, d);
}
static inline bool SetLongLong(Value *v, const long d) {
  return ekonValueSetLongLong(v, d);
}
static inline bool SetStrFast(Value *v, const char *str, uint8_t option) {
  return ekonValueSetStrFast(v, str, option);
}
static inline bool SetStrLenFast(Value *v, const char *str, uint32_t len) {
  return ekonValueSetStrLenFast(v, str, len);
}
static inline bool SetStr(Value *v, const char *str) {
  return ekonValueSetStr(v, str);
}
static inline bool SetStrLen(Value *v, const char *str, uint32_t len) {
  return ekonValueSetStrLen(v, str, len);
}
static inline bool SetStrEscape(Value *v, const char *str, uint8_t option) {
  return ekonValueSetStrEscape(v, str, option);
}
static inline bool SetStrLenEscape(Value *v, const char *str, uint32_t len,
                                   uint8_t option) {
  return ekonValueSetStrLenEscape(v, str, len, option);
}
static inline bool SetKeyFast(Value *v, const char *key, uint8_t option,
                              bool replace) {
  return ekonValueSetKeyFast(v, key, option, replace);
}
static inline bool SetKeyLenFast(Value *v, const char *key, uint32_t len,
                                 bool replace) {
  return ekonValueSetKeyLenFast(v, key, len, replace);
}
static inline bool SetKey(Value *v, const char *key, bool replace) {
  return ekonValueSetKey(v, key, replace);
}
static inline bool SetKeyLen(Value *v, const char *key, uint32_t len,
                             bool replace) {
  return ekonValueSetKeyLen(v, key, len, replace);
}
static inline bool SetKeyEscape(Value *v, const char *key, uint8_t option,
                                bool replace) {
  return ekonValueSetKeyEscape(v, key, option, replace);
}
static inline bool SetKeyLenEscape(Value *v, const char *key, uint32_t len,
                                   uint8_t option, bool replace) {
  return ekonValueSetKeyLenEscape(v, key, len, option, replace);
}
static inline bool SetArray(Value *v) { return ekonValueSetArray(v); }
static inline bool SetObj(Value *v) { return ekonValueSetObj(v); }
static inline bool SetFast(Value *v, Value *vv) {
  return ekonValueSetFast(v, vv);
}
static inline bool Set(Value *v, const Value *vv) {
  return ekonValueSet(v, vv);
}
static inline bool ObjAddFast(Value *v, Value *vv) {
  return ekonValueObjAddFast(v, vv);
}
static inline bool ObjAdd(Value *v, const Value *vv) {
  return ekonValueObjAdd(v, vv);
}
static inline bool ArrayAddFast(Value *v, Value *vv) {
  return ekonValueArrayAddFast(v, vv);
}
static inline bool ArrayAdd(Value *v, const Value *vv) {
  return ekonValueArrayAdd(v, vv);
}
static inline bool ArrayDel(Value *v, uint32_t len) {
  return ekonValueArrayDel(v, len);
}
static inline bool ObjDel(Value *v, const char *key) {
  return ekonValueObjDel(v, key);
}
#endif

#endif
