#ifndef EKON_H
#define EKON_H

#include <stdbool.h> // import bool, true, false
#include <stdint.h>  // import u32
#include <stdio.h>   // import snprintf
#include <stdlib.h>  // import atof, atoi, atol, atoll, malloc, free
#include <string.h>  // import memcpy

#include "common.h"

// A few types I use regularly
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

// from hashmap.h

static const bool ekonTrue = true;
static const bool ekonFalse = false;

// enumerate types with numbers
/* enum _EkonType { */
/*   EKON_TYPE_BOOL, */
/*   EKON_TYPE_ARRAY, */
/*   EKON_TYPE_OBJECT, */
/*   EKON_TYPE_STRING, */
/*   EKON_TYPE_NULL, */
/*   EKON_TYPE_NUMBER, */
/* }; */
/* #define EkonType enum _EkonType */

// The primary json node
/* struct { */
/*   EkonType ekonType; */
/*   uint8_t option; */
/*   const char *key; */
/*   u32 keyLen; */

/*   EkonHashmap *keyTable; // duplicate prevention & faster retrieval of
 * objects */

/*   // pointer to (pointer to the current node in `keyTable`) */
/*   struct _EkonNode **node; */

/*   union { */
/*     struct _EkonNode *node; */
/*     const char *str; */
/*   } value; */
/*   u32 len; // string length */
/*   struct _EkonNode *next; */
/*   struct _EkonNode *prev; */
/*   struct _EkonNode *father; */
/*   struct _EkonNode *end; */
/* } _EkonNode; */
/* #define EkonNode struct _EkonNode */

typedef enum {
  EKON_IS_KEY_SPACED = 1 << 0,
  EKON_IS_KEY_MULTILINED = 1 << 1,
  EKON_IS_STR_SPACED = 1 << 2,
  EKON_IS_KEY_ESCAPABLE = 1 << 3,
  EKON_IS_STR_MULTILINED = 1 << 4,
  EKON_IS_STR_ESCAPABLE = 1 << 5,
  EKON_IS_NUM_BINARY = 1 << 6,
  EKON_IS_NUM_OCTAL = 1 << 7,
  EKON_IS_NUM_DECIMAL = 1 << 8,
  EKON_IS_NUM_HEXADECIMAL = 1 << 9,
  EKON_IS_NUM_FLOAT = 1 << 10,
  EKON_IS_NUM_INT = 1 << 11
} EKON_NODE_OPTIONS;

#define EkonOption uint16_t

/* // Node for allocator */
/* struct _EkonANode { */
/*   char *data; */
/*   u32 size; */
/*   u32 pos; */
/*   struct _EkonANode *next; */
/* }; */
/* #define EkonANode struct _EkonANode */

/* // Allocator */
/* struct _EkonAllocator { */
/*   EkonANode *root; */
/*   EkonANode *end; */
/* }; */
/* #define EkonAllocator struct _EkonAllocator */

// Ekon value
typedef struct {
  EkonNode *n;
  EkonAllocator *a;
} EkonValue;

typedef struct EkonBeautifyOptions {
  bool unEscapeString;
  bool asJSON;
  bool preserveComments;
} EkonBeautifyOptions;

struct _EkonString {
  char *data;
  u32 pos;
  u32 size;
  EkonAllocator *a;
};
#define EkonString struct _EkonString

static const u32 ekonDelta = 2;
static const u32 ekonAllocatorInitMemSize = 1024 * 4;
static const u32 ekonStringInitMemSize = 1024;
static const u32 ekonStringCacheInitMemSize = 128;

#ifndef EKON_MEMORY_NODE
#define EKON_MEMORY_NODE 1
#endif

#if EKON_MEMORY_NODE == 1
static inline void *ekonNew(u32 size) { return malloc(size); }
static inline void ekonFree(void *pointer) { free(pointer); }
#elif EKON_MEMORY_NODE == 2
static u32 ekonAllocMemorySize = 0, ekonAllocMemoryCount = 0,
           ekonFreeMemoryCount = 0;
void *ekonNew(u32 size) {
  return ekonAllocMemorySize += size, ekonAllocMemoryCount += 1, malloc(size);
}
void ekonFree(void *ptr) { freeMemoryCount += 1, free(ptr); }
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
EkonAllocator *ekonAllocatorNew();
char *ekonAllocatorAlloc(EkonAllocator *a, u32 size);
void ekonAllocatorRelease(EkonAllocator *rootAlloc);
EkonValue *ekonValueNew(EkonAllocator *alloc);

bool ekonError(char **outMessage, const char *s, const u32 index);
bool ekonDuplicateKeyError(char **outMessage, const char *s, const u32 index,
                           const u32 keyLen);

/**
 * v - EkonValue
 * s - Source code string
 * errMessage - the pointer to errMessage char-array
 * schema - the pointer to the schema char-array. if schema != NULL,
 *          then the memory allocation will not happen in schema.
 *          this is a performance measure. schema as to be NULL for schema.
 * */
bool ekonValueParseFast(EkonValue *v, const char *s, char **errMessage,
                        char **schema);

bool ekonValueParseLen(EkonValue *v, const char *s, u32 len, char **err,
                       char **schema);
bool ekonValueParse(EkonValue *v, const char *s, char **err, char **schema);

const char *ekonValueStringifyToJSON(const EkonValue *v, bool unEscapeString);
const char *ekonValueStringify(const EkonValue *v, bool unEscapeString);
const char *ekonBeautify(const char *src, char **err,
                         EkonBeautifyOptions options);

const char *ekonValueGetStrFast(const EkonValue *v, u32 *len);
const char *ekonValueGetUnEspaceStr(EkonValue *v);
const char *ekonValueGetStr(EkonValue *v, uint8_t *option);

const char *ekonValueGetNumFast(const EkonValue *v, u32 *len);
const char *ekonValueGetNumStr(EkonValue *v);
const bool ekonValueGetNum(EkonValue *v, double *d);
const bool ekonValueGetDouble(EkonValue *v, double *d);
const bool ekonValueGetInt(EkonValue *v, int *i);
const bool ekonValueGetLong(EkonValue *v, long *l);
const bool ekonValueGetLongLong(EkonValue *v, long long *ll);

const bool *ekonValueGetBool(const EkonValue *v);

bool ekonValueIsNull(const EkonValue *v);

const char *ekonValueGetKey(EkonValue *v, uint8_t *option, u32 *keyLength);
const char *ekonValueGetUnEspaceKey(EkonValue *v);
const char *ekonValueGetKeyFast(const EkonValue *v, u32 *len);

EkonValue *ekonValueObjGet(const EkonValue *v, const char *key);
EkonValue *ekonValueObjGetLen(const EkonValue *v, const char *key, u32 len);

const EkonType *ekonValueType(const EkonValue *v);
u32 ekonValueSize(const EkonValue *v);

EkonValue *ekonValueArrayGet(const EkonValue *v, u32 index);

EkonValue *ekonValueBegin(const EkonValue *v);
EkonValue *ekonValueNext(const EkonValue *v);

bool ekonValueCopyFrom(EkonValue *v, const EkonValue *vv);

EkonValue *ekonValueCopy(const EkonValue *v);
bool ekonValueMove(EkonValue *v);

bool ekonValueSetNull(EkonValue *v);

bool ekonValueSetBool(EkonValue *v, bool b);

bool ekonValueSetNumStrFast(EkonValue *v, const char *num);
bool ekonValueSetNumStrLenFast(EkonValue *v, const char *num, u32 len);
bool ekonValueSetNumStr(EkonValue *v, const char *num);
bool ekonValueSetNumStrLen(EkonValue *v, const char *num, u32 len);
bool ekonValueSetNum(EkonValue *v, const double d);
bool ekonValueSetDouble(EkonValue *v, const double d);
bool ekonValueSetInt(EkonValue *v, const int n);
bool ekonValueSetLong(EkonValue *v, const long d);
bool ekonValueSetLongLong(EkonValue *v, const long long d);

bool ekonValueSetStrFast(EkonValue *v, const char *str, u16 option);
bool ekonValueSetStrLenFast(EkonValue *v, const char *str, u32 len);
bool ekonValueSetStr(EkonValue *v, const char *str);
bool ekonValueSetStrLen(EkonValue *v, const char *str, u32 len);
bool ekonValueSetStrEscape(EkonValue *v, const char *str, u16 option);
bool ekonValueSetStrLenEscape(EkonValue *v, const char *str, u32 len,
                              u16 option);

bool ekonValueSetKeyFast(EkonValue *v, const char *key, u16 option,
                         bool replace);
bool ekonValueSetKeyLenFast(EkonValue *v, const char *key, u32 len,
                            bool replce);
bool ekonValueSetKey(EkonValue *v, const char *key, bool replace);
bool ekonValueSetKeyLen(EkonValue *v, const char *key, u32 len, bool replace);
bool ekonValueSetKeyEscape(EkonValue *v, const char *key, u16 option,
                           bool replace);
bool ekonValueSetKeyLenEscape(EkonValue *v, const char *key, u32 len,
                              u16 option, bool replace);

bool ekonValueSetArray(EkonValue *v);

bool ekonValueSetObj(EkonValue *v);

bool ekonValueSetFast(EkonValue *v, EkonValue *vv);
bool ekonValueSet(EkonValue *v, const EkonValue *vv);

bool ekonValueObjAddFast(EkonValue *v, EkonValue *vv);
bool ekonValueObjAdd(EkonValue *v, const EkonValue *vv);

bool ekonValueArrayAddFast(EkonValue *v, EkonValue *vv);
bool ekonValueArrayAdd(EkonValue *v, const EkonValue *vv);

bool ekonValueArrayDel(EkonValue *v, u32 index);

bool ekonValueObjDel(EkonValue *v, const char *key);

bool ekonSkin(const char c);
bool ekonCheckNum(const char *s, u32 *outLen);

#ifdef DEBUG_FUNC
// debug functions
void debPrintEkonType(const EkonType type);
void debPrintNodeMin(const EkonNode *node);
void debPrintStrM(const char *s, u32 index, const char *mes, u32 size);
void debPrintChar(const char c);
void debPrintCharM(const char c, const char *mes);
void debPrintBool(const bool b);
void debPrintBoolM(const bool b, const char *mes);
void printOption(u16 option, char *delimiter);
void debPrintStr(const char *s, u32 index, u32 size);
void debPrintStrMin(const char *s, u32 size);
void debPrintTab(int depth);
void debPrintNodeSing(const EkonNode *n, int depth);
void debPrintNode(const EkonNode *n, int depth);
void debPrintHashmap(const EkonHashmap *hashmap);
#endif

#endif
