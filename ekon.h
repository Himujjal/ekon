#ifndef EKON_H
#define EKON_H

#include <stdbool.h> // import bool, true, false
#include <stdint.h>  // import uint32_t
#include <stdio.h>   // import snprintf
#include <stdlib.h>  // import atof, atoi, atol, atoll, malloc, free
#include <string.h>  // import memcpy

static const bool ekonTrue = true;
static const bool ekonFalse = false;

// enumerate types with numbers
typedef enum {
  EKON_TYPE_BOOL,
  EKON_TYPE_ARRAY,
  EKON_TYPE_OBJECT,
  EKON_TYPE_STRING,
  EKON_TYPE_NULL,
  EKON_TYPE_NUMBER,
} EkonType;

// store constants in char arrays
static const char *ekonStrTrue = "true";
static const char *ekonStrFalse = "false";
static const char *ekonStrNull = "null";

static const uint32_t ekonDelta = 2;
static const uint32_t ekonAllocatorInitMemSize = 1024 * 4;
static const uint32_t ekonStringInitMemSize = 1024;
static const uint32_t ekonStringCacheInitMemSize = 128;

#ifndef EKON_MEMORY_NODE
#define EKON_MEMORY_NODE 1
#endif

#if EKON_MEMORY_NODE == 1

static inline void *ekonNew(uint32_t size) { return malloc(size); }
static inline void ekonFree(void *pointer) { free(pointer); }

#elif EKON_MEMORY_NODE == 2

static uint32_t ekonAllocMemorySize = 0, ekonAllocMemoryCount = 0,
                ekonFreeMemoryCount = 0;
static inline void *ekonNew(uint32_t size) {
  return ekonAllocMemorySize += size, ekonAllocMemoryCount += 1, malloc(size);
}
static inline void ekonFree(void *ptr) { freeMemoryCount += 1, free(ptr); }

#elif EKON_MEMORY_NODE == 3
// TODO: Understand this part later
#endif

// TODO: Remove this part later ------------------
#define EKON_LIKELY(x) __builtin_expect(x, 1)
#define EKON_UNLIKELY(x) __builtin_expect(x, 0)
// -----------------------------------------

#if EKON_EXPECT_MODE == 1
// Only for gcc, clang __builtin_expect is for compiler optimizations
// __builtin_expect(x, n) will return x
// but if n == 1, compiler optimizes knowing the fact that it is more likely
// that x will be equal to 1 most of the time (statistically) if n == 0, it is
// more likely that x == 0. This is some heavy level compiler shit!
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
struct EkonAllocator;
struct EkonValue;

// Long API
static inline struct EkonAllocator *ekonAllocatorNew();
static inline void ekonAllocatorRelease(struct EkonAllocator *rootAlloc);
static inline struct EkonValue *ekonValueNew(struct EkonAllocator *alloc);

static inline bool ekonError(char **mes, uint32_t line, uint32_t pos, char c);
static inline bool ekonValueParseFast(struct EkonValue *v, const char *s,
                                      char **err);

static inline bool ekonValueParseLen(struct EkonValue *v, const char *s,
                                     uint32_t len, char **err);
static inline bool ekonValueParse(struct EkonValue *v, const char *s,
                                  char **err);

static inline const char *ekonValueStringifyToJSON(const struct EkonValue *v);
static inline const char *ekonValueStringify(const struct EkonValue *v);

static inline const char *ekonValueGetStrFast(const struct EkonValue *v,
                                              uint32_t *len);
// TODO: Work on only allowing multiple lined string later instead of unescaped
static inline const char *ekonValueGetUnEspaceStr(struct EkonValue *v);
static inline const char *ekonValueGetStr(struct EkonValue *v);

static inline void ekonUpdateLine(uint32_t *line, uint32_t *pos, char c);
static inline const char *ekonValueGetNumFast(const struct EkonValue *v,
                                              uint32_t *len);
static inline const char *ekonValueGetNumStr(struct EkonValue *v);
static inline const double *ekonValueGetNum(struct EkonValue *v);
static inline const double *ekonValueGetDouble(struct EkonValue *v);
static inline const int *ekonValueGetInt(struct EkonValue *v);
static inline const long *ekonValueGetLong(struct EkonValue *v);
static inline const long long *ekonValueGetLongLong(struct EkonValue *v);

static inline const bool *ekonValueGetBool(const struct EkonValue *v);

static inline bool ekonValueIsNull(const struct EkonValue *v);

static inline const char *ekonValueGetKey(struct EkonValue *v);
static inline const char *ekonValueGetUnEspaceKey(struct EkonValue *v);
static inline const char *ekonValueGetKeyFast(const struct EkonValue *v,
                                              uint32_t *len);

static inline struct EkonValue *ekonValueObjGet(const struct EkonValue *v,
                                                const char *key);
static inline struct EkonValue *
ekonValueObjGetLen(const struct EkonValue *v, const char *key, uint32_t len);

static inline const EkonType *ekonValueType(const struct EkonValue *v);
static inline uint32_t ekonValueSize(const struct EkonValue *v);

static inline struct EkonValue *ekonValueArrayGet(const struct EkonValue *v,
                                                  uint32_t index);

static inline struct EkonValue *ekonValueBegin(const struct EkonValue *v);
static inline struct EkonValue *ekonValueNext(const struct EkonValue *v);

static inline bool ekonValueCopyFrom(struct EkonValue *v,
                                     const struct EkonValue *vv);

static inline struct EkonValue *ekonValueCopy(const struct EkonValue *v);
static inline bool ekonValueMove(struct EkonValue *v);

static inline bool ekonValueSetNull(struct EkonValue *v);

static inline bool ekonValueSetBool(struct EkonValue *v, bool b);

static inline bool ekonValueSetNumStrFast(struct EkonValue *v, const char *num);
static inline bool ekonValueSetNumStrLenFast(struct EkonValue *v,
                                             const char *num, uint32_t len);
static inline bool ekonValueSetNumStr(struct EkonValue *v, const char *num);
static inline bool ekonValueSetNumStrLen(struct EkonValue *v, const char *num,
                                         uint32_t len);
static inline bool ekonValueSetNum(struct EkonValue *v, const double d);
static inline bool ekonValueSetDouble(struct EkonValue *v, const double d);
static inline bool ekonValueSetInt(struct EkonValue *v, const int n);
static inline bool ekonValueSetLong(struct EkonValue *v, const long d);
static inline bool ekonValueSetLongLong(struct EkonValue *v, const long long d);

static inline bool ekonValueSetStrFast(struct EkonValue *v, const char *str);
static inline bool ekonValueSetStrLenFast(struct EkonValue *v, const char *str,
                                          uint32_t len);
static inline bool ekonValueSetStr(struct EkonValue *v, const char *str);
static inline bool ekonValueSetStrLen(struct EkonValue *v, const char *str,
                                      uint32_t len);
static inline bool ekonValueSetStrEscape(struct EkonValue *v, const char *str);
static inline bool ekonValueSetStrLenEscape(struct EkonValue *v,
                                            const char *str, uint32_t len);

static inline bool ekonValueSetKeyFast(struct EkonValue *v, const char *key);
static inline bool ekonValueSetKeyLenFast(struct EkonValue *v, const char *key,
                                          uint32_t len);
static inline bool ekonValueSetKey(struct EkonValue *v, const char *key);
static inline bool ekonValueSetKeyLen(struct EkonValue *v, const char *key,
                                      uint32_t len);
static inline bool ekonValueSetKeyEscape(struct EkonValue *v, const char *key);
static inline bool ekonValueSetKeyLenEscape(struct EkonValue *v,
                                            const char *key, uint32_t len);

static inline bool ekonValueSetArray(struct EkonValue *v);

static inline bool ekonValueSetObj(struct EkonValue *v);

static inline bool ekonValueSetFast(struct EkonValue *v, struct EkonValue *vv);
static inline bool ekonValueSet(struct EkonValue *v,
                                const struct EkonValue *vv);

static inline bool ekonValueObjAddFast(struct EkonValue *v,
                                       struct EkonValue *vv);
static inline bool ekonValueObjAdd(struct EkonValue *v,
                                   const struct EkonValue *vv);

static inline bool ekonValueArrayAddFast(struct EkonValue *v,
                                         struct EkonValue *vv);
static inline bool ekonValueArrayAdd(struct EkonValue *v,
                                     const struct EkonValue *vv);

static inline bool ekonValueArrayDel(struct EkonValue *v, uint32_t index);

static inline bool ekonValueObjDel(struct EkonValue *v, const char *key);

static inline bool ekonCheckIfComment(const char *str, uint32_t index);
static inline bool ekonParseTillCommentEnd(const char *str, uint32_t *index);
static inline bool ekonConsumeComment(const char *s, uint32_t *index,
                                      uint32_t *line, uint32_t *pos);

// ...
#ifndef EKON_SHORT_API
#define EKON_SHORT_API 1
#endif
#if EKON_SHORT_API == 1

// Allocator and Value
typedef struct EkonAllocator Allocator;
typedef struct EkonValue Value;

// API declarations
static inline Allocator *NewAllocator() { return ekonAllocatorNew(); }
static inline void ReleaseAllocator(Allocator *rootAlloc) {
  ekonAllocatorRelease(rootAlloc);
}
static inline Value *NewValue(Allocator *alloc) { return ekonValueNew(alloc); }
static inline bool ParseFast(Value *v, const char *s, char **err) {
  return ekonValueParseFast(v, s, err);
}
static inline bool ParseLen(Value *v, const char *s, uint32_t len, char **err) {
  return ekonValueParseLen(v, s, len, err);
}
static inline bool Parse(Value *v, const char *s, char **err) {
  return ekonValueParse(v, s, err);
}
static inline const char *Stringify(const Value *v) {
  return ekonValueStringify(v);
}
static inline const char *StringifyToJSON(const Value *v) {
  return ekonValueStringifyToJSON(v);
}
static inline const char *GetStrFast(const Value *v, uint32_t *len) {
  return ekonValueGetStrFast(v, len);
}
static inline const char *GetUnEscapeStr(Value *v) {
  return ekonValueGetUnEspaceStr(v);
}
static inline const char *GetStr(Value *v) { return ekonValueGetStr(v); }
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
static inline const char *GetKey(Value *v) { return ekonValueGetKey(v); }
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
static inline bool SetStrFast(Value *v, const char *str) {
  return ekonValueSetStrFast(v, str);
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
static inline bool SetStrEscape(Value *v, const char *str) {
  return ekonValueSetStrEscape(v, str);
}
static inline bool SetStrLenEscape(Value *v, const char *str, uint32_t len) {
  return ekonValueSetStrLenEscape(v, str, len);
}
static inline bool SetKeyFast(Value *v, const char *key) {
  return ekonValueSetKeyFast(v, key);
}
static inline bool SetKeyLenFast(Value *v, const char *key, uint32_t len) {
  return ekonValueSetKeyLenFast(v, key, len);
}
static inline bool SetKey(Value *v, const char *key) {
  return ekonValueSetKey(v, key);
}
static inline bool SetKeyLen(Value *v, const char *key, uint32_t len) {
  return ekonValueSetKeyLen(v, key, len);
}
static inline bool SetKeyEscape(Value *v, const char *key) {
  return ekonValueSetKeyEscape(v, key);
}
static inline bool SetKeyLenEscape(Value *v, const char *key, uint32_t len) {
  return ekonValueSetKeyLenEscape(v, key, len);
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

// copy a string from one pointer to another
static inline void ekonCopy(const char *src, uint32_t len, char *des) {
  memcpy(des, src, len);
}

static inline uint32_t ekonStrLen(const char *str) {
  return (uint32_t)strlen(str);
}

static inline int ekonStrToInt(const char *str) { return atoi(str); }

static inline long ekonStrToLong(const char *str) { return atol(str); }
static inline long long ekonStrToLongLong(const char *str) {
  return atoll(str);
}
static inline double ekonStrToDouble(const char *str) { return atof(str); }
static inline uint32_t ekonIntToStr(int n, char *buff) {
  return snprintf(buff, 12, "%d", n);
}
static inline uint32_t ekonLongToStr(long n, char *buff) {
  return snprintf(buff, 24, "%ld", n);
}
static inline uint32_t ekonLongLongToStr(long long n, char *buff) {
  return snprintf(buff, 24, "%lld", n);
}
static inline uint32_t ekonDoubleToStr(double n, char *buff) {
  return snprintf(buff, 32, "%.17g", n);
}

static inline bool ekonError(char **mes, uint32_t line, uint32_t pos, char c) {
  return snprintf(*mes, 100, "[%d:%d] ERROR at %c", line, pos, c) != 0;
}

// compare strings
static inline bool ekonStrIsEqual(const char *a, const char *b, uint32_t len) {
  uint32_t i;
  for (i = 0; EKON_LIKELY(i < len); ++i) {
    if (EKON_LIKELY(a[i] != b[i]))
      return false;
  }
  if (EKON_LIKELY(a[i] == 0))
    return true;
  return false;
}

// compare string with lengths
static inline bool ekonStrIsEqualLen(const char *a, uint32_t a_len,
                                     const char *b, uint32_t b_len) {
  if (EKON_LIKELY(a_len != b_len))
    return false;
  uint32_t i;
  for (i = 0; EKON_LIKELY(i < a_len); ++i) {
    if (EKON_LIKELY(a[i] != b[i]))
      return false;
  }
  return true;
}

// A Obj Node - Linked List
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

// Init new Allocator - API
static inline struct EkonAllocator *ekonAllocatorNew() {
  void *ptr = ekonNew(sizeof(struct EkonAllocator) + sizeof(struct EkonANode) +
                      ekonAllocatorInitMemSize);
  if (EKON_UNLIKELY(ptr == 0))
    return 0;
  struct EkonAllocator *alloc = (struct EkonAllocator *)ptr;
  alloc->root =
      (struct EkonANode *)((char *)ptr + sizeof(struct EkonAllocator));
  alloc->end = alloc->root;

  alloc->root->size = ekonAllocatorInitMemSize;
  alloc->root->data =
      (char *)ptr + sizeof(struct EkonAllocator) + sizeof(struct EkonANode);
  alloc->root->pos = 0;
  alloc->root->next = 0;
  return alloc;
}

// Free Allocator - Release
static inline void ekonAllocatorRelease(struct EkonAllocator *alloc) {
  struct EkonANode *next = alloc->root->next;
  while (EKON_LIKELY(next != 0)) {
    struct EkonANode *nn = next->next;
    ekonFree((void *)next);
    next = nn;
  }
  ekonFree((void *)alloc);
}

// append Child to the Allocator
static inline bool ekonAllocatorAppendChild(uint32_t init_size,
                                            struct EkonAllocator *alloc) {
  //
  void *ptr = ekonNew(sizeof(struct EkonANode) + init_size);
  if (EKON_UNLIKELY(ptr == 0))
    return false;

  struct EkonANode *node = (struct EkonANode *)ptr;
  node->size = init_size;
  node->data = (char *)ptr + sizeof(struct EkonANode);
  node->pos = 0;
  node->next = 0;
  alloc->end->next = node;
  alloc->end = node;
  return true;
}

// allocate Allocator
static inline char *ekonAllocatorAlloc(struct EkonAllocator *alloc,
                                       uint32_t size) {
  struct EkonANode *currNode = alloc->end;
  uint32_t s = currNode->size;
  if (EKON_UNLIKELY(currNode->pos + size > s)) {
    s *= ekonDelta;
    while (EKON_UNLIKELY(size > s))
      s *= ekonDelta;
    if (EKON_UNLIKELY(ekonAllocatorAppendChild(s, alloc) == false))
      return 0;
    currNode = alloc->end;
  }
  char *ret = currNode->data + currNode->pos;
  currNode->pos += size;
  return ret;
}

struct EkonString {
  char *data;
  uint32_t pos;
  uint32_t size;
  struct EkonAllocator *a;
};

static struct EkonString *ekonStringCache = 0;

static inline struct EkonString *ekonStringNew(struct EkonAllocator *alloc,
                                               uint32_t initSize) {
  struct EkonString *str = (struct EkonString *)ekonAllocatorAlloc(
      alloc, sizeof(struct EkonString) + initSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  str->size = initSize;
  str->data = (char *)str + sizeof(struct EkonString);
  str->pos = 0;
  str->a = alloc;
  return str;
}

static inline void ekonStringReset(struct EkonString *str) { str->pos = 0; }

static inline bool ekonStringAppendStr(struct EkonString *str, const char *s,
                                       uint32_t size) {
  uint32_t srcS = str->size;
  if (EKON_UNLIKELY(str->pos + size > srcS)) {
    srcS += ekonDelta;
    while (EKON_UNLIKELY(str->pos + size > srcS))
      srcS += ekonDelta;
    const char *srcD = str->data;
    str->data = (char *)ekonAllocatorAlloc(str->a, srcS);
    if (EKON_UNLIKELY(str->data == 0))
      return false;
    str->size = srcS;
    ekonCopy(srcD, str->pos, str->data);
  }
  ekonCopy(s, size, str->data + str->pos);
  str->pos += size;
  return true;
}

static inline bool ekonStringAppendChar(struct EkonString *str, const char c) {
  uint32_t srcS = str->size;
  if (EKON_UNLIKELY(str->pos + 1 > srcS)) {
    srcS *= ekonDelta;
    const char *srcD = str->data;
    str->data = (char *)ekonAllocatorAlloc(str->a, srcS);
    if (EKON_UNLIKELY(str->data == 0))
      return false;
    str->size = srcS;
    ekonCopy(srcD, str->pos, str->data);
  }
  *(str->data + str->pos) = c;
  str->pos += 1;
  return true;
}

static inline bool ekonStringAppendEnd(struct EkonString *str) {
  return ekonStringAppendChar(str, 0);
}

static inline const char *ekonStringStr(struct EkonString *str) {
  return str->data;
}

// The primary json node
struct EkonNode {
  EkonType type;
  const char *key;
  uint32_t keyLen;
  bool isKeySpaced;
  union {
    struct EkonNode *node;
    const char *str;
  } value;
  uint32_t len;
  bool isStrSpaced;
  bool isStrMultilined;
  struct EkonNode *next;
  struct EkonNode *prev;
  struct EkonNode *father;
  struct EkonNode *end;
};

// ekon value
struct EkonValue {
  struct EkonNode *n;
  struct EkonAllocator *a;
};

// allocate new value into the memory and store the memory pointer
// in the v->allocator (v->a) value
static inline struct EkonValue *ekonValueNew(struct EkonAllocator *alloc) {
  struct EkonValue *v =
      (struct EkonValue *)ekonAllocatorAlloc(alloc, sizeof(struct EkonValue));
  if (EKON_UNLIKELY(v == 0))
    return 0;
  v->a = alloc;
  v->n = 0;
  return v;
}

// add new value to EkonValue
static inline struct EkonValue *ekonValueInnerNew(struct EkonAllocator *alloc,
                                                  struct EkonNode *n) {
  struct EkonValue *v =
      (struct EkonValue *)ekonAllocatorAlloc(alloc, sizeof(struct EkonValue));
  if (EKON_UNLIKELY(v == 0))
    return 0;
  v->a = alloc;
  v->n = n;
  return v;
}

static inline void ekonUpdateLine(uint32_t *line, uint32_t *pos, char c) {
  if (c == '\n') {
    (*line)++;
    (*pos) = 1;
  }
}

// ignore spacing
static inline bool ekonSkin(const char c) {
  if (EKON_UNLIKELY(
          EKON_UNLIKELY(c == ' ') ||
          EKON_UNLIKELY(c == '\t' ||
                        EKON_UNLIKELY(c == '\n' || EKON_UNLIKELY(c == '\r')))))
    return true;
  return false;
}

static inline bool ekonConsume(const char c, const char *s, uint32_t *index);

static inline bool ekonConsumeWhiteChars(const char *s, uint32_t *index,
                                         uint32_t *line, uint32_t *pos) {
  while (ekonSkin(s[*index])) {
    ekonUpdateLine(line, pos, s[*index]);
    ++(*index);
  }

  if (s[*index] == '/') {
    if (ekonConsumeComment(s, index, line, pos) == false)
      return false;
  }
  while (ekonSkin(s[*index])) {
    ekonUpdateLine(line, pos, s[*index]);
    ++(*index);
  }

  return true;
}

// peek current character
static inline char ekonPeek(const char *s, uint32_t *index, uint32_t *line,
                            uint32_t *pos) {
  if (ekonConsumeWhiteChars(s, index, line, pos) == false)
    return 0;
  return s[(*index)++];
}

// consume current character
static inline bool ekonConsume(const char c, const char *s, uint32_t *index) {
  if (s[*index] == c) {
    ++(*index);
    return true;
  }
  return false;
}

// consume using likely
static inline bool ekonLikelyConsume(const char c, const char *s,
                                     uint32_t *index) {
  if (EKON_LIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// consume using unlikely
static inline bool ekonUnlikelyConsume(const char c, const char *s,
                                       uint32_t *index) {
  if (EKON_UNLIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// peek and consume using likely
static inline bool ekonLikelyPeekAndConsume(const char c, const char *s,
                                            uint32_t *index, uint32_t *line,
                                            uint32_t *pos) {
  if (ekonConsumeWhiteChars(s, index, line, pos) == false)
    return false;

  if (EKON_LIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// peek and consume using unlikely
static inline bool ekonUnlikelyPeekAndConsume(const char c, const char *s,
                                              uint32_t *index, uint32_t *line,
                                              uint32_t *pos) {
  if (ekonConsumeWhiteChars(s, index, line, pos) == false)
    return false;

  if (EKON_UNLIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// consume a comment
// multiline comments
static inline bool ekonConsumeComment(const char *s, uint32_t *index,
                                      uint32_t *line, uint32_t *pos) {
  char curr = s[(*index)];
  if (s[(*index)++] == '/' && s[(*index)++] == '/') {
    if (s[*index] == '\n') {
      ekonUpdateLine(line, pos, s[*index]);
      (*index)++;
      if (ekonConsumeWhiteChars(s, index, line, pos) == false)
        return false;
      return true;
    }

    if (curr == 0)
      return true;

    while (EKON_UNLIKELY(curr != '\n' && curr != 0))
      curr = s[(*index)++];

    if (ekonConsumeWhiteChars(s, index, line, pos) == false)
      return false;

    return true;
  }
  return false;
}

// consume 'false' string
static inline bool ekonConsumeFalse(const char *s, uint32_t *index) {
  if (EKON_LIKELY(*((uint32_t *)("alse")) == *((uint32_t *)(s + *index)))) {
    *index += 4;
    return true;
  }
  return false;
}

// consume 'true' string
static inline bool ekonConsumeTrue(const char *s, uint32_t *index) {
  if (EKON_LIKELY(*((uint32_t *)ekonStrTrue) ==
                  *((uint32_t *)(s + *index - 1)))) {
    *index += 3;
    return true;
  }
  return false;
}

// consume 'null' string
static inline bool ekonConsumeNull(const char *s, uint32_t *index) {
  if (EKON_LIKELY(*((uint32_t *)ekonStrNull) ==
                  *((uint32_t *)(s + *index - 1)))) {
    *index += 3;
    return true;
  }
  return false;
}

// check if its a hex character
static inline bool ekonHexCodePoint(const char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return 16;
}

// ekonValueGetUnEspaceStr
static inline uint32_t ekonHexCodePointForUnEscape(const char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return c - 'a' + 10;
}

// consume Hex one char
static inline bool ekonConsumeHexOne(const char *s, uint32_t *index,
                                     uint32_t *cp) {
  uint32_t tcp = ekonHexCodePoint(s[*index]);
  if (EKON_LIKELY(tcp < 16)) {
    *cp = *cp << 4;
    *cp += tcp;
    ++(*index);
    return true;
  }
  return false;
}

// valueGetUnEscapeStr
static inline void ekonConsumeHexOneForUnEscape(const char *s, uint32_t *index,
                                                uint32_t *cp) {
  *cp = *cp << 4;
  *cp += ekonHexCodePointForUnEscape(s[*index]);
  ++(*index);
  return;
}

// consume a hex code
static inline bool ekonConsumeHex(const char *s, uint32_t *index,
                                  uint32_t *cp) {
  if (EKON_LIKELY(EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)))) {
    return true;
  }
  return false;
}

// consume Hex for unescaped
static inline void ekonConsumeHexForUnEscape(const char *s, uint32_t *index,
                                             uint32_t *cp) {
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  return;
}

// ... apend string without having a length ...
static inline void ekonAppend(char *s, uint32_t *index, char c) {
  s[(*index)++] = c;
}

// ... append strings with length ...
static inline void ekonAppendLen(char *s, uint32_t *index, const char *str,
                                 uint32_t len) {
  ekonCopy(str, len, s + (*index));
  *index += len;
}

// Append UTF-8
static inline void ekonAppendUTF8(char *s, uint32_t *index,
                                  uint32_t codepoint) {
  // UTF8 and UNICODE support
  if (codepoint <= 0x7F)
    ekonAppend(s, index, (char)(codepoint & 0xFF));
  else if (codepoint <= 0x7FF) {
    ekonAppend(s, index, (char)(0xC0 | ((codepoint >> 6) & 0xFF)));
    ekonAppend(s, index, (char)(0x80 | ((codepoint & 0x3F))));
  } else if (codepoint <= 0xFFFF) {
    ekonAppend(s, index, (char)(0xE0 | ((codepoint >> 12) & (0xFF))));
    ekonAppend(s, index, (char)(0x80 | ((codepoint >> 6) & 0x3F)));
    ekonAppend(s, index, (char)(0x80 | (codepoint & 0x3F)));
  } else {
    ekonAppend(s, index, (char)(0xF0 | ((codepoint >> 18) & 0xFF)));
    ekonAppend(s, index, (char)(0x80 | ((codepoint >> 12) & 0x3F)));
    ekonAppend(s, index, (char)(0x80 | ((codepoint >> 6) & 0x3F)));
    ekonAppend(s, index, (char)(0x80 | (codepoint & 0x3F)));
  }
}

// Append At the End
static inline void ekonAppendEnd(char *s, uint32_t *index) {
  ekonAppend(s, index, 0);
}

// Unescape Str
static inline void ekonUnEscapeStr(const char *str, uint32_t len, char *s) {
  uint32_t sIndex = 0;
  uint32_t index;
  char c;
  for (index = 0; index < len;) {
    c = str[index];
    // .... EKON ...
    if (EKON_UNLIKELY(c == '\\')) {
      c = str[index + 1];
      switch (c) {
      case '"': {
        ekonAppend(s, &sIndex, '\"');
        index += 2;
        break;
      }
      case '\\': {
        ekonAppend(s, &sIndex, '\\');
        index += 2;
        break;
      }
      case 'b': {
        ekonAppend(s, &sIndex, '\b');
        index += 2;
        break;
      }
      case 'f': {
        ekonAppend(s, &sIndex, '\f');
        index += 2;
        break;
      }
      case 'n': {
        ekonAppend(s, &sIndex, '\n');
        index += 2;
        break;
      }
      case 'r': {
        ekonAppend(s, &sIndex, '\r');
        index += 2;
        break;
      }
      case 't': {
        ekonAppend(s, &sIndex, '\t');
        index += 2;
        break;
      }
      case '/': {
        ekonAppend(s, &sIndex, '/');
        index += 2;
        break;
      }
      case 'u': {
        index += 2;
        uint32_t cp = 0;
        ekonConsumeHexForUnEscape(str, &index, &cp);
        if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xDBFF)) {
          uint32_t cp1 = 0;
          index += 2;
          ekonConsumeHexForUnEscape(str, &index, &cp1);
          cp = (((cp - 0xD800) << 10) | (cp1 - 0xDC00)) + 0x10000;
        }
        ekonAppendUTF8(s, &sIndex, cp);
        break;
      }
      }
    } else {
      ekonAppend(s, &sIndex, c);
      index += 1;
    }
  }
  ekonAppendEnd(s, &sIndex);
  return;
}

struct EkonEscapeChar {
  const char *str;
  uint32_t len;
};

static const struct EkonEscapeChar ekonEscapeChars[256] = {
    {"\\u0000", 6}, {"\\u0001", 6}, {"\\u0002", 6}, {"\\u0003", 6},
    {"\\u0004", 6}, {"\\u0005", 6}, {"\\u0006", 6}, {"\\u0007", 6},
    {"\\b", 2},     {"\\t", 2},     {"\\n", 2},     {"\\u000b", 6},
    {"\\f", 2},     {"\\r", 2},     {"\\u000e", 6}, {"\\u000f", 6},
    {"\\u0010", 6}, {"\\u0011", 6}, {"\\u0012", 6}, {"\\u0013", 6},
    {"\\u0014", 6}, {"\\u0015", 6}, {"\\u0016", 6}, {"\\u0017", 6},
    {"\\u0018", 6}, {"\\u0019", 6}, {"\\u001a", 6}, {"\\u001b", 6},
    {"\\u001c", 6}, {"\\u001d", 6}, {"\\u001e", 6}, {"\\u001f", 6},
    {"\x20", 1},    {"\x21", 1},    {"\\\"", 2},    {"\x23", 1},
    {"\x24", 1},    {"\x25", 1},    {"\x26", 1},    {"\x27", 1},
    {"\x28", 1},    {"\x29", 1},    {"\x2a", 1},    {"\x2b", 1},
    {"\x2c", 1},    {"\x2d", 1},    {"\x2e", 1},    {"\x2f", 1},
    {"\x30", 1},    {"\x31", 1},    {"\x32", 1},    {"\x33", 1},
    {"\x34", 1},    {"\x35", 1},    {"\x36", 1},    {"\x37", 1},
    {"\x38", 1},    {"\x39", 1},    {"\x3a", 1},    {"\x3b", 1},
    {"\x3c", 1},    {"\x3d", 1},    {"\x3e", 1},    {"\x3f", 1},
    {"\x40", 1},    {"\x41", 1},    {"\x42", 1},    {"\x43", 1},
    {"\x44", 1},    {"\x45", 1},    {"\x46", 1},    {"\x47", 1},
    {"\x48", 1},    {"\x49", 1},    {"\x4a", 1},    {"\x4b", 1},
    {"\x4c", 1},    {"\x4d", 1},    {"\x4e", 1},    {"\x4f", 1},
    {"\x50", 1},    {"\x51", 1},    {"\x52", 1},    {"\x53", 1},
    {"\x54", 1},    {"\x55", 1},    {"\x56", 1},    {"\x57", 1},
    {"\x58", 1},    {"\x59", 1},    {"\x5a", 1},    {"\x5b", 1},
    {"\\\\", 2},    {"\x5d", 1},    {"\x5e", 1},    {"\x5f", 1},
    {"\x60", 1},    {"\x61", 1},    {"\x62", 1},    {"\x63", 1},
    {"\x64", 1},    {"\x65", 1},    {"\x66", 1},    {"\x67", 1},
    {"\x68", 1},    {"\x69", 1},    {"\x6a", 1},    {"\x6b", 1},
    {"\x6c", 1},    {"\x6d", 1},    {"\x6e", 1},    {"\x6f", 1},
    {"\x70", 1},    {"\x71", 1},    {"\x72", 1},    {"\x73", 1},
    {"\x74", 1},    {"\x75", 1},    {"\x76", 1},    {"\x77", 1},
    {"\x78", 1},    {"\x79", 1},    {"\x7a", 1},    {"\x7b", 1},
    {"\x7c", 1},    {"\x7d", 1},    {"\x7e", 1},    {"\x7f", 1},
    {"\x80", 1},    {"\x81", 1},    {"\x82", 1},    {"\x83", 1},
    {"\x84", 1},    {"\x85", 1},    {"\x86", 1},    {"\x87", 1},
    {"\x88", 1},    {"\x89", 1},    {"\x8a", 1},    {"\x8b", 1},
    {"\x8c", 1},    {"\x8d", 1},    {"\x8e", 1},    {"\x8f", 1},
    {"\x90", 1},    {"\x91", 1},    {"\x92", 1},    {"\x93", 1},
    {"\x94", 1},    {"\x95", 1},    {"\x96", 1},    {"\x97", 1},
    {"\x98", 1},    {"\x99", 1},    {"\x9a", 1},    {"\x9b", 1},
    {"\x9c", 1},    {"\x9d", 1},    {"\x9e", 1},    {"\x9f", 1},
    {"\xa0", 1},    {"\xa1", 1},    {"\xa2", 1},    {"\xa3", 1},
    {"\xa4", 1},    {"\xa5", 1},    {"\xa6", 1},    {"\xa7", 1},
    {"\xa8", 1},    {"\xa9", 1},    {"\xaa", 1},    {"\xab", 1},
    {"\xac", 1},    {"\xad", 1},    {"\xae", 1},    {"\xaf", 1},
    {"\xb0", 1},    {"\xb1", 1},    {"\xb2", 1},    {"\xb3", 1},
    {"\xb4", 1},    {"\xb5", 1},    {"\xb6", 1},    {"\xb7", 1},
    {"\xb8", 1},    {"\xb9", 1},    {"\xba", 1},    {"\xbb", 1},
    {"\xbc", 1},    {"\xbd", 1},    {"\xbe", 1},    {"\xbf", 1},
    {"\xc0", 1},    {"\xc1", 1},    {"\xc2", 1},    {"\xc3", 1},
    {"\xc4", 1},    {"\xc5", 1},    {"\xc6", 1},    {"\xc7", 1},
    {"\xc8", 1},    {"\xc9", 1},    {"\xca", 1},    {"\xcb", 1},
    {"\xcc", 1},    {"\xcd", 1},    {"\xce", 1},    {"\xcf", 1},
    {"\xd0", 1},    {"\xd1", 1},    {"\xd2", 1},    {"\xd3", 1},
    {"\xd4", 1},    {"\xd5", 1},    {"\xd6", 1},    {"\xd7", 1},
    {"\xd8", 1},    {"\xd9", 1},    {"\xda", 1},    {"\xdb", 1},
    {"\xdc", 1},    {"\xdd", 1},    {"\xde", 1},    {"\xdf", 1},
    {"\xe0", 1},    {"\xe1", 1},    {"\xe2", 1},    {"\xe3", 1},
    {"\xe4", 1},    {"\xe5", 1},    {"\xe6", 1},    {"\xe7", 1},
    {"\xe8", 1},    {"\xe9", 1},    {"\xea", 1},    {"\xeb", 1},
    {"\xec", 1},    {"\xed", 1},    {"\xee", 1},    {"\xef", 1},
    {"\xf0", 1},    {"\xf1", 1},    {"\xf2", 1},    {"\xf3", 1},
    {"\xf4", 1},    {"\xf5", 1},    {"\xf6", 1},    {"\xf7", 1},
    {"\xf8", 1},    {"\xf9", 1},    {"\xfa", 1},    {"\xfb", 1},
    {"\xfc", 1},    {"\xfd", 1},    {"\xfe", 1},    {"\xff", 1}};

// str ....
static inline const char *ekonEscapeStr(const char *str,
                                        struct EkonAllocator *a) {
  uint32_t len = 0;
  const char *src = str;
  while (EKON_LIKELY(*str != 0)) {
    len += ekonEscapeChars[(unsigned char)(*str)].len;
    ++str;
  }
  char *s = ekonAllocatorAlloc(a, len + 1);
  if (EKON_UNLIKELY(s == 0))
    return 0;
  uint32_t index = 0;
  str = src;
  while (EKON_LIKELY(*str != 0)) {
    ekonAppendLen(s, &index, ekonEscapeChars[(unsigned char)(*str)].str,
                  ekonEscapeChars[(unsigned char)(*str)].len);
    ++str;
  }
  ekonAppendEnd(s, &index);
  return s;
}

// str escape len with str
static inline const char *
ekonEscapeStrLen(const char *str, struct EkonAllocator *a, uint32_t len) {
  uint32_t l = 0;
  const char *src = str;
  uint32_t srcLen = len;
  while (EKON_LIKELY(len != 0)) {
    l += ekonEscapeChars[(unsigned char)(*str)].len;
    ++str;
    --len;
  }
  char *s = ekonAllocatorAlloc(a, l + 1);
  if (EKON_UNLIKELY(s == 0))
    return 0;
  uint32_t index = 0;
  str = src;
  len = srcLen;
  while (EKON_LIKELY(len != 0)) {
    ekonAppendLen(s, &index, ekonEscapeChars[(unsigned char)(*str)].str,
                  ekonEscapeChars[(unsigned char)(*str)].len);
    ++str;
    --len;
  }
  ekonAppendEnd(s, &index);
  return s;
}

// consume a string
static inline bool ekonConsumeStr(const char *s, uint32_t *index,
                                  const char quoteType, uint32_t *line,
                                  uint32_t *pos, bool *isStrSpaced,
                                  bool *isStrMultilined) {
  char c;
  c = s[(*index)++];
  while (EKON_LIKELY(c != 0)) {
    if (c == '\n') {
      if (quoteType != '`')
        return false;
      else {
        *isStrMultilined = true;
        ekonUpdateLine(line, pos, c);
        return ekonConsumeStr(s, index, quoteType, line, pos, isStrSpaced,
                              isStrMultilined);
      }
    }
    if (EKON_UNLIKELY((unsigned char)c <= 0x1f))
      return false;

    if (EKON_UNLIKELY(ekonSkin(c)))
      *isStrSpaced = true;

    if (EKON_UNLIKELY(c == '\\')) {
      c = s[(*index)++];
      switch (c) {
      case '\\':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
      case '/': {
        c = s[(*index)++];
        continue;
      }
      case 'u': {
        uint32_t cp = 0;
        if (EKON_LIKELY(ekonConsumeHex(s, index, &cp))) {
          // ... something to do with handling UTF16 characters
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0xDFFF))
            return false;
          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, index) &&
                            ekonLikelyConsume('u', s, index))) {
              uint32_t cp2 = 0;
              if (EKON_LIKELY(ekonConsumeHex(s, index, &cp2))) {
                if (EKON_UNLIKELY(cp2 < 0xDC00 || cp2 > 0xDFFF))
                  return false;
              }
              return false;
            } else
              return false;
          }
          c = s[(*index)++];
        } else
          return false;
        continue;
      }
      default: {
        if (c == '`' || c == '"' || c == '\'') {
          if (c == quoteType) {
            c = s[(*index)++];
            continue;
          }
        }
        return false;
      }
      }
    }

    if (EKON_UNLIKELY(c == quoteType))
      return true;
    c = s[(*index)++];
  }
  return false;
}

// consume a string in the form of a word
static inline bool ekonConsumeUnquotedStr(const char *s, uint32_t *index) {
  char c;
  c = s[(*index)++];
  while (EKON_LIKELY(c != 0)) {
    if (EKON_UNLIKELY(ekonSkin(c)) || c == '\'' || c == '"' || c == ',' ||
        c == '{' || c == '}' || c == '[' || c == ']') {
      return true;
    }

    if (c == ':') {
      (*index)--;
      return true;
    }

    if (EKON_UNLIKELY((unsigned char)c <= 0x1f))
      return false;
    c = s[(*index)++];
  }
  return false;
}

// Check String, Set Str
static inline bool ekonCheckStr(const char *s, uint32_t *len) {
  uint32_t index = 0;
  char c;
  c = s[index++];
  while (EKON_LIKELY(c != 0)) {
    if (EKON_UNLIKELY(EKON_UNLIKELY((unsigned char)c <= 0x1f) ||
                      EKON_UNLIKELY(c == '"')))
      return false;
    if (EKON_UNLIKELY(c == '\\')) {
      c = s[index++];
      switch (c) {
      case '"':
      case '\\':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
      case '/':
        c = s[index++];
        continue;
      case 'u': {
        uint32_t cp = 0;
        if (EKON_LIKELY(ekonConsumeHex(s, &index, &cp))) {
          // UTF16 and UNICODE support ....
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0xDFFFF))
            return false;
          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, &index) &&
                            ekonLikelyConsume('u', s, &index))) {
              uint32_t cp2 = 0;
              if (EKON_LIKELY(ekonConsumeHex(s, &index, &cp2))) {
                if (EKON_UNLIKELY(cp2 < 0xDC00 || cp2 > 0xDFFF))
                  return false;
              } else
                return false;
            } else
              return false;
          }
          c = s[index++];
        } else
          return false;
        continue;
      }
      default:
        return false;
      }
    }
    c = s[index++];
  }
  *len = index - 1;
  return true;
}

// Check string with length
static inline bool ekonCheckStrLen(struct EkonAllocator *alloc, const char *s,
                                   uint32_t len) {
  if (EKON_UNLIKELY(ekonStringCache == 0)) {
    ekonStringCache = ekonStringNew(alloc, ekonStringCacheInitMemSize);
    if (EKON_UNLIKELY(ekonStringCache == 0))
      return false;
  } else {
    ekonStringReset(ekonStringCache);
  }

  if (EKON_UNLIKELY(ekonStringAppendStr(ekonStringCache, s, len) == false))
    return false;
  ekonStringAppendEnd(ekonStringCache);
  uint32_t avail_len;
  if (EKON_UNLIKELY(ekonCheckStr(ekonStringStr(ekonStringCache), &avail_len) ==
                    false))
    return false;
  if (EKON_UNLIKELY(avail_len != len))
    return false;
  return true;
}

// consume a number
static inline bool ekonConsumeNum(const char *s, uint32_t *index,
                                  uint32_t *line, uint32_t *pos) {
  --(*index);

  // handle unary minus
  if (s[*index] == '-' || s[*index] == '+')
    ++(*index);

  bool decimalStarted = false;

  if (s[*index] == '.') {
    decimalStarted = true;
    (*index)++;
  }

  if (ekonUnlikelyConsume('0', s, index)) {
    if (ekonLikelyConsume('x', s, index) || ekonLikelyConsume('X', s, index)) {
      uint8_t i = 0;
      char c = s[*index];
      while ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ||
             (c >= '0' && c <= '9')) {
        if (i > 4)
          return false;
        c = s[++(*index)];
        i++;
      }
      return true;
    }
  } else if (EKON_LIKELY(EKON_LIKELY(s[*index] >= '1') &&
                         EKON_LIKELY(s[*index] <= '9'))) {
    char c = s[++(*index)];
    while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9')))
      c = s[++(*index)];
  } else
    return false;

  if (ekonConsume('.', s, index)) {
    if (decimalStarted == true)
      return false;

    char c = s[*index];
    if ((EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9'))) {
      c = s[++(*index)];
      while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9')))
        c = s[++(*index)];
    } else if (c == ',' || c == '[' || c == ']' || c == '{' || c == '}' ||
               c == '/' || c == '"' || c == '\'' || ekonSkin(c)) {
      ekonUpdateLine(line, pos, c);
    } else
      return false;
  }

  if (s[*index] == 'e' || s[*index] == 'E') {
    char c = s[++(*index)];
    if (c == '-' || c == '+')
      ++(*index);
    c = s[*index];

    if (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9'))) {
      c = s[++(*index)];
      while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9')))
        c = s[++(*index)];
    } else
      return false;
  }

  return true;
}

// check a number
static inline bool ekonCheckNum(const char *s, uint32_t *len) {
  uint32_t index = 0;

  if (s[index] == '-')
    ++(index);

  if (ekonUnlikelyConsume('0', s, &index)) {
  } else if (EKON_LIKELY(EKON_LIKELY(s[index] >= '1') &&
                         EKON_LIKELY(s[index] <= '9'))) {
    char c = s[++index];
    while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9')))
      c = s[++index];
  } else
    return false;

  if (ekonConsume('.', s, &index)) {
    char c = s[index];
    if ((EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9'))) {
      c = s[++index];
      while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9')))
        c = s[++index];
    } else
      return false;
  }

  if (s[index] == 'e' || s[index] == 'E') {
    char c = s[++index];

    if (c == '-')
      ++index;
    else if (c == '+')
      ++index;

    c = s[index];
    if (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9'))) {
      c = s[++index];
      while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9')))
        c = s[++index];
    } else
      return false;
  }
  *len = index;
  return ekonLikelyConsume(0, s, &index);
}

static inline bool ekonCheckNumLen(struct EkonAllocator *alloc, const char *s,
                                   uint32_t len) {
  if (EKON_UNLIKELY(ekonStringCache == 0)) {
    ekonStringCache = ekonStringNew(alloc, ekonStringCacheInitMemSize);
    if (EKON_UNLIKELY(ekonStringCache == 0))
      return false;
  } else {
    ekonStringReset(ekonStringCache);
  }

  if (EKON_UNLIKELY(ekonStringAppendStr(ekonStringCache, s, len)) == false)
    return false;
  if (EKON_UNLIKELY(ekonStringAppendEnd(ekonStringCache) == false))
    return false;

  uint32_t avail_len;
  if (EKON_UNLIKELY(ekonCheckNum(ekonStringStr(ekonStringCache), &avail_len) ==
                    false))
    return false;

  if (EKON_UNLIKELY(avail_len != len))
    return false;
  return true;
}

// Value Parse Fast - API
static inline bool ekonValueParseFast(struct EkonValue *v, const char *s,
                                      char **err) {
  if (EKON_UNLIKELY(s[0] == '\0')) {
    ekonError(err, 1, 0, 0);
    return false;
  }

  uint32_t line = 1;
  uint32_t pos = 1;
  struct EkonNode *srcNode;

  // v->node is empty and v->alloc has pointer to EkonNode
  if (EKON_LIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0)) {
      ekonError(err, line, pos, 0);
      return false;
    }
    v->n->prev = 0;
    v->n->next = 0;
    v->n->father = 0;
    v->n->key = 0;
    srcNode = 0;
  } else {
    // v->node is not empty and allocator is used to allocate srcNode
    srcNode =
        (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(srcNode == 0)) {
      ekonError(err, line, pos, 0);
      return false;
    }
    *srcNode = *v->n;
  }

  uint32_t index = 0;
  struct EkonNode *node = v->n;
  bool isRootNoCurlyBrace = false;

  char c = ekonPeek(s, &index, &line, &pos);
  const uint32_t ifRootStart = index - 1;

  switch (c) {
  case '[': {
    node->type = EKON_TYPE_ARRAY;
    if (ekonUnlikelyPeekAndConsume(']', s, &index, &line, &pos)) {
      node->value.node = 0;
      node->len = 0;
      break;
    }

    struct EkonNode *n =
        (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(n == 0)) {
      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);
      return false;
    }
    n->father = node;
    n->prev = 0;

    node->value.node = n;
    node->end = n;
    node->len = 1;
    node = n;
    break;
  }
  case '{': {
    node->type = EKON_TYPE_OBJECT;
    if (ekonUnlikelyPeekAndConsume('}', s, &index, &line, &pos)) {
      node->value.node = 0;
      node->len = 0;
      break;
    }
    struct EkonNode *n =
        (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(n == 0)) {
      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);
      return false;
    }
    n->father = node;
    n->prev = 0;
    n->next = 0;

    node->value.node = n;
    node->end = n;
    node->len = 1;
    node = n;
    break;
  }
  case 'n': {
    uint32_t start = index - 1;
    if (EKON_LIKELY(ekonConsumeNull(s, &index))) {
      const char nextChar = s[index];
      if (ekonSkin(nextChar) || nextChar == ']' || nextChar == '}' ||
          nextChar == ',' || nextChar == '"' || nextChar == '\'') {
        ekonUpdateLine(&line, &pos, nextChar);
        node->type = EKON_TYPE_NULL;
        node->value.str = ekonStrNull;
        node->len = 4;
        break;
      }
      if (ekonConsumeUnquotedStr(s, &index)) {
        if (ekonUnlikelyPeekAndConsume(':', s, &index, &line, &pos)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        node->isStrMultilined = false;
        node->isStrSpaced = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        index--;
        break;
      }
    }
    if (ekonConsumeUnquotedStr(s, &index)) {
      if (ekonUnlikelyPeekAndConsume(':', s, &index, &line, &pos)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      node->isStrMultilined = false;
      node->isStrSpaced = false;
      node->type = EKON_TYPE_STRING;
      node->value.str = s + start;
      node->len = index - start - 1;
      index--;
      break;
    }
    if (EKON_LIKELY(srcNode == 0))
      v->n = srcNode;
    else
      *v->n = *srcNode;
    ekonError(err, line, pos, c);
    return false;
  }
  case 'f': {
    uint32_t start = index - 1;
    if (EKON_LIKELY(ekonConsumeFalse(s, &index))) {
      const char nextChar = s[index];
      if (ekonSkin(nextChar) || nextChar == ']' || nextChar == '}' ||
          nextChar == ',' || nextChar == '"' || nextChar == '\'') {
        node->type = EKON_TYPE_BOOL;
        node->value.str = ekonStrFalse;
        node->len = 5;
        break;
      }

      if (ekonConsumeUnquotedStr(s, &index)) {
        if (ekonUnlikelyPeekAndConsume(':', s, &index, &line, &pos)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        node->isStrMultilined = false;
        node->isStrSpaced = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        index--;
        break;
      }
    }
    if (ekonConsumeUnquotedStr(s, &index)) {
      if (ekonUnlikelyPeekAndConsume(':', s, &index, &line, &pos)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      node->isStrMultilined = false;
      node->isStrSpaced = false;
      node->type = EKON_TYPE_STRING;
      node->value.str = s + start;
      node->len = index - start - 1;
      index--;
      break;
    }
    if (EKON_LIKELY(srcNode == 0))
      v->n = srcNode;
    else
      *v->n = *srcNode;
    ekonError(err, line, pos, c);
    return false;
  }
  case 't': {
    uint32_t start = index - 1;
    if (EKON_LIKELY(ekonConsumeTrue(s, &index))) {
      const char nextChar = s[index];
      if (ekonSkin(nextChar) || nextChar == ']' || nextChar == '}' ||
          nextChar == ',' || nextChar == '"' || nextChar == '\'') {
        node->type = EKON_TYPE_BOOL;
        node->value.str = ekonStrTrue;
        node->len = 4;
        break;
      }

      if (ekonConsumeUnquotedStr(s, &index)) {
        if (ekonUnlikelyPeekAndConsume(':', s, &index, &line, &pos)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        node->isStrMultilined = false;
        node->isStrSpaced = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        index--;
        break;
      }
    }
    if (ekonConsumeUnquotedStr(s, &index)) {
      if (ekonUnlikelyPeekAndConsume(':', s, &index, &line, &pos)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      node->isStrMultilined = false;
      node->isStrSpaced = false;
      node->type = EKON_TYPE_STRING;
      node->value.str = s + start;
      node->len = index - start - 1;
      index--;
      break;
    }
    if (EKON_LIKELY(srcNode == 0))
      v->n = srcNode;
    else
      *v->n = *srcNode;
    ekonError(err, line, pos, c);
    return false;
  }
  case '\'':
  case '`':
  case '"': {
    uint32_t start = index;
    bool isStrSpaced = false;
    bool isStrMultilined = false;
    if (EKON_UNLIKELY(ekonUnlikelyConsume(c, s, &index))) {
      node->isStrSpaced = true;
      node->isStrMultilined = false;
      node->type = EKON_TYPE_STRING;
      node->value.str = s + index;
      node->len = 0;
      break;
    }
    if (EKON_LIKELY(ekonConsumeStr(s, &index, c, &line, &pos, &isStrSpaced,
                                   &isStrMultilined))) {
      if (ekonUnlikelyPeekAndConsume(':', s, &index, &line, &pos)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }

      node->isStrSpaced = isStrSpaced;
      node->type = EKON_TYPE_STRING;
      node->value.str = s + start;
      node->len = index - start - 1;
      break;
    }

    if (EKON_LIKELY(srcNode == 0))
      v->n = srcNode;
    else
      *v->n = *srcNode;
    ekonError(err, line, pos, c);
    return false;
  }
  default: {
    uint32_t start = index - 1;
    if (s[start] == '-' || s[start] == '.' || s[start] == '+') {
      if (s[index] >= '0' && s[index] <= '9') {
        if (EKON_LIKELY(ekonConsumeNum(s, &index, &line, &pos))) {
          node->type = EKON_TYPE_NUMBER;
          node->value.str = s + start;
          node->len = index - start;
          break;
        }
      } else {
        index--;
        if (ekonConsumeUnquotedStr(s, &index)) {
          if (s[index] == ':') {
            isRootNoCurlyBrace = true;
            index = ifRootStart;
            break;
          }
          node->isStrSpaced = false;
          node->type = EKON_TYPE_STRING;
          node->value.str = s + start;
          node->len = index - start - 1;
          index--;
          break;
        }
      }
    }

    if (s[start] >= '0' && s[start] <= '9') {
      if (EKON_LIKELY(ekonConsumeNum(s, &index, &line, &pos))) {
        node->type = EKON_TYPE_NUMBER;
        node->value.str = s + start;
        node->len = index - start;
        break;
      }
    } else {
      index--;
      if (ekonConsumeUnquotedStr(s, &index)) {
        if (s[index] == ':') {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        node->isStrSpaced = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        index--;
        break;
      }
    }

    if (EKON_LIKELY(srcNode == 0))
      v->n = srcNode;
    else
      *v->n = *srcNode;
    ekonError(err, line, pos, c);
    return false;
  }
  }

  if (isRootNoCurlyBrace == true) {
    // take this as an object
    node->type = EKON_TYPE_OBJECT;
    struct EkonNode *n =
        (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(n == 0)) {
      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);

      return false;
    }
    n->father = node;
    n->prev = 0;
    n->next = 0;

    node->value.node = n;
    node->end = n;
    node->len = 1;
    node = n;
  }

  while (EKON_LIKELY(node != v->n)) {
    if (node->father->type == EKON_TYPE_OBJECT) {
      bool isKeyUnquoted = false;
      if (EKON_UNLIKELY(ekonLikelyPeekAndConsume('"', s, &index, &line, &pos) ==
                        false) &&
          EKON_UNLIKELY(ekonLikelyPeekAndConsume('\'', s, &index, &line,
                                                 &pos) == false)) {
        uint32_t start = index;
        if (ekonConsumeUnquotedStr(s, &index)) {
          isKeyUnquoted = true;
          node->isKeySpaced = false;
          node->key = s + start;
          node->keyLen = index - start;
        } else {
          if (EKON_LIKELY(srcNode == 0))
            v->n = srcNode;
          else
            *v->n = *srcNode;
          ekonError(err, line, pos, c);
          return false;
        }
      }

      if (isKeyUnquoted == false) {
        char quoteType = s[index - 1]; // either `"` or `'` or "`"
        uint32_t start = index;
        if (EKON_UNLIKELY(ekonUnlikelyConsume(quoteType, s, &index))) {
          node->key = s + start;
          node->keyLen = 0;
        } else {
          bool isKeySpaced = false;
          bool isStrMultilined = false;
          if (EKON_UNLIKELY(ekonConsumeStr(s, &index, quoteType, &line, &pos,
                                           &isKeySpaced,
                                           &isStrMultilined) == false)) {
            if (EKON_LIKELY(srcNode == 0))
              v->n = srcNode;
            else
              *v->n = *srcNode;
            ekonError(err, line, pos, c);
            return false;
          }
          node->isKeySpaced = isKeySpaced;
          node->key = s + start;
          node->keyLen = index - start - 1;
        }
      }

      if (EKON_UNLIKELY(ekonLikelyPeekAndConsume(':', s, &index, &line, &pos) ==
                        false)) {
        if (EKON_LIKELY(srcNode == 0))
          v->n = srcNode;
        else
          *v->n = *srcNode;
        ekonError(err, line, pos, c);
        return false;
      }
    } else {
      node->key = 0;
    }

    c = ekonPeek(s, &index, &line, &pos);
    switch (c) {
    case '[': {
      node->type = EKON_TYPE_ARRAY;
      if (ekonUnlikelyPeekAndConsume(']', s, &index, &line, &pos)) {
        node->value.node = 0;
        node->len = 0;
        break;
      }
      struct EkonNode *n =
          (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
      if (EKON_UNLIKELY(n == 0)) {
        if (EKON_LIKELY(srcNode == 0))
          v->n = srcNode;
        else
          *v->n = *srcNode;
        ekonError(err, line, pos, c);
        return false;
      }

      n->father = node;
      n->prev = 0;

      node->value.node = n;
      node->end = n;
      node->len = 1;
      node = n;
      continue;
    }
    case '{': {
      node->type = EKON_TYPE_OBJECT;
      if (ekonUnlikelyPeekAndConsume('}', s, &index, &line, &pos)) {
        node->value.node = 0;
        node->len = 0;
        break;
      }
      struct EkonNode *n =
          (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
      if (EKON_UNLIKELY(n == 0)) {
        if (EKON_LIKELY(srcNode == 0))
          v->n = srcNode;
        else
          *v->n = *srcNode;
        ekonError(err, line, pos, c);
        return false;
      }
      n->father = node;
      n->prev = 0;
      n->next = 0;

      node->value.node = n;
      node->end = n;
      node->len = 1;
      node = n;

      continue;
    }
    case 'n': {
      uint32_t start = index - 1;
      if (EKON_LIKELY(ekonConsumeNull(s, &index))) {
        const char nextChar = s[index];
        if (ekonSkin(nextChar) || nextChar == ']' || nextChar == '}' ||
            nextChar == ',' || nextChar == '"' || nextChar == '\'' ||
            nextChar == 0) {
          ekonUpdateLine(&line, &pos, nextChar);
          node->type = EKON_TYPE_NULL;
          node->value.str = ekonStrNull;
          node->len = 4;
          break;
        }

        if (ekonConsumeUnquotedStr(s, &index)) {
          node->type = EKON_TYPE_STRING;
          node->isStrMultilined = false;
          node->isStrSpaced = false;
          node->value.str = s + start;
          node->len = index - start - 1;
          index--;
          break;
        }
      }
      if (ekonConsumeUnquotedStr(s, &index)) {
        node->isStrSpaced = false;
        node->isStrMultilined = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        index--;
        break;
      }
      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);
      return false;
    }
    case 'f': {
      uint32_t start = index - 1;
      if (EKON_LIKELY(ekonConsumeFalse(s, &index))) {
        const char nextChar = s[index];
        if (ekonSkin(nextChar) || nextChar == ']' || nextChar == '}' ||
            nextChar == ',' || nextChar == '"' || nextChar == '\'' ||
            nextChar == 0) {
          ekonUpdateLine(&line, &pos, nextChar);
          node->type = EKON_TYPE_BOOL;
          node->value.str = ekonStrFalse;
          node->len = 5;
          break;
        }

        index--;
        if (ekonConsumeUnquotedStr(s, &index)) {
          node->isStrSpaced = false;
          node->isStrMultilined = false;
          node->type = EKON_TYPE_STRING;
          node->value.str = s + start;
          node->len = index - start - 1;
          index--;
          break;
        }
      }
      if (ekonConsumeUnquotedStr(s, &index)) {
        node->isStrSpaced = false;
        node->isStrMultilined = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        index--;
        break;
      }

      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);
      return false;
    }
    case 't': {
      uint32_t start = index - 1;
      if (EKON_LIKELY(ekonConsumeTrue(s, &index))) {
        const char nextChar = s[index];
        if (ekonSkin(nextChar) || nextChar == ']' || nextChar == '}' ||
            nextChar == ',' || nextChar == '"' || nextChar == 0 ||
            nextChar == '\'') {
          ekonUpdateLine(&line, &pos, c);
          node->type = EKON_TYPE_BOOL;
          node->value.str = ekonStrTrue;
          node->len = 4;
          break;
        }

        index--;
        if (ekonConsumeUnquotedStr(s, &index)) {
          node->isStrSpaced = false;
          node->isStrMultilined = false;
          node->type = EKON_TYPE_STRING;
          node->value.str = s + start;
          node->len = index - start - 1;
          index--;
          break;
        }
      }

      if (ekonConsumeUnquotedStr(s, &index)) {
        node->isStrSpaced = false;
        node->isStrMultilined = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        index--;
        break;
      }

      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);
      return false;
    }
    case '`':
    case '\'':
    case '"': {
      uint32_t start = index;
      bool isStrSpaced = false;
      bool isStrMultilined = false;

      if (EKON_UNLIKELY(ekonUnlikelyConsume(c, s, &index))) {
        node->isStrSpaced = true;
        node->isStrMultilined = false;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + index;
        node->len = 0;
        break;
      }
      if (EKON_LIKELY(ekonConsumeStr(s, &index, c, &line, &pos, &isStrSpaced,
                                     &isStrMultilined))) {
        node->isStrSpaced = isStrSpaced;
        node->isStrMultilined = isStrMultilined;
        node->type = EKON_TYPE_STRING;
        node->value.str = s + start;
        node->len = index - start - 1;
        break;
      }
      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);
      return false;
    }
    default: {
      uint32_t start = index - 1;
      if (s[start] == '-' || s[start] == '.' || s[start] == '+') {
        if ((s[index] >= '0' && s[index] <= '9') || s[index] == '.') {
          if (EKON_LIKELY(ekonConsumeNum(s, &index, &line, &pos))) {
            node->type = EKON_TYPE_NUMBER;
            node->value.str = s + start;
            node->len = index - start;
            break;
          }
        } else {
          if (ekonConsumeUnquotedStr(s, &index)) {
            node->isStrSpaced = false;
            node->isStrMultilined = false;
            node->type = EKON_TYPE_STRING;
            node->value.str = s + start;
            node->len = index - start - 1;
            index--;
            break;
          }
        }
      }

      if (s[start] >= '0' && s[start] <= '9') {
        if (EKON_LIKELY(ekonConsumeNum(s, &index, &line, &pos))) {
          node->type = EKON_TYPE_NUMBER;
          node->value.str = s + start;
          node->len = index - start;
          break;
        }
      } else {
        index--;
        if (ekonConsumeUnquotedStr(s, &index)) {
          node->isStrSpaced = false;
          node->isStrMultilined = false;
          node->type = EKON_TYPE_STRING;
          node->value.str = s + start;
          node->len = index - start - 1;
          break;
        }
      }
      if (EKON_LIKELY(srcNode == 0))
        v->n = srcNode;
      else
        *v->n = *srcNode;
      ekonError(err, line, pos, c);
      return false;
    }
    }

    while (EKON_LIKELY(node != v->n)) {
      char c = ekonPeek(s, &index, &line, &pos);
      if (c == ',')
        if (c == ',')
          c = ekonPeek(s, &index, &line, &pos);

      if ((EKON_LIKELY((EKON_LIKELY(c == '}') &&
                        EKON_LIKELY(node->father->type == EKON_TYPE_OBJECT))) ||
           EKON_LIKELY(EKON_LIKELY(c == ']') &&
                       EKON_LIKELY(node->father->type == EKON_TYPE_ARRAY))) ||
          (c == 0 && isRootNoCurlyBrace)) {
        node->next = 0;
        node = node->father;
      } else {
        struct EkonNode *n = (struct EkonNode *)ekonAllocatorAlloc(
            v->a, sizeof(struct EkonNode));
        if (EKON_UNLIKELY(n == 0)) {
          if (EKON_LIKELY(srcNode == 0))
            v->n = srcNode;
          else
            *v->n = *srcNode;
          ekonError(err, line, pos, c);
          return false;
        }
        n->father = node->father;
        n->prev = node;

        node->father->end = n;
        ++(node->father->len);
        node->next = n;
        node = n;
        index--;
        break;
      }
    }
  }

  if (EKON_LIKELY(ekonLikelyPeekAndConsume(0, s, &index, &line, &pos)))
    return true;

  if (EKON_LIKELY(srcNode == 0))
    v->n = srcNode;
  else
    *v->n = *srcNode;
  ekonError(err, line, pos, c);
  return false;
}

// ekon parse - API
static inline bool ekonValueParseLen(struct EkonValue *v, const char *s,
                                     uint32_t len, char **err) {
  char *str = ekonAllocatorAlloc(v->a, len + 1);
  if (EKON_UNLIKELY(str == 0))
    return false;
  ekonCopy(s, len, str);
  str[len] = 0;
  return ekonValueParseFast(v, s, err);
}

// The main parser - API
static inline bool ekonValueParse(struct EkonValue *v, const char *s,
                                  char **err) {
  return ekonValueParseLen(v, s, ekonStrLen(s), err);
}

// stringify a value
static inline const char *ekonValueStringifyToJSON(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return "";

  struct EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  struct EkonNode *node = v->n;

  switch (node->type) {
  case EKON_TYPE_ARRAY: {
    if (EKON_UNLIKELY(ekonStringAppendChar(str, '[') == false))
      return 0;
    if (node->value.node != 0)
      node = node->value.node;
    else {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
        return 0;
    }
    break;
  }
  case EKON_TYPE_OBJECT: {
    if (EKON_UNLIKELY(ekonStringAppendChar(str, '{') == false))
      return 0;

    if (node->len != 0)
      node = node->value.node;
    else {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '}') == false))
        return 0;
    }
    break;
  }
  case EKON_TYPE_STRING: {
    if (EKON_UNLIKELY(ekonStringAppendChar(str, '"') == false))
      return 0;
    if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                      false))
      return 0;
    if (EKON_UNLIKELY(ekonStringAppendChar(str, '"') == false))
      return 0;
    break;
  }
  default: {
    if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                      false))
      return 0;
    break;
  }
  }

  while (EKON_LIKELY(node != v->n)) {
    if (node->key != 0) {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '"') == false))
        return 0;
      if (EKON_UNLIKELY(ekonStringAppendStr(str, node->key, node->keyLen) ==
                        false))
        return 0;
      if (EKON_UNLIKELY(ekonStringAppendStr(str, "\":", 2) == false))
        return 0;
    }
    switch (node->type) {
    case EKON_TYPE_ARRAY: {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '[') == false))
        return 0;
      if (node->value.node != 0) {
        node = node->value.node;
        continue;
      } else {
        if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
          return 0;
      }
      break;
    }
    case EKON_TYPE_OBJECT: {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '{') == false))
        return 0;
      if (node->len != 0) {
        node = node->value.node;
        continue;
      } else {
        if (EKON_UNLIKELY(ekonStringAppendChar(str, '}') == false))
          return 0;
      }
      break;
    }
    case EKON_TYPE_STRING: {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '"') == false))
        return 0;
      if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                        false))
        return 0;
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '"') == false))
        return 0;
      break;
    }
    default: {
      if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                        false))
        return 0;
      break;
    }
    }

    while (EKON_LIKELY(node != v->n)) {
      if (EKON_LIKELY(node->next != 0)) {
        if (EKON_UNLIKELY(ekonStringAppendChar(str, ',') == false))
          return 0;
        node = node->next;
        break;
      } else {
        node = node->father;
        if (node->type == EKON_TYPE_ARRAY) {
          if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
            return 0;
        } else {
          if (EKON_UNLIKELY(ekonStringAppendChar(str, '}') == false))
            return 0;
        }
      }
    }
  }

  if (EKON_UNLIKELY(ekonStringAppendEnd(str) == false))
    return 0;

  return ekonStringStr(str);
}

static inline const bool ekonAppendQuote(const struct EkonNode *node,
                                         struct EkonString *str) {
  if (node->isStrMultilined == true) {
    if (EKON_UNLIKELY(ekonStringAppendChar(str, '`') == false))
      return false;
  } else if (node->isStrSpaced == true) {
    if (EKON_UNLIKELY(ekonStringAppendChar(str, '`') == false))
      return false;
  }
  return true;
}

static inline const char *ekonValueStringify(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return "";

  struct EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  struct EkonNode *node = v->n;

  switch (node->type) {
  case EKON_TYPE_ARRAY: {
    if (EKON_UNLIKELY(ekonStringAppendChar(str, '[') == false))
      return 0;
    if (node->value.node != 0)
      node = node->value.node;
    else {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
        return 0;
    }
    break;
  }
  case EKON_TYPE_OBJECT: {
    if (node->len != 0)
      node = node->value.node;
    break;
  }
  case EKON_TYPE_STRING: {
    if (ekonAppendQuote(node, str) == false)
      return 0;

    if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                      false))
      return 0;

    if (ekonAppendQuote(node, str) == false)
      return 0;

    break;
  }
  default: {
    if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                      false))
      return 0;
    break;
  }
  }

  while (EKON_LIKELY(node != v->n)) {
    if (node->key != 0) {
      if (node->isKeySpaced == true &&
          EKON_UNLIKELY(ekonStringAppendChar(str, '\'') == false))
        return 0;
      if (EKON_UNLIKELY(ekonStringAppendStr(str, node->key, node->keyLen) ==
                        false))
        return 0;
      if (node->isKeySpaced == true &&
          EKON_UNLIKELY(ekonStringAppendChar(str, '\'')) == false)
        return 0;

      if (EKON_UNLIKELY(ekonStringAppendChar(str, ':') == false))
        return 0;
    }
    switch (node->type) {
    case EKON_TYPE_ARRAY: {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '[') == false))
        return 0;
      if (node->value.node != 0) {
        node = node->value.node;
        continue;
      } else {
        if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
          return 0;
      }
      break;
    }
    case EKON_TYPE_OBJECT: {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '{') == false))
        return 0;
      if (node->len != 0) {
        node = node->value.node;
        continue;
      } else {
        if (EKON_UNLIKELY(ekonStringAppendChar(str, '}') == false))
          return 0;
      }
      break;
    }
    case EKON_TYPE_STRING: {
      if (ekonAppendQuote(node, str) == false)
        return 0;
      if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                        false))
        return 0;
      if (ekonAppendQuote(node, str) == false)
        return 0;
      break;
    }
    default: {
      if (EKON_UNLIKELY(ekonStringAppendStr(str, node->value.str, node->len) ==
                        false))
        return 0;
      break;
    }
    }

    while (EKON_LIKELY(node != v->n)) {
      if (EKON_LIKELY(node->next != 0)) {
        if (EKON_UNLIKELY(ekonStringAppendChar(str, ' ') == false))
          return 0;
        node = node->next;
        break;
      } else {
        node = node->father;
        if (node->type == EKON_TYPE_ARRAY) {
          if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
            return 0;
        } else {
          if (node->father != 0 &&
              EKON_UNLIKELY(ekonStringAppendChar(str, '}') == false))
            return 0;
        }
      }
    }
  }

  if (EKON_UNLIKELY(ekonStringAppendEnd(str) == false))
    return 0;

  return ekonStringStr(str);
}

// get str fast
static inline const char *ekonValueGetStrFast(const struct EkonValue *v,
                                              uint32_t *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_STRING))
    return 0;
  *len = v->n->len;
  return v->n->value.str;
}

// get string from value
static inline const char *ekonValueGetStr(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_STRING))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(v->n->value.str, v->n->len, str);
  str[v->n->len] = 0;
  return str;
}

// get unescaped string
static inline const char *ekonValueGetUnEspaceStr(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_STRING))
    return 0;

  char *retStr = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(retStr == 0))
    return 0;
  ekonUnEscapeStr(v->n->value.str, v->n->len, retStr);
  return retStr;
}

// get num fast
static inline const char *ekonValueGetNumFast(const struct EkonValue *v,
                                              uint32_t *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_NUMBER))
    return 0;
  *len = v->n->len;
  return v->n->value.str;
}

// get num as str
static inline const char *ekonValueGetNumStr(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_NUMBER))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(v->n->value.str, v->n->len, str);
  str[v->n->len] = 0;
  return str;
}

// get num as a double
static inline const double *ekonValueGetNum(struct EkonValue *v) {
  return ekonValueGetDouble(v);
}

// get num as a double
static inline const double *ekonValueGetDouble(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_NUMBER))
    return 0;
  double *d = (double *)ekonAllocatorAlloc(v->a, sizeof(double));
  if (EKON_UNLIKELY(d == 0))
    return 0;
  *d = ekonStrToDouble(v->n->value.str);
  return d;
}

// get num as an int
static inline const int *ekonValueGetInt(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_NUMBER))
    return 0;
  int *i = (int *)ekonAllocatorAlloc(v->a, sizeof(int));
  if (EKON_UNLIKELY(i == 0))
    return 0;
  *i = ekonStrToInt(v->n->value.str);
  return i;
}

// get value as an long
static inline const long *ekonValueGetLong(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_NUMBER))
    return 0;
  long *i = (long *)ekonAllocatorAlloc(v->a, sizeof(long));
  if (EKON_UNLIKELY(i == 0))
    return 0;
  *i = ekonStrToLong(v->n->value.str);
  return i;
}

// get value as an long
static inline const long long *ekonValueGetLongLong(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_NUMBER))
    return 0;
  long long *i = (long long *)ekonAllocatorAlloc(v->a, sizeof(long long));
  if (EKON_UNLIKELY(i == 0))
    return 0;
  *i = ekonStrToLongLong(v->n->value.str);
  return i;
}

// get value as bool
static inline const bool *ekonValueGetBool(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_BOOL))
    return 0;
  bool *val = (bool *)malloc(1);
  *val = false;
  if (*(v->n->value.str) == 't')
    *val = true;
  return val;
}

// check value as null
static inline bool ekonValueIsNull(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_NULL))
    return false;
  return true;
}

// get the key
static inline const char *ekonValueGetKey(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->keyLen + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(v->n->key, v->n->keyLen, str);
  str[v->n->keyLen] = 0;
  return str;
}

// get the key as unescaped
static inline const char *ekonValueGetUnEspaceKey(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->keyLen + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonUnEscapeStr(v->n->key, v->n->keyLen, str);
  return str;
}

// get the key fast
static inline const char *ekonValueGetKeyFast(const struct EkonValue *v,
                                              uint32_t *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  *len = v->n->keyLen;
  return v->n->key;
}

// get the value of the object
static inline struct EkonValue *ekonValueObjGet(const struct EkonValue *v,
                                                const char *key) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_OBJECT))
    return 0;
  struct EkonNode *next = v->n->value.node;
  while (EKON_LIKELY(next != 0)) {
    if (EKON_UNLIKELY(ekonStrIsEqual(key, next->key, next->keyLen) == true)) {
      struct EkonValue *retVal = ekonValueInnerNew(v->a, next);
      return retVal;
    }
    next = next->next;
  }
  return 0;
}

// get the value of the object with length
static inline struct EkonValue *
ekonValueObjGetLen(const struct EkonValue *v, const char *key, uint32_t len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_OBJECT))
    return 0;
  struct EkonNode *next = v->n->value.node;
  while (EKON_LIKELY(next != 0)) {
    if (EKON_UNLIKELY(ekonStrIsEqualLen(key, len, next->key, next->keyLen))) {
      struct EkonValue *retVal = ekonValueInnerNew(v->a, next);
      return retVal;
    }
  }
  return 0;
}

// Get the type of EKON Value
static inline const EkonType *ekonValueType(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  return &v->n->type;
}

// Get the size of the EKON Value
static inline uint32_t ekonValueSize(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_OBJECT &&
                    v->n->type != EKON_TYPE_ARRAY))
    return 0;
  return v->n->len;
}

// Get the value as array
static inline struct EkonValue *ekonValueArrayGet(const struct EkonValue *v,
                                                  uint32_t index) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_ARRAY))
    return 0;
  struct EkonNode *next = v->n->value.node;
  uint32_t i = 0;
  while (EKON_LIKELY(next != 0)) {
    if (EKON_UNLIKELY(i == index)) {
      struct EkonValue *retVal = ekonValueInnerNew(v->a, next);
      return retVal;
    }
    next = next->next;
    ++i;
  }
  return 0;
}

// start the current value
static inline struct EkonValue *ekonValueBegin(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_OBJECT &&
                    v->n->type != EKON_TYPE_ARRAY))
    return 0;

  if (EKON_UNLIKELY(v->n->value.node != 0)) {
    struct EkonValue *retVal = ekonValueInnerNew(v->a, v->n->value.node);
    return retVal;
  }
  return 0;
}

// get the next value
static inline struct EkonValue *ekonValueNext(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_LIKELY(v->n->next != 0)) {
    struct EkonValue *retVal = ekonValueInnerNew(v->a, v->n->next);
    return retVal;
  }
  return 0;
}

// Copy value from one pointer to another
static inline bool ekonValueCopyFrom(struct EkonValue *v,
                                     const struct EkonValue *vv) {
  if (EKON_UNLIKELY(vv->n == 0))
    return false;
  struct EkonAllocator *const a = v->a;
  v->n = (struct EkonNode *)ekonAllocatorAlloc(a, sizeof(struct EkonNode));
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  v->n->prev = 0;
  v->n->next = 0;
  v->n->father = 0;

  struct EkonNode *node = vv->n;
  struct EkonNode *desNode = v->n;

  do {
    desNode->type = node->type;
    if (node->key != 0) {
      char *k = ekonAllocatorAlloc(a, node->keyLen);
      if (EKON_UNLIKELY(k == 0))
        return false;
      ekonCopy(node->key, node->keyLen, k);
      desNode->key = k;
      desNode->keyLen = node->keyLen;
    } else
      desNode->key = 0;

    switch (node->type) {
    case EKON_TYPE_OBJECT:
    case EKON_TYPE_ARRAY: {
      desNode->len = node->len;
      if (EKON_LIKELY(node->value.node != 0)) {
        node = node->value.node;
        struct EkonNode *n =
            (struct EkonNode *)ekonAllocatorAlloc(a, sizeof(struct EkonNode));
        if (EKON_UNLIKELY(n == 0))
          return false;
        n->father = desNode;
        n->prev = 0;
        desNode->value.node = n;
        desNode->end = n;
        desNode = n;
        continue;
      }
      desNode->value.node = 0;
      desNode->end = 0;
      break;
    }
    case EKON_TYPE_BOOL:
    case EKON_TYPE_NULL: {
      desNode->value.str = node->value.str;
      desNode->len = node->len;
      break;
    }
    case EKON_TYPE_NUMBER:
    case EKON_TYPE_STRING: {
      char *s = ekonAllocatorAlloc(a, node->len);
      if (EKON_UNLIKELY(s == 0))
        return false;
      ekonCopy(node->value.str, node->len, s);
      desNode->value.str = s;
      desNode->len = node->len;
    } break;
    }
    while (EKON_LIKELY(node != vv->n)) {
      if (EKON_LIKELY(node->next != 0)) {
        node = node->next;
        struct EkonNode *n =
            (struct EkonNode *)ekonAllocatorAlloc(a, sizeof(struct EkonNode));
        if (EKON_UNLIKELY(n == 0))
          return false;
        n->father = desNode->father;
        n->prev = desNode;
        n->father->end = n;
        desNode->next = n;
        desNode = n;
        break;
      } else {
        node = node->father;
        desNode->next = 0;
        desNode = desNode->father;
      }
    }
  } while (EKON_UNLIKELY(node != vv->n));

  return true;
}

// copy a value and return it
static inline struct EkonValue *ekonValueCopy(const struct EkonValue *v) {
  struct EkonValue *retVal = ekonValueNew(v->a);
  if (EKON_UNLIKELY(ekonValueCopyFrom(retVal, v) == false))
    return 0;
  return retVal;
}

// move a value // TODO: update the comment
static inline bool ekonValueMove(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (v->n->father != 0) {

    if (v->n->prev == 0)
      v->n->father->value.node = v->n->next;
    else {
      v->n->prev->next = v->n->next;
      v->n->prev = 0;
    }

    if (v->n->next == 0)
      v->n->father->end = v->n->prev;
    else {
      v->n->next->prev = v->n->prev;
      v->n->next = 0;
    }

    --(v->n->father->len);
    v->n->father = 0;
  }
  return true;
}

// set a value to null
static inline bool ekonValueSetNull(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NULL;
  v->n->value.str = ekonStrNull;
  v->n->len = 4;
  return true;
}

// set a value to bool
static inline bool ekonValueSetBool(struct EkonValue *v, bool b) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_BOOL;
  if (b == false) {
    v->n->value.str = ekonStrFalse;
    v->n->len = 5;
    return true;
  }
  v->n->value.str = ekonStrTrue;
  v->n->len = 4;
  return true;
}

// set a num str fast
static inline bool ekonValueSetNumStrFast(struct EkonValue *v,
                                          const char *num) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckNum(num, &len) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set a num str with length fast
static inline bool ekonValueSetNumStrLenFast(struct EkonValue *v,
                                             const char *num, uint32_t len) {
  if (EKON_UNLIKELY(ekonCheckNumLen(v->a, num, len) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set a num str
static inline bool ekonValueSetNumStr(struct EkonValue *v, const char *num) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckNum(num, &len) == false))
    return false;
  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(num, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set a num str with length
static inline bool ekonValueSetNumStrLen(struct EkonValue *v, const char *num,
                                         uint32_t len) {
  if (EKON_UNLIKELY(ekonCheckNumLen(v->a, num, len) == false))
    return false;
  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(num, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set num to a value
static inline bool ekonValueSetNum(struct EkonValue *v, const double d) {
  return ekonValueSetDouble(v, d);
}

// set double to a value
static inline bool ekonValueSetDouble(struct EkonValue *v, const double d) {
  char *num = ekonAllocatorAlloc(v->a, 32);
  if (EKON_UNLIKELY(num == 0))
    return false;
  uint32_t len = ekonDoubleToStr(d, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_LIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->next = 0;
    v->n->father = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set int to a value
static inline bool ekonValueSetInt(struct EkonValue *v, const int n) {
  char *num = ekonAllocatorAlloc(v->a, 16);
  if (EKON_UNLIKELY(num == 0))
    return false;
  uint32_t len = ekonIntToStr(n, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set long to a value
static inline bool ekonValueSetLong(struct EkonValue *v, const long n) {
  char *num = ekonAllocatorAlloc(v->a, 24);
  if (EKON_UNLIKELY(num == 0))
    return false;
  uint32_t len = ekonLongToStr(n, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set long long to a value
static inline bool ekonValueSetLongLong(struct EkonValue *v,
                                        const long long n) {
  char *num = ekonAllocatorAlloc(v->a, 24);
  if (EKON_UNLIKELY(num == 0))
    return false;
  uint32_t len = ekonLongLongToStr(n, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set str escaped character
static inline bool ekonValueSetStrEscape(struct EkonValue *v, const char *str) {
  const char *es = ekonEscapeStr(str, v->a);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetStrFast(v, es);
}

// set str escaped characters with len
static inline bool ekonValueSetStrLenEscape(struct EkonValue *v,
                                            const char *str, uint32_t len) {
  const char *es = ekonEscapeStrLen(str, v->a, len);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetStrFast(v, es);
}

// set str fast
static inline bool ekonValueSetStrFast(struct EkonValue *v, const char *str) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckStr(str, &len) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_STRING;
  v->n->value.str = str;
  v->n->len = len;
  return true;
}

// set str with length fast
static inline bool ekonValueSetStrLenFast(struct EkonValue *v, const char *str,
                                          uint32_t len) {
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, str, len) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_STRING;
  v->n->value.str = str;
  v->n->len = len;
  return true;
}

// set value set str
static inline bool ekonValueSetStr(struct EkonValue *v, const char *str) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckStr(str, &len) == false))
    return false;
  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(str, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
  }
  v->n->type = EKON_TYPE_STRING;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set value to str with length
static inline bool ekonValueSetStrLen(struct EkonValue *v, const char *str,
                                      uint32_t len) {
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, str, len) == false))
    return false;

  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(str, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_STRING;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set key with escape chars
static inline bool ekonValueSetKeyEscape(struct EkonValue *v, const char *key) {
  const char *es = ekonEscapeStr(key, v->a);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetKeyFast(v, es);
}

// set key  with escape and length
static inline bool ekonValueSetKeyLenEscape(struct EkonValue *v,
                                            const char *key, uint32_t len) {
  const char *es = ekonEscapeStrLen(key, v->a, len);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetKeyFast(v, es);
}

// set key fast
static inline bool ekonValueSetKeyFast(struct EkonValue *v, const char *key) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckStr(key, &len) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->type = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->type != EKON_TYPE_OBJECT))
    return false;

  v->n->key = key;
  v->n->keyLen = len;
  return true;
}

// set key len fast
static inline bool ekonValueSetKeyLenFast(struct EkonValue *v, const char *key,
                                          uint32_t len) {
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, key, len) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->type = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->type != EKON_TYPE_OBJECT))
    return false;
  v->n->key = key;
  v->n->keyLen = len;
  return true;
}

// set key
static inline bool ekonValueSetKey(struct EkonValue *v, const char *key) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckStr(key, &len) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->type = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->type != EKON_TYPE_OBJECT))
    return false;

  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(key, len, s);
  v->n->key = s;
  v->n->keyLen = len;
  return true;
}

// set key with a particular length
static inline bool ekonValueSetKeyLen(struct EkonValue *v, const char *key,
                                      uint32_t len) {
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, key, len) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->type = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->type != EKON_TYPE_OBJECT))
    return false;

  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(key, len, s);
  v->n->key = s;
  v->n->keyLen = len;
  return true;
}

// set array value
static inline bool ekonValueSetArray(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_ARRAY;
  v->n->value.node = 0;
  v->n->len = 0;
  return true;
}

// Set Object
static inline bool ekonValueSetObj(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->type = EKON_TYPE_OBJECT;
  v->n->value.node = 0;
  v->n->len = 0;
  return true;
}

// set a value fast
static inline bool ekonValueSetFast(struct EkonValue *v, struct EkonValue *vv) {
  if (EKON_UNLIKELY(ekonValueMove(vv) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = vv->n;
    vv->n = 0;
    return true;
  }
  v->n->type = vv->n->type;
  if (v->n->key != 0 && vv->n->key != 0) {
    v->n->key = vv->n->key;
    v->n->keyLen = vv->n->keyLen;
  }
  v->n->value = vv->n->value;
  v->n->len = vv->n->len;
  if (v->n->type == EKON_TYPE_ARRAY || v->n->type == EKON_TYPE_OBJECT) {
    v->n->end = vv->n->end;
    struct EkonNode *next = v->n->value.node;
    while (EKON_LIKELY(next != 0)) {
      next->father = v->n;
      next = next->next;
    }
  }
  vv->n = 0;
  return true;
}

// 函数说明详见《API》
static inline bool ekonValueSet(struct EkonValue *v,
                                const struct EkonValue *vv) {
  struct EkonValue *cp = ekonValueCopy(vv);
  if (EKON_UNLIKELY(cp == 0))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = cp->n;
    return true;
  }
  v->n->type = cp->n->type;
  if (v->n->key != 0 && vv->n->key != 0) {
    v->n->key = cp->n->key;
    v->n->keyLen = cp->n->keyLen;
  }
  v->n->value = cp->n->value;
  v->n->len = cp->n->len;
  if (v->n->type == EKON_TYPE_ARRAY || v->n->type == EKON_TYPE_OBJECT) {
    v->n->end = vv->n->end;
    struct EkonNode *next = v->n->value.node;
    while (EKON_LIKELY(next != 0)) {
      next->father = v->n;
      next = next->next;
    }
  }
  return true;
}

// 函数说明详见《API》
static inline bool ekonValueObjAddFast(struct EkonValue *v,
                                       struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_OBJECT))
    return false;
  if (EKON_UNLIKELY(vv->n == 0))
    return false;
  if (EKON_UNLIKELY(vv->n->key == 0))
    return false;
  if (EKON_UNLIKELY(ekonValueMove(vv) == false))
    return false;
  vv->n->father = v->n;
  if (EKON_UNLIKELY(v->n->value.node == 0)) {
    v->n->value.node = vv->n;
    v->n->len = 1;
    v->n->end = vv->n;
  } else {
    v->n->end->next = vv->n;
    vv->n->prev = v->n->end;
    v->n->end = vv->n;
    ++v->n->len;
  }
  vv->n = 0;
  return true;
}

// 函数说明详见《API》
static inline bool ekonValueObjAdd(struct EkonValue *v,
                                   const struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_OBJECT))
    return false;
  if (EKON_UNLIKELY(vv->n == 0))
    return false;
  if (EKON_UNLIKELY(vv->n->key == 0))
    return false;
  struct EkonValue *cp = ekonValueCopy(vv);
  if (EKON_UNLIKELY(cp == 0))
    return false;
  cp->n->father = v->n;
  if (EKON_UNLIKELY(v->n->value.node == 0)) {
    v->n->value.node = cp->n;
    v->n->len = 1;
    v->n->end = cp->n;
  } else {
    v->n->end->next = cp->n;
    cp->n->prev = v->n->end;
    v->n->end = cp->n;
    ++v->n->len;
  }
  return true;
}

// 函数说明详见《API》
static inline bool ekonValueArrayAddFast(struct EkonValue *v,
                                         struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_ARRAY))
    return false;
  if (EKON_UNLIKELY(ekonValueMove(vv) == false))
    return false;
  vv->n->key = 0;
  vv->n->father = v->n;
  if (EKON_UNLIKELY(v->n->value.node == 0)) {
    v->n->value.node = vv->n;
    v->n->len = 1;
    v->n->end = vv->n;
  } else {
    v->n->end->next = vv->n;
    vv->n->prev = v->n->end;
    v->n->end = vv->n;
    ++v->n->len;
  }
  vv->n = 0;
  return true;
}

// 函数说明详见《API》
static inline bool ekonValueArrayAdd(struct EkonValue *v,
                                     const struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->type != EKON_TYPE_ARRAY))
    return false;
  struct EkonValue *cp = ekonValueCopy(vv);
  if (EKON_UNLIKELY(cp == 0))
    return false;
  cp->n->key = 0;
  cp->n->father = v->n;
  if (EKON_UNLIKELY(v->n->value.node == 0)) {
    v->n->value.node = cp->n;
    v->n->len = 1;
    v->n->end = cp->n;
  } else {
    v->n->end->next = cp->n;
    cp->n->prev = v->n->end;
    v->n->end = cp->n;
    ++v->n->len;
  }
  return true;
}

// 函数说明详见《API》
static inline bool ekonValueArrayDel(struct EkonValue *v, uint32_t index) {
  struct EkonValue *dv = ekonValueArrayGet(v, index);
  if (EKON_UNLIKELY(dv == 0))
    return false;
  return ekonValueMove(dv);
}

// 函数说明详见《API》
static inline bool ekonValueObjDel(struct EkonValue *v, const char *key) {
  struct EkonValue *dv = ekonValueObjGet(v, key);
  if (EKON_UNLIKELY(dv == 0))
    return false;
  return ekonValueMove(dv);
}

#endif
