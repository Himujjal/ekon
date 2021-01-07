#include "ekon.h"
#include "hashtable.h"
// store constants in char arrays
static const char *ekonStrTrue = "true";
static const char *ekonStrFalse = "false";
static const char *ekonStrNull = "null";

bool ekonIsQuote(const char c) { return c == '\'' || c == '"'; }
bool ekonIsBracket(const char c) {
  return (c == '{' || c == '}' || c == ']' || c == '[');
}
// <space> \t \n \r [ ] { } ' " , : \0
bool ekonIsNonUnquoteStrChar(const char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '[' ||
         c == ']' || c == '{' || c == '}' || c == '"' || c == '\'' ||
         c == '\0' || c == ',' || c == ':';
}

// copy a string from one pointer to another
void ekonCopy(const char *src, uint32_t len, char *des) {
  memcpy(des, src, len);
}

char *ekonCopySchema(const char *src, uint32_t len) {
  char *schema = (char *)malloc(sizeof(char) * (len + 1));
  memcpy(schema, src, len);
  schema[len] = '\0';
  return schema;
}

uint32_t ekonStrLen(const char *str) { return (uint32_t)strlen(str); }

int ekonStrToInt(const char *str) { return atoi(str); }

long ekonStrToLong(const char *str) { return atol(str); }
long long ekonStrToLongLong(const char *str) { return atoll(str); }
double ekonStrToDouble(const char *str) { return atof(str); }
uint32_t ekonIntToStr(int n, char *buff) { return snprintf(buff, 12, "%d", n); }
uint32_t ekonLongToStr(long n, char *buff) {
  return snprintf(buff, 24, "%ld", n);
}
uint32_t ekonLongLongToStr(long long n, char *buff) {
  return snprintf(buff, 24, "%lld", n);
}
uint32_t ekonDoubleToStr(double n, char *buff) {
  return snprintf(buff, 32, "%.17g", n);
}

/**
 * mes - The pointer to the character array for error messages
 * line - line of the file where the error occured
 * pos - position of the file where the error occured
 * c - character where the error occured
 * */
bool ekonError(char **message, struct EkonAllocator *a, const char *s,
               uint32_t index) {
  uint32_t pos = 1;
  uint32_t line = 1;
  uint32_t cursor = 0;
  while (s[cursor] != 0 && cursor != index) {
    if (s[cursor] == '\n') {
      pos = 0;
      line++;
    }
    pos++;
    cursor++;
  }
  pos--;

  *message = ekonAllocatorAlloc(a, sizeof(char) * 50);

  // error messages will be of format: "<line>:<pos>:<character>" with `:` as
  // delimiter
  if (s[index] == 0)
    snprintf(*message, 50, "%d:%d:0", line, pos);
  else
    snprintf(*message, 50, "%d:%d:%c", line, pos, s[index - 1]);

  return false;
}

bool ekonDuplicateKeyError(char **message, struct EkonAllocator *a,
                           const char *s, uint32_t index, uint32_t keyLen) {
  uint32_t pos = 1;
  uint32_t line = 1;
  uint32_t cursor = 0;
  while (s[cursor] != 0 && cursor != index) {
    if (s[cursor] == '\n') {
      pos = 0;
      line++;
    }
    pos++;
    cursor++;
  }

  *message = ekonAllocatorAlloc(a, sizeof(char) * (100 + keyLen));
  char *key = ekonAllocatorAlloc(a, (sizeof(char) * (keyLen + 1)));
  ekonCopy(s + index, keyLen, key);
  key[index] = '\0';

  // <line>:<pos>:<key>:<message>
  snprintf(*message, 100, "%d:%d:%s:Duplicate key", line, pos, key);
  return false;
}

// compare strings
bool ekonStrIsEqual(const char *a, const char *b, uint32_t len) {
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
bool ekonStrIsEqualLen(const char *a, uint32_t a_len, const char *b,
                       uint32_t b_len) {
  if (EKON_LIKELY(a_len != b_len))
    return false;
  uint32_t i;
  for (i = 0; EKON_LIKELY(i < a_len); ++i) {
    if (EKON_LIKELY(a[i] != b[i]))
      return false;
  }
  return true;
}

// Init new Allocator - API
struct EkonAllocator *ekonAllocatorNew() {
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
void ekonAllocatorRelease(struct EkonAllocator *alloc) {
  struct EkonANode *next = alloc->root->next;
  while (EKON_LIKELY(next != 0)) {
    struct EkonANode *nn = next->next;
    ekonFree((void *)next);
    next = nn;
  }
  ekonFree((void *)alloc);
}

// append Child to the Allocator
bool ekonAllocatorAppendChild(uint32_t init_size, struct EkonAllocator *alloc) {
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
char *ekonAllocatorAlloc(struct EkonAllocator *alloc, uint32_t size) {
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

static struct EkonString *ekonStringCache = 0;

struct EkonString *ekonStringNew(struct EkonAllocator *alloc,
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

void ekonStringReset(struct EkonString *str) { str->pos = 0; }

bool ekonStringAppendStr(struct EkonString *str, const char *s, uint32_t size) {
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

bool ekonStringAppendChar(struct EkonString *str, const char c) {
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

bool ekonStringAppendEnd(struct EkonString *str) {
  return ekonStringAppendChar(str, 0);
}

const char *ekonStringStr(struct EkonString *str) { return str->data; }

void debPrintEkonType(const EkonType type) {
#ifdef DEBUG_FUNC
  switch (type) {
  case EKON_TYPE_STRING:
    printf("EKON_TYPE_STRING");
    break;
  case EKON_TYPE_BOOL:
    printf("EKON_TYPE_BOOL");
    break;
  case EKON_TYPE_NULL:
    printf("EKON_TYPE_NULL");
    break;
  case EKON_TYPE_OBJECT:
    printf("EKON_TYPE_OBJECT");
    break;
  case EKON_TYPE_ARRAY:
    printf("EKON_TYPE_ARRAY");
    break;
  case EKON_TYPE_NUMBER:
    printf("EKON_TYPE_NUMBER");
    break;
  }
#endif
}

void debPrintNodeMin(const struct EkonNode *node) {
#ifdef DEBUG_FUNC
  if (node != NULL || node != 0) {
    printf("{EkonType:");
    debPrintEkonType(node->ekonType);
    printf("}");
  } else
    printf("NULL");
#endif
}
void debPrintStrM(const char *s, uint32_t index, const char *mes,
                  uint32_t size) {
#ifdef DEBUG_FUNC
  if (size == 0)
    size = strlen(s);
  char *des = (char *)malloc(size + 1);
  strncpy(des, s + index, size);
  des[size] = 0;
  printf("%s~~>s[i]: `%s`\n", mes, des);
  free(des);
#endif
}

void debPrintChar(const char c) {
#ifdef DEBUG_FUNC
  printf("c:%c\n", c);
#endif
}
void debPrintCharM(const char c, const char *mes) {
#ifdef DEBUG_FUNC
  printf("%s~~>c:%c\n", mes, c);
#endif
}
void debPrintBool(const bool b) {
#ifdef DEBUG_FUNC
  printf("%s\n", b == 1 ? "true" : "false");
#endif
}

void debPrintBoolM(const bool b, const char *mes) {
#ifdef DEBUG_FUNC
  printf("%s~>%s\n", mes, b == 1 ? "true" : "false");
#endif
}
void printOption(uint8_t option, char *delimiter) {
#define TORF(x) (((option & (x)) != 0) == true ? "true" : "false")
  printf("%sisKeySpaced:%s,", delimiter, TORF(EKON_IS_KEY_SPACED));
  printf("%sisKeyMultilined:%s,", delimiter, TORF(EKON_IS_KEY_MULTILINED));
  printf("%sisStrSpaced:%s,", delimiter, TORF(EKON_IS_STR_SPACED));
  printf("%sisStrMultilined:%s,", delimiter, TORF(EKON_IS_STR_MULTILINED));
#undef TORF
}
void debPrintStr(const char *s, uint32_t index, uint32_t size) {
#ifdef DEBUG_FUNC
  if (size == 0)
    size = strlen(s);
  char *des = (char *)malloc(size + 1);
  strncpy(des, s + index, size);
  des[size] = 0;
  printf("s[i]:`%s`\n", des);
  free(des);
#endif
}
void debPrintStrMin(const char *s, uint32_t size) {
#ifdef DEBUG_FUNC
  if (s != NULL || s != 0) {
    if (size == 0)
      size = strlen(s);
    char *des = (char *)malloc(size + 1);
    strncpy(des, s, size);
    des[size] = 0;
    printf("`%s`", des);
    free(des);
  } else {
    printf("NULL");
  }
#endif
}

void debPrintTab(int depth) {
#ifdef DEBUG_FUNC
  for (int i = 0; i < depth; i++)
    printf("  ");
  printf("|");
#endif
}
void debPrintNodeSing(const struct EkonNode *n, int depth) {
#ifdef DEBUG_FUNC
  if (n != NULL || n != 0) {
    debPrintTab(depth);
    printf("{\n");
    debPrintTab(depth);
    printf("\tEkonType:");
    debPrintEkonType(n->ekonType);
    printf(",\n");
    debPrintTab(depth);
    printf("\tkey:");
    debPrintStrMin(n->key, n->keyLen);
    printf(",\n");
    debPrintTab(depth);
    printf("\tkeyLen:%d", n->keyLen);
    printf(",\n");
    debPrintTab(depth);
    printf("\toption:");
    printOption(n->option, "\t");
    printf(",\n");
    debPrintTab(depth);
    printf("\tvalue.str:");
    debPrintStrMin(n->value.str, n->len);
    printf(",\n");
    debPrintTab(depth);
    printf("\tlen:%d,", n->len);
    printf(",\n");
    debPrintTab(depth);
    printf("\tvalue.node:");
    debPrintNodeMin(n->value.node);
    printf(",\n");
    debPrintTab(depth);
    printf("\tprev:");
    debPrintNodeMin(n->prev);
    printf(",\n");
    debPrintTab(depth);
    printf("\tnext:");
    debPrintNodeMin(n->next);
    printf(",\n");
    debPrintTab(depth);
    printf("\tfather:");
    debPrintNodeMin(n->father);
    printf(",\n");
    debPrintTab(depth);
    printf("\tend:");
    debPrintNodeMin(n->end);
    printf("\n");
    debPrintTab(depth);
    printf("}\n");
  } else {
    printf("NULL\n");
  }
#endif
}

void debPrintNode(const struct EkonNode *n, int depth) {
#ifdef DEBUG_FUNC
  debPrintNodeSing(n, depth);
  if (n->ekonType == EKON_TYPE_OBJECT || n->ekonType == EKON_TYPE_ARRAY) {
    if (n->value.node) {
      debPrintNode(n->value.node, depth + 1);
    }
  }
  if (n->next != 0 || n->next != NULL) {
    debPrintNode(n->next, depth);
  }
#endif
}

// allocate new value into the memory and store the memory pointer
// in the v->allocator (v->a) value
struct EkonValue *ekonValueNew(struct EkonAllocator *alloc) {
  struct EkonValue *v =
      (struct EkonValue *)ekonAllocatorAlloc(alloc, sizeof(struct EkonValue));
  if (EKON_UNLIKELY(v == 0))
    return 0;
  v->a = alloc;
  v->n = 0;
  return v;
}

// add new value to EkonValue
struct EkonValue *ekonValueInnerNew(struct EkonAllocator *alloc,
                                    struct EkonNode *n) {
  struct EkonValue *v =
      (struct EkonValue *)ekonAllocatorAlloc(alloc, sizeof(struct EkonValue));
  if (EKON_UNLIKELY(v == 0))
    return 0;
  v->a = alloc;
  v->n = n;
  return v;
}

// ignore spacing
bool ekonSkin(const char c) {
  return EKON_UNLIKELY(
      EKON_UNLIKELY(c == ' ') ||
      EKON_UNLIKELY(c == '\t' ||
                    EKON_UNLIKELY(c == '\n' || EKON_UNLIKELY(c == '\r'))));
}

bool ekonConsume(const char c, const char *s, uint32_t *index);

bool ekonConsumeWhiteChars(const char *s, uint32_t *index) {
  while (ekonSkin(s[*index]))
    ++(*index);
  if (s[*index] == '/') {
    if (ekonConsumeComment(s, index) == false)
      return false;
  }
  while (ekonSkin(s[*index]))
    ++(*index);
  return true;
}

// peek current character
char ekonPeek(const char *s, uint32_t *index) {
  if (ekonConsumeWhiteChars(s, index) == false)
    return 0;
  return s[(*index)++];
}

// consume current character
bool ekonConsume(const char c, const char *s, uint32_t *index) {
  if (s[*index] == c) {
    ++(*index);
    return true;
  }
  return false;
}

// consume using likely
bool ekonLikelyConsume(const char c, const char *s, uint32_t *index) {
  if (EKON_LIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// consume using unlikely
bool ekonUnlikelyConsume(const char c, const char *s, uint32_t *index) {
  if (EKON_UNLIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// peek and consume using likely
bool ekonLikelyPeekAndConsume(const char c, const char *s, uint32_t *index) {
  if (ekonConsumeWhiteChars(s, index) == false)
    return false;

  if (EKON_LIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// peek and consume using unlikely
bool ekonUnlikelyPeekAndConsume(const char c, const char *s, uint32_t *index) {
  if (ekonConsumeWhiteChars(s, index) == false)
    return false;

  if (EKON_UNLIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// consume a comment
// multiline comments
bool ekonConsumeComment(const char *s, uint32_t *index) {
  char curr = s[(*index)];
  if (s[(*index)++] == '/' && s[(*index)++] == '/') {
    if (s[*index] == '\n') {
      (*index)++;
      if (ekonConsumeWhiteChars(s, index) == false)
        return false;
      return true;
    }

    if (curr == 0)
      return true;

    while (EKON_UNLIKELY(curr != '\n' && curr != 0))
      curr = s[(*index)++];

    if (ekonConsumeWhiteChars(s, index) == false)
      return false;

    return true;
  }
  return false;
}

// consume 'false' string
bool ekonConsumeFalse(const char *s, uint32_t *index) {
  if (EKON_LIKELY(*((uint32_t *)("alse")) == *((uint32_t *)(s + *index)))) {
    *index += 4;
    return true;
  }
  return false;
}

// consume 'true' string
bool ekonConsumeTrue(const char *s, uint32_t *index) {
  if (EKON_LIKELY(*((uint32_t *)ekonStrTrue) ==
                  *((uint32_t *)(s + *index - 1)))) {
    *index += 3;
    return true;
  }
  return false;
}

// consume 'null' string
bool ekonConsumeNull(const char *s, uint32_t *index) {
  if (EKON_LIKELY(*((uint32_t *)ekonStrNull) ==
                  *((uint32_t *)(s + *index - 1)))) {
    *index += 3;
    return true;
  }
  return false;
}

// check if its a hex character
uint32_t ekonHexCodePoint(const char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return 16;
}

// ekonValueGetUnEspaceStr
uint32_t ekonHexCodePointForUnEscape(const char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return c - 'a' + 10;
}

// consume Hex one char
bool ekonConsumeHexOne(const char *s, uint32_t *index, uint32_t *cp) {
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
void ekonConsumeHexOneForUnEscape(const char *s, uint32_t *index,
                                  uint32_t *cp) {
  *cp = *cp << 4;
  *cp += ekonHexCodePointForUnEscape(s[*index]);
  ++(*index);
  return;
}

// consume a hex code
bool ekonConsumeHex(const char *s, uint32_t *index, uint32_t *cp) {

  if (EKON_LIKELY(EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)))) {
    return true;
  }
  return false;
}

// consume Hex for unescaped
void ekonConsumeHexForUnEscape(const char *s, uint32_t *index, uint32_t *cp) {
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  return;
}

// ... apend string without having a length ...
void ekonAppend(char *s, uint32_t *index, char c) { s[(*index)++] = c; }

// ... append strings with length ...
void ekonAppendLen(char *s, uint32_t *index, const char *str, uint32_t len) {
  ekonCopy(str, len, s + (*index));
  *index += len;
}

// Append UTF-8
void ekonAppendUTF8(char *s, uint32_t *index, uint32_t codepoint) {
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
void ekonAppendEnd(char *s, uint32_t *index) { ekonAppend(s, index, 0); }

// Unescape Str
void ekonUnEscapeStr(const char *str, uint32_t len, char *s,
                     uint32_t *finalLen) {
  uint32_t sIndex = 0;
  uint32_t index;
  char c;
  for (index = 0; index < len;) {
    c = str[index];
    if (EKON_UNLIKELY(c == '\\')) {
      c = str[index + 1];
      switch (c) {
      case '"': {
        ekonAppend(s, &sIndex, '"');
        index += 2;
        break;
      }
      case '\'': {
        ekonAppend(s, &sIndex, '\'');
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
      case 'U': {
        index += 2;
        uint32_t cp = 0;
        ekonConsumeHexForUnEscape(str, &index, &cp);
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
  *finalLen = sIndex - 1;
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
    {"\x20", 1},    {"\x21", 1},    {"\"", 1},      {"\x23", 1},
    {"\x24", 1},    {"\x25", 1},    {"\x26", 1},    {"\\'", 2},
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
const char *ekonEscapeStr(const char *str, struct EkonAllocator *a,
                          uint32_t *finalLen) {
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
  *finalLen = len;
  return s;
}

const char *ekonEscapeStrJSON(const char *str, struct EkonAllocator *a,
                              uint32_t *finalLen) {
  uint32_t len = 0;
  const char *src = str;
  while (EKON_LIKELY(*str != 0)) {
    if (*str == '"' || *str == '\n' || *str == '\t') {
      if (*str == '"')
        len += 2;
      else
        len += ekonEscapeChars[(unsigned char)(*str)].len;
    } else {
      len++;
    }
    ++str;
  }
  char *s = ekonAllocatorAlloc(a, len + 1);
  if (EKON_UNLIKELY(s == 0))
    return 0;
  uint32_t index = 0;
  str = src;
  while (EKON_LIKELY(*str != 0)) {
    if (*str == '"' || *str == '\n' || *str == '\t' || *str == '\r') {
      if (*str == '"')
        ekonAppendLen(s, &index, "\\\"", 2);
      else
        ekonAppendLen(s, &index, ekonEscapeChars[(unsigned char)(*str)].str,
                      ekonEscapeChars[(unsigned char)(*str)].len);
    } else {
      const char str2[2] = {(char)*str, '\0'};
      ekonAppendLen(s, &index, str2, 1);
    }
    ++str;
  }
  ekonAppendEnd(s, &index);
  *finalLen = len;
  return s;
}

// str escape len with str
const char *ekonEscapeStrLen(const char *str, struct EkonAllocator *a,
                             uint32_t len) {
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
bool ekonConsumeStr(const char *s, uint32_t *index, const char quoteType,
                    uint8_t *option) {
  char c = s[*index];
  while (EKON_LIKELY(c != 0)) {
    if (c == '\n') {
      if (EKON_LIKELY(quoteType != '\'')) {
        return false;
      } else {
        *option = *option | EKON_IS_STR_SPACED | EKON_IS_STR_MULTILINED;
        c = s[++(*index)];
        continue;
      }
    }

    if (c == ' ' || c == '\t') {
      *option = *option | EKON_IS_STR_SPACED;
      c = s[++(*index)];
      continue;
    }

    if (EKON_UNLIKELY((unsigned char)c <= 0x1f))
      return false;

    if (EKON_UNLIKELY(c == '\\')) {
      c = s[++(*index)];
      switch (c) {
      case '\\':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
      case '"':
      case '\'':
      case '/': {
        c = s[++(*index)];
        continue;
      }
      case 'U':
      case 'u': {
        c = s[++(*index)];
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
              } else {
                return false;
              }
            } else
              return false;
          }
          c = s[(*index)];
        } else
          return false;
        continue;
      }
      case 'x': {
        c = s[++(*index)];
        uint32_t cp = 0;
        if (EKON_LIKELY(ekonConsumeHexOne(s, index, &cp)) &&
            EKON_LIKELY(ekonConsumeHexOne(s, index, &cp))) {
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0XDFFF))
            return false;

          /* printf("%s %x\n", s + *index, 0xD800); */
          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, index) &&
                            ekonLikelyConsume('x', s, index))) {
              uint32_t cp2 = 0;
              if (EKON_LIKELY(ekonConsumeHexOne(s, index, &cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, &cp))) {
                if (EKON_UNLIKELY(cp2 < 0xDC00 || cp2 > 0xDFFF))
                  return false;
              }
              return false;
            } else {
              return false;
            }
          }
          c = s[(*index)];
        } else
          return false;
        continue;
      }
      default: {
      }
      }
    }

    if ((*option & EKON_IS_STR_SPACED) == 0 &&
        (c == '[' || c == ']' || c == ':' || c == '{' || c == '}' ||
         c == ',')) {
      *option |= EKON_IS_STR_SPACED;
    }

    if (EKON_UNLIKELY(c == quoteType)) {
      (*index)++;
      return true;
    }

    c = s[++(*index)];
  }
  return false;
}

bool ekonConsumeSchema(const char *s, uint32_t *index) {
  char c = s[*index];
  while (EKON_LIKELY(c != 0)) {
    if (EKON_UNLIKELY(c == '`')) {
      return true;
    }
    c = s[(*index)++];
  }
  return false;
}
// consume a string in the form of a word
bool ekonConsumeUnquotedStr(const char *s, uint32_t *index) {
  char c = s[(*index)];
  while (EKON_LIKELY(c != 0)) {
    if (ekonIsNonUnquoteStrChar(c)) {
      return true;
    }

    if (EKON_UNLIKELY((unsigned char)c <= 0x1f)) {
      return false;
    }

    c = s[++(*index)];
  }
  return true;
}

// Check String, Set Str
bool ekonCheckStr(const char *s, uint32_t *len) {
  uint32_t index = 0;
  char c = s[index++];
  while (EKON_LIKELY(c != 0)) {
    if (EKON_UNLIKELY(EKON_UNLIKELY((unsigned char)c <= 0x1f))) {
      if (!ekonSkin(c)) {
        return false;
      }
    }
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
      case '\'':
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
      case 'x': {
        uint32_t cp = 0;
        if (EKON_LIKELY(ekonConsumeHexOne(s, &index, &cp)) &&
            EKON_LIKELY(ekonConsumeHexOne(s, &index, &cp))) {
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0xDFFFF))
            return false;
          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, &index) &&
                            ekonLikelyConsume('x', s, &index))) {
              uint32_t cp2 = 0;
              if (EKON_LIKELY(ekonConsumeHexOne(s, &index, &cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, &index, &cp))) {
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
bool ekonCheckStrLen(struct EkonAllocator *alloc, const char *s, uint32_t len) {
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
bool ekonConsumeNum(const char *s, uint32_t *index) {
  // handle unary minus
  if ((ekonConsume('-', s, index) || ekonConsume('+', s, index))) {
  }

  bool decimalStarted = false;

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
    if (ekonLikelyConsume('b', s, index)) {
      uint8_t i = 0;
      char c = s[*index];
      while ((c == '0' || c == '1' || (c == '_' && i > 0))) {
        c = s[++(*index)];
        i++;
      }
      if (c == ',' || c == '[' || c == ']' || c == '{' || c == '}' ||
          c == '/' || c == '"' || c == '\'' || ekonSkin(c)) {
        return true;
      }
      return false;
    }
    if (ekonLikelyConsume('o', s, index)) {
      uint8_t i = 0;
      char c = s[*index];
      while (((c >= '0' && c <= '7') || (c == '_' && i > 0))) {
        c = s[++(*index)];
        i++;
      }
      if (c == ',' || c == '[' || c == ']' || c == '{' || c == '}' ||
          c == '/' || c == '"' || c == '\'' || ekonSkin(c)) {
        return true;
      }
      return false;
    }
  } else if (EKON_LIKELY(EKON_LIKELY(s[*index] >= '1') &&
                         EKON_LIKELY(s[*index] <= '9'))) {
    char c = s[*index];
    while (EKON_LIKELY(c >= '0' && c <= '9'))
      c = s[++(*index)];
  } else
    return false;

  if (ekonConsume('.', s, index)) {
    if (decimalStarted == true)
      return false;

    decimalStarted = true;

    char c = s[*index];
    if ((EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9'))) {
      c = s[++(*index)];
      while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9')))
        c = s[++(*index)];
    } else if (ekonSkin(c) || c == ',' || c == '[' || c == ']' || c == '{' ||
               c == '}' || c == '/' || c == '"' || c == '\'') {
      ++(*index);
      return true;
    } else {
      return false;
    }
  }

  if (ekonConsume('.', s, index))
    return false;

  if (s[*index] == 'e' || s[*index] == 'E') {
    char c = s[++(*index)];
    if (c == '-' || c == '+')
      ++(*index);
    c = s[*index];
    if (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9'))) {
      c = s[++(*index)];
      while (EKON_LIKELY(EKON_LIKELY(c >= '0') && EKON_LIKELY(c <= '9'))) {
        c = s[++(*index)];
      }
      if (ekonSkin(c) || c == ',' || c == '[' || c == ']' || c == '{' ||
          c == '}' || c == '/' || c == '"' || c == '\'') {
        return true;
      }
    } else
      return false;
  }

  return true;
}

// check a number
bool ekonCheckNum(const char *s, uint32_t *len) {
  uint32_t index = 0;

  if (s[index] == '-' || s[index] == '+')
    ++(index);

  if (ekonUnlikelyConsume('0', s, &index)) {
    if (ekonLikelyConsume('x', s, &index) ||
        ekonLikelyConsume('X', s, &index)) {
      uint8_t i = 0;
      char c = s[index];
      while ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ||
             (c >= '0' && c <= '9')) {
        if (i > 4)
          return false;
        c = s[++index];
        i++;
      }
      *len = index;
      return true;
    }
    if (ekonLikelyConsume('b', s, &index)) {
      uint8_t i = 0;
      char c = s[index];
      while ((c == '0' || c == '1' || (c == '_' && i > 0))) {
        c = s[++(index)];
        i++;
      }
      if (c == ',' || c == '[' || c == ']' || c == '{' || c == '}' ||
          c == '/' || c == '"' || c == '\'' || ekonSkin(c)) {
        *len = index;
        return true;
      }
      return false;
    }
    if (ekonLikelyConsume('o', s, &index)) {
      uint8_t i = 0;
      char c = s[index];
      while (((c >= '0' && c <= '7') || (c == '_' && i > 0))) {
        c = s[++(index)];
        i++;
      }
      if (c == ',' || c == '[' || c == ']' || c == '{' || c == '}' ||
          c == '/' || c == '"' || c == '\'' || ekonSkin(c)) {
        *len = index;
        return true;
      }
      return false;
    }
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

bool ekonCheckNumLen(struct EkonAllocator *alloc, const char *s, uint32_t len) {
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

void ekonNodeAddStr(struct EkonNode *node, const char *s, uint32_t len,
                    uint8_t option) {
  node->option = option;
  node->value.str = s;
  node->len = len;
  node->ekonType = EKON_TYPE_STRING;
}

void ekonNodeAddNull(struct EkonNode *node) {
  node->ekonType = EKON_TYPE_NULL;
  node->value.str = ekonStrNull;
  node->len = 4;
}
void ekonNodeAddBoolean(struct EkonNode *node, bool val) {
  node->ekonType = EKON_TYPE_BOOL;
  if (val == true) {
    node->value.str = ekonStrTrue;
    node->len = 4;
  } else {
    node->value.str = ekonStrFalse;
    node->len = 5;
  }
}

void ekonNodeAddNumber(struct EkonNode *node, const char *s, uint32_t len) {
  node->value.str = s;
  node->len = len;
  node->ekonType = EKON_TYPE_NUMBER;
}
void ekonNodeAddKey(struct EkonValue *v, const char *s, uint32_t len,
                    uint8_t option, EkonHashTable *keyTable) {
  v->n->key = s;
  v->n->keyLen = len;
  v->n->option = option;
  ekonHashSet(v->a, v->n->table, key, keyLen, v->n);
  hashsetAddMember(keySet, s, len);
}

bool ekonSrcNodeError(struct EkonNode *srcNode, struct EkonValue *v,
                      const char *s, char **errMessage, uint32_t index) {
  if (EKON_LIKELY(srcNode == 0))
    v->n = srcNode;
  else
    *v->n = *srcNode;
  ekonError(errMessage, v->a, s, index);
  return false;
}

bool ekonNodeAddObjOrArrNode(struct EkonNode **node, struct EkonValue *v,
                             struct EkonNode *srcNode, const char *s,
                             uint32_t *index, char **errMessage, bool isObj,
                             bool isRootObj) {
  if (isObj == false) {
    (*node)->ekonType = EKON_TYPE_ARRAY;
    if (ekonUnlikelyPeekAndConsume(']', s, index)) {
      (*node)->value.node = 0;
      (*node)->len = 0;
      return true;
    }
  } else {
    (*node)->ekonType = EKON_TYPE_OBJECT;
    if (isRootObj == false && ekonUnlikelyPeekAndConsume('}', s, index)) {
      (*node)->value.node = 0;
      (*node)->len = 0;
      return true;
    }
  }

  struct EkonNode *n =
      (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));

  if (EKON_UNLIKELY(n == 0))
    return ekonSrcNodeError(srcNode, v, s, errMessage, *index);

  n->father = *node;
  n->prev = 0;

  if (isObj == true) {
    (*node)->keySet = hashsetCreate(v->a, errMessage, s, *index);
  }
  (*node)->value.node = n;
  (*node)->end = n;
  (*node)->len = 1;
  *node = n;
  return true;
}

bool ekonValueParseFast(struct EkonValue *v, const char *s, char **errMessage,
                        char **schema) {
  if (EKON_UNLIKELY(s[0] == '\0')) {
    ekonError(errMessage, v->a, s, 0);
    return false;
  }

  struct EkonNode *srcNode;

  if (EKON_LIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0)) {
      ekonError(errMessage, v->a, s, 0);
      return false;
    }
    v->n->prev = 0;
    v->n->next = 0;
    v->n->father = 0;
    v->n->key = 0;
    srcNode = 0;
  } else {
    srcNode =
        (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(srcNode == 0)) {
      ekonError(errMessage, v->a, s, 0);
      return false;
    }
    *srcNode = *v->n;
  }

  uint32_t index = 0;

  struct EkonNode *node = v->n;
  bool isRootNoCurlyBrace = false;

  char c = ekonPeek(s, &index);
  if (c == '`') {
    const uint32_t start = index;
    if (ekonConsumeSchema(s, &index) == false)
      return ekonError(errMessage, v->a, s, index);

    if (*schema == NULL) {
      *schema = ekonCopySchema(s + start, index - start - 1);
    }
    c = ekonPeek(s, &index);
  }

  const uint32_t ifRootStart = index - 1;

  switch (c) {
  case '[': {
    if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage, false,
                                false))
      break;
    return false;
  }
  case '{': {
    if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage, true,
                                false))
      break;
    return false;
  }
  case 'n': {
    uint32_t start = index - 1;
    if (EKON_LIKELY(ekonConsumeNull(s, &index))) {
      if (ekonIsNonUnquoteStrChar(s[index])) {
        ekonNodeAddNull(node);
        break;
      }
      if (ekonConsumeUnquotedStr(s, &index)) {
        uint32_t strEnd = index;
        if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        ekonNodeAddStr(node, s + start, strEnd - start, (EKON_NODE_OPTIONS)0);
        break;
      }
    }
    if (ekonConsumeUnquotedStr(s, &index)) {
      uint32_t strEnd = index;
      if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      ekonNodeAddStr(node, s + start, strEnd - start, (EKON_NODE_OPTIONS)0);
      break;
    }
    return ekonSrcNodeError(srcNode, v, s, errMessage, index);
  }
  case 'f': {
    uint32_t start = index - 1;
    if (EKON_LIKELY(ekonConsumeFalse(s, &index))) {
      if (ekonIsNonUnquoteStrChar(s[index])) {
        ekonNodeAddBoolean(node, false);
        break;
      }
      if (ekonConsumeUnquotedStr(s, &index)) {
        uint32_t strEnd = index;
        if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        ekonNodeAddStr(node, s + start, strEnd - start, (EKON_NODE_OPTIONS)0);
        break;
      }
    }
    if (ekonConsumeUnquotedStr(s, &index)) {
      uint32_t strEnd = index;
      if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      ekonNodeAddStr(node, s + start, strEnd - start, (EKON_NODE_OPTIONS)0);
      break;
    }
    return ekonSrcNodeError(srcNode, v, s, errMessage, index);
  }
  case 't': {
    uint32_t start = index - 1;
    if (EKON_LIKELY(ekonConsumeTrue(s, &index))) {
      if (ekonIsNonUnquoteStrChar(s[index]) == true) {
        ekonNodeAddBoolean(node, true);
        break;
      }
      if (ekonConsumeUnquotedStr(s, &index) == true) {
        uint32_t strEnd = index;
        if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        ekonNodeAddStr(node, s + start, strEnd - start, (EKON_NODE_OPTIONS)0);
        break;
      }
    }
    if (ekonConsumeUnquotedStr(s, &index)) {
      uint32_t strEnd = index;
      if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      ekonNodeAddStr(node, s + start, strEnd - start, (EKON_NODE_OPTIONS)0);
      break;
    }
    return ekonSrcNodeError(srcNode, v, s, errMessage, index);
  }
  case '\'':
  case '"': {
    uint32_t start = index;
    if (EKON_UNLIKELY(ekonUnlikelyConsume(c, s, &index))) {
      ekonNodeAddStr(node, s + index, 0, EKON_IS_STR_SPACED);
      break;
    }

    uint8_t option = 0;
    if (c == '"')
      option |= EKON_IS_STR_ESCAPABLE;

    if (EKON_LIKELY(ekonConsumeStr(s, &index, c, &option))) {
      uint32_t strEnd = index - 1;
      if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      ekonNodeAddStr(node, s + start, strEnd - start,
                     (EKON_NODE_OPTIONS)option);
      break;
    }
    return ekonSrcNodeError(srcNode, v, s, errMessage, index);
  }
  default: {
    index--;
    uint32_t start = index;
    if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
      if (ekonConsumeNum(s, &index)) {
        uint32_t numEnd = index;
        if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        ekonNodeAddNumber(node, s + start, numEnd - start);
        break;
      } else {
        index = start;
        if (ekonConsumeUnquotedStr(s, &index)) {
          uint32_t numEnd = index;
          if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
            isRootNoCurlyBrace = true;
            index = ifRootStart;
            break;
          }
          ekonNodeAddStr(node, s + start, numEnd - start - 1, 0);
          index--;
          break;
        }
      }
    }

    if (ekonConsumeUnquotedStr(s, &index)) {
      uint32_t strEnd = index;
      if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
        isRootNoCurlyBrace = true;
        index = ifRootStart;
        break;
      }
      ekonNodeAddStr(node, s + start, strEnd - start, 0);
      break;
    }

    return ekonSrcNodeError(srcNode, v, s, errMessage, index);
  }
  }

  if (isRootNoCurlyBrace == true) {
    if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage, true,
                                true) == false) {
      return false;
    }
  }

  while (EKON_LIKELY(node != v->n)) {
    uint8_t option = 0;

    if (node->father->ekonType == EKON_TYPE_OBJECT) {
      struct Hashset *keySet = node->father->keySet;
      bool isKeyUnquoted = ekonIsQuote(s[index]) == false;
      if (isKeyUnquoted) {
        uint32_t start = index;
        if (ekonConsumeUnquotedStr(s, &index)) {
          uint32_t keyLen = index - start;
          if (hashsetIsMember(keySet, s + start, keyLen)) {
            return ekonDuplicateKeyError(errMessage, v->a, s, start, keyLen);
          } else
            ekonNodeAddKey(node, s + start, index - start, 0, keySet);
        } else {
          return ekonSrcNodeError(srcNode, v, s, errMessage, index);
        }
      } else {
        char quoteType = s[index];
        if (quoteType == '"')
          option |= EKON_IS_KEY_ESCAPABLE;

        uint32_t start = ++index;
        if (EKON_UNLIKELY(ekonUnlikelyConsume(quoteType, s, &index))) {
          if (hashsetIsMember(keySet, s + start, 0)) {
            ekonDuplicateKeyError(errMessage, v->a, s, index, 0);
            return false;
          } else {
            option |= EKON_IS_KEY_SPACED;
            ekonNodeAddKey(node, s, 0, option, keySet);
          }
        } else {
          if (EKON_UNLIKELY(ekonConsumeStr(s, &index, quoteType, &option) ==
                            false)) {
            return ekonSrcNodeError(srcNode, v, s, errMessage, index);
          }

          if (hashsetIsMember(keySet, s + start, index - start - 1)) {
            ekonDuplicateKeyError(errMessage, v->a, s + start, index,
                                  index - start - 1);
            return false;
          } else {
            if (option & EKON_IS_STR_SPACED)
              option |= EKON_IS_KEY_SPACED;
            if (option & EKON_IS_STR_MULTILINED)
              option |= EKON_IS_KEY_MULTILINED;
            option &= ~EKON_IS_STR_MULTILINED;
            option &= ~EKON_IS_STR_SPACED;

            ekonNodeAddKey(node, s + start, index - start - 1, option, keySet);
          }
        }
      }

      if (EKON_UNLIKELY(ekonLikelyPeekAndConsume(':', s, &index) == false))
        return ekonSrcNodeError(srcNode, v, s, errMessage, index);
    } else {
      node->key = 0;
    }

    c = ekonPeek(s, &index);

    switch (c) {
    case '[': {
      struct EkonNode *currNode = node;
      if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage,
                                  false, false) == false) {
        return false;
      }
      if (currNode == node)
        break;

      if (ekonPeek(s, &index) == ',')
        return ekonError(errMessage, v->a, s, index);

      index--;
      continue;
    }
    case '{': {
      struct EkonNode *currNode = node;
      if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage,
                                  true, false) == false) {
        return false;
      }

      if (currNode == node)
        break;

      char nextChar = ekonPeek(s, &index);
      if (nextChar == ':' || nextChar == ',')
        return ekonError(errMessage, v->a, s, index);

      index--;
      continue;
    }
    case 'n': {
      uint32_t start = index - 1;
      if (EKON_LIKELY(ekonConsumeNull(s, &index))) {
        if (ekonIsNonUnquoteStrChar(s[index]) == true) {
          ekonNodeAddNull(node);
          break;
        }

        if (ekonConsumeUnquotedStr(s, &index)) {
          option &= ~EKON_IS_STR_MULTILINED;
          option &= ~EKON_IS_STR_SPACED;
          ekonNodeAddStr(node, s + start, index - start, option);
          break;
        }
      }

      index--;
      if (ekonConsumeUnquotedStr(s, &index)) {
        option &= ~EKON_IS_STR_MULTILINED;
        option &= ~EKON_IS_STR_SPACED;
        ekonNodeAddStr(node, s + start, index - start, option);
        break;
      }

      return ekonSrcNodeError(node, v, s, errMessage, index);
    }
    case 'f': {
      uint32_t start = index - 1;
      if (EKON_LIKELY(ekonConsumeFalse(s, &index))) {
        if (ekonIsNonUnquoteStrChar(s[index]) == true) {
          ekonNodeAddBoolean(node, false);
          break;
        }

        if (ekonConsumeUnquotedStr(s, &index)) {
          option &= ~EKON_IS_STR_MULTILINED;
          option &= ~EKON_IS_STR_SPACED;
          ekonNodeAddStr(node, s + start, index - start, option);
          break;
        }
      }

      index--;
      if (ekonConsumeUnquotedStr(s, &index)) {
        option &= ~EKON_IS_STR_MULTILINED;
        option &= ~EKON_IS_STR_SPACED;
        ekonNodeAddStr(node, s + start, index - start, option);
        break;
      }

      return ekonSrcNodeError(node, v, s, errMessage, index);
    }
    case 't': {
      uint32_t start = index - 1;
      if (EKON_LIKELY(ekonConsumeTrue(s, &index))) {
        if (ekonIsNonUnquoteStrChar(s[index]) == true) {
          ekonNodeAddBoolean(node, true);
          break;
        }

        if (ekonConsumeUnquotedStr(s, &index)) {
          option &= ~EKON_IS_STR_MULTILINED;
          option &= ~EKON_IS_STR_SPACED;
          ekonNodeAddStr(node, s + start, index - start, option);
          break;
        }
      }

      index--;
      if (ekonConsumeUnquotedStr(s, &index)) {
        option &= ~EKON_IS_STR_MULTILINED;
        option &= ~EKON_IS_STR_SPACED;
        ekonNodeAddStr(node, s + start, index - start, option);
        break;
      }
      return ekonSrcNodeError(node, v, s, errMessage, index);
    }
    case '\'':
    case '"': {
      uint32_t start = index;
      if (c == '"')
        option |= EKON_IS_STR_ESCAPABLE;

      if (EKON_UNLIKELY(ekonUnlikelyConsume(c, s, &index))) {
        option |= EKON_IS_STR_SPACED;
        option &= ~EKON_IS_STR_MULTILINED;
        ekonNodeAddStr(node, s + index, 0, option);
        break;
      }

      if (EKON_LIKELY(ekonConsumeStr(s, &index, c, &option))) {
        ekonNodeAddStr(node, s + start, index - start - 1, option);
        break;
      }
      return ekonSrcNodeError(srcNode, v, s, errMessage, index);
    }
    default: {
      if (c == ',')
        return ekonError(errMessage, v->a, s, index);

      index--;
      uint32_t start = index;
      if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
        if (ekonConsumeNum(s, &index)) {
          ekonNodeAddNumber(node, s + start, index - start);
          break;
        } else {
          index = start;
          if (ekonConsumeUnquotedStr(s, &index)) {
            uint32_t numEnd = index;
            option &= ~EKON_IS_STR_SPACED;
            option &= ~EKON_IS_STR_MULTILINED;
            ekonNodeAddStr(node, s + start, numEnd - start, option);
            break;
          }
        }
      }

      if (ekonConsumeUnquotedStr(s, &index)) {
        uint32_t strEnd = index;
        option &= ~EKON_IS_STR_SPACED;
        option &= ~EKON_IS_STR_MULTILINED;
        ekonNodeAddStr(node, s + start, strEnd - start, option);
        break;
      }

      return ekonSrcNodeError(srcNode, v, s, errMessage, index);
    }
    }

    while (EKON_LIKELY(node != v->n)) {
      char c = ekonPeek(s, &index);
      if (c == ',')
        c = ekonPeek(s, &index);

      if (c == ',') {
        ekonError(errMessage, v->a, s, index);
        return false;
      }

      if (c == 0) {
        if (isRootNoCurlyBrace) {
          node->next = 0;
          return true;
        } else {
          ekonError(errMessage, v->a, s, index);
          return false;
        }
      }

      if (c == ':') {
        ekonError(errMessage, v->a, s, index);
        return false;
      }

      if (c == '}' || c == ']') {
        if (c == '}') {
          if (node->father->ekonType == EKON_TYPE_OBJECT) {
            node->next = 0;
            node = node->father;
          }
        } else {
          if (node->father->ekonType == EKON_TYPE_ARRAY) {
            node->next = 0;
            node = node->father;
          }
        }
      } else {
        struct EkonNode *n = (struct EkonNode *)ekonAllocatorAlloc(
            v->a, sizeof(struct EkonNode));

        if (EKON_UNLIKELY(n == 0))
          return ekonSrcNodeError(srcNode, v, s, errMessage, index);

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

  if (EKON_LIKELY(ekonLikelyPeekAndConsume(0, s, &index)))
    return true;

  return ekonSrcNodeError(srcNode, v, s, errMessage, index);
}

// ekon parse - API
bool ekonValueParseLen(struct EkonValue *v, const char *s, uint32_t len,
                       char **err, char **schema) {
  char *str = ekonAllocatorAlloc(v->a, len + 1);
  if (EKON_UNLIKELY(str == 0))
    return false;
  ekonCopy(s, len, str);
  str[len] = 0;
  return ekonValueParseFast(v, s, err, schema);
}

// The main parser - API
bool ekonValueParse(struct EkonValue *v, const char *s, char **err,
                    char **schema) {
  return ekonValueParseLen(v, s, ekonStrLen(s), err, schema);
}

// -------------- util functions for stringifying ----------------
// append Quotes
const bool ekonAppendQuote(const struct EkonNode *node,
                           struct EkonString *str) {
  if (EKON_UNLIKELY(ekonStringAppendChar(str, '\'') == false))
    return false;
  return true;
}
bool ekonUtilAppendArray(struct EkonString *str, struct EkonNode **node) {
  if (EKON_UNLIKELY(ekonStringAppendChar(str, '[') == false))
    return false;

  if ((*node)->value.node != 0) {
    *node = (*node)->value.node;
  } else {
    if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
      return false;
  }
  return true;
}

bool ekonUtilAppendObj(struct EkonString *str, struct EkonNode **node,
                       bool isRootObj) {
  if ((*node)->value.node != 0) {
    if (isRootObj == false) {
      if (EKON_UNLIKELY(ekonStringAppendChar(str, '{') == false))
        return false;
    }
    *node = (*node)->value.node;
  } else {
    if (EKON_UNLIKELY(ekonStringAppendStr(str, "{}", 2) == false))
      return false;
  }
  return true;
}
#define APPEND_QUOTE(str, isJSON)                                              \
  (EKON_UNLIKELY(ekonStringAppendChar((str), (isJSON) ? '"' : '\'')))

bool ekonUtilAppendStr(struct EkonString *str, struct EkonNode *node,
                       const struct EkonValue *v, bool unEscapeString,
                       const bool isJSON) {
  if ((node->option & EKON_IS_STR_SPACED) != 0 ||
      (node->option & EKON_IS_STR_MULTILINED) != 0) {
    if (APPEND_QUOTE(str, isJSON) == false)
      return false;
  }

  uint32_t finalLen = node->len;
  const char *ss;
  if ((node->option & EKON_IS_STR_ESCAPABLE) != 0) {
    ss = ekonEscapeStr(node->value.str, v->a, &finalLen);
  } else {
    ss = node->value.str;
  }

  if (unEscapeString) {
    char *retStr = ekonAllocatorAlloc(v->a, node->len + 1);
    if (EKON_UNLIKELY(retStr == 0))
      return 0;

    uint32_t finalLen2;
    ekonUnEscapeStr(ss, finalLen, retStr, &finalLen2);
    if (EKON_UNLIKELY(ekonStringAppendStr(str, retStr, finalLen2) == false))
      return 0;
  } else {
    if (EKON_UNLIKELY(ekonStringAppendStr(str, ss, finalLen) == false))
      return 0;
  }

  if ((node->option & EKON_IS_STR_SPACED) != 0 ||
      (node->option & EKON_IS_STR_MULTILINED) != 0) {
    if (APPEND_QUOTE(str, isJSON) == false)
      return 0;
  }
  return true;
}
bool ekonUtilAppendKey(struct EkonString *str, struct EkonNode *node,
                       const struct EkonValue *v, bool unEscapeString,
                       const bool isJSON) {
  if (node->key != 0) {
    if ((node->option & EKON_IS_KEY_SPACED) != 0 ||
        (node->option & EKON_IS_KEY_MULTILINED) != 0) {
      if (APPEND_QUOTE(str, isJSON) == false)
        return false;
    }
    if (EKON_UNLIKELY(ekonStringAppendStr(str, node->key, node->keyLen) ==
                      false))
      return false;
    if ((node->option & EKON_IS_KEY_SPACED) != 0 ||
        (node->option & EKON_IS_KEY_MULTILINED) != 0) {
      if (APPEND_QUOTE(str, isJSON) == false)
        return false;
    }
    if (EKON_UNLIKELY(ekonStringAppendChar(str, ':') == false))
      return false;
  }
  return true;
}
// ---------------------------------------------------------------

// stringify an EKON node - with option to unescape
const char *ekonValueStringify(const struct EkonValue *v, bool unEscapeString) {
  if (EKON_UNLIKELY(v->n == 0))
    return "";

  struct EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  struct EkonNode *node = v->n;

  switch (node->ekonType) {
  case EKON_TYPE_ARRAY: {
    if (ekonUtilAppendArray(str, &node) == false)
      return 0;
    break;
  }
  case EKON_TYPE_OBJECT: {
    node = node->value.node;
    break;
  }
  case EKON_TYPE_STRING: {
    if (ekonUtilAppendStr(str, node, v, false, false) == false)
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
    if (ekonUtilAppendKey(str, node, v, unEscapeString, false) == false)
      return 0;

    switch (node->ekonType) {
    case EKON_TYPE_ARRAY: {
      const struct EkonNode *currentNode = node;
      if (ekonUtilAppendArray(str, &node) == false)
        return 0;
      if (currentNode == node)
        break;
      continue;
    }
    case EKON_TYPE_OBJECT: {
      const struct EkonNode *currentNode = node;
      if (ekonUtilAppendObj(str, &node, false) == false)
        return 0;
      if (currentNode == node)
        break;
      continue;
    }
    case EKON_TYPE_STRING: {
      if (ekonUtilAppendStr(str, node, v, unEscapeString, false) == false)
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
        if ((node->option & EKON_IS_STR_SPACED) == 0) {
          if (EKON_UNLIKELY(ekonStringAppendChar(str, ' ') == false))
            return 0;
        }
        node = node->next;
        break;
      } else {
        node = node->father;
        if (node->ekonType == EKON_TYPE_ARRAY) {
          if (EKON_UNLIKELY(ekonStringAppendChar(str, ']') == false))
            return 0;
        } else {
          if (node->father != 0 && ekonStringAppendChar(str, '}') == false)
            return 0;
        }
      }
    }
  }

  if (EKON_UNLIKELY(ekonStringAppendEnd(str) == false))
    return 0;

  return ekonStringStr(str);
}

// stringify a value - with option to unescape
const char *ekonValueStringifyToJSON(const struct EkonValue *v,
                                     bool unEscapeString) {
  if (EKON_UNLIKELY(v->n == 0))
    return "";

  struct EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  struct EkonNode *node = v->n;

  switch (node->ekonType) {
  case EKON_TYPE_ARRAY: {
    if (ekonUtilAppendArray(str, &node) == false)
      return 0;
    break;
  }
  case EKON_TYPE_OBJECT: {
    if (ekonUtilAppendObj(str, &node, false) == false)
      return 0;
    break;
  }
  case EKON_TYPE_STRING: {
    if (ekonUtilAppendStr(str, node, v, unEscapeString, true) == false)
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
    if (ekonUtilAppendKey(str, node, v, unEscapeString, true) == false)
      return 0;

    switch (node->ekonType) {
    case EKON_TYPE_ARRAY: {
      if (ekonUtilAppendArray(str, &node) == false)
        return 0;
      break;
    }
    case EKON_TYPE_OBJECT: {
      if (ekonUtilAppendObj(str, &node, false) == false)
        return 0;
      break;
    }
    case EKON_TYPE_STRING: {
      if (ekonUtilAppendStr(str, node, v, unEscapeString, true) == false)
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
        if (node->ekonType == EKON_TYPE_ARRAY) {
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

bool ekonStringAppendTab(struct EkonString *str, const char *s,
                         uint32_t depth) {
  for (int i = 0; i < depth; i++) {
    if (EKON_UNLIKELY(ekonStringAppendStr(str, s, strlen(s))) == false)
      return false;
  }
  return true;
}

const char *ekonValueBeautify(struct EkonValue *v, char **err,
                              bool unEscapeString, bool asJSON) {
  struct EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  struct EkonNode *node = v->n;
  uint32_t depth = 0;

  switch (node->ekonType) {
  case EKON_TYPE_ARRAY: {
    depth += 1;
    if (ekonUtilAppendArray(str, &node) == false)
      return 0;
    break;
  }
  case EKON_TYPE_OBJECT: {
    if (ekonUtilAppendObj(str, &node, true) == false)
      return 0;
    break;
  }
  case EKON_TYPE_STRING: {
    if (ekonUtilAppendStr(str, node, v, unEscapeString, asJSON) == false)
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
    switch (node->father->ekonType) {
    case EKON_TYPE_OBJECT:
    case EKON_TYPE_ARRAY:
      if (ekonStringAppendTab(str, "  ", depth) == false)
        return 0;
    default:;
    }

    if (ekonUtilAppendKey(str, node, v, unEscapeString, asJSON) == false)
      return 0;

    switch (node->ekonType) {
    case EKON_TYPE_ARRAY: {
      depth++;
      const struct EkonNode *currentNode = node;
      if (ekonUtilAppendArray(str, &node) == false)
        return 0;
      if (ekonStringAppendStr(str, "\n", 1) == false)
        return false;
      if (currentNode == node)
        break;
      continue;
    }
    case EKON_TYPE_OBJECT: {
      depth++;
      const struct EkonNode *currentNode = node;
      if (ekonUtilAppendObj(str, &node, false) == false)
        return 0;
      if (ekonStringAppendStr(str, "\n", 1) == false)
        return false;
      if (currentNode == node)
        break;
      continue;
    }
    case EKON_TYPE_STRING: {
      if (ekonUtilAppendStr(str, node, v, unEscapeString, asJSON) == false)
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
      if (EKON_LIKELY(node->next != NULL)) {
        if (EKON_UNLIKELY(ekonStringAppendChar(str, '\n') == false))
          return 0;
        node = node->next;
        break;
      } else {
        if (depth > 0)
          depth--;
        if (EKON_UNLIKELY(ekonStringAppendChar(str, '\n')) == false)
          return 0;
        if (ekonStringAppendTab(str, "  ", depth) == false)
          return 0;

        node = node->father;
        if (node->ekonType == EKON_TYPE_ARRAY) {
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

  char *des = (char *)malloc(str->size);
  ekonCopy(str->data, str->size, des);

  return des;
}

// TODO: preserve comments
const char *ekonBeautify(const char *src, char **err,
                         EkonBeautifyOptions options) {
  bool unEscapeString = options.unEscapeString;
  bool asJson = options.asJSON;
  bool preserveComments = options.preserveComments;
  struct EkonAllocator *a = ekonAllocatorNew();
  struct EkonValue *v = ekonValueNew(a);

  char *schema = NULL;
  const bool ret = ekonValueParseFast(v, src, err, &schema);
  if (ret == false) {
    return 0;
  }

  if (EKON_UNLIKELY(v->n == 0))
    return 0;

  return ekonValueBeautify(v, err, unEscapeString, asJson);
}

// get str fast
const char *ekonValueGetStrFast(const struct EkonValue *v, uint32_t *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_STRING))
    return 0;
  *len = v->n->len;
  return v->n->value.str;
}

// get string from value
const char *ekonValueGetStr(struct EkonValue *v, uint8_t *option) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_STRING))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(v->n->value.str, v->n->len, str);
  str[v->n->len] = 0;
  *option = v->n->option;
  return str;
}

// get unescaped string
const char *ekonValueGetUnEspaceStr(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_STRING))
    return 0;

  char *retStr = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(retStr == 0))
    return 0;
  uint32_t finalLen;
  ekonUnEscapeStr(v->n->value.str, v->n->len, retStr, &finalLen);
  return retStr;
}

// get num fast
const char *ekonValueGetNumFast(const struct EkonValue *v, uint32_t *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  *len = v->n->len;
  return v->n->value.str;
}

// get num as str
const char *ekonValueGetNumStr(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(v->n->value.str, v->n->len, str);
  str[v->n->len] = 0;
  return str;
}

// get num as a double
const double *ekonValueGetNum(struct EkonValue *v) {
  return ekonValueGetDouble(v);
}

// get num as a double
const double *ekonValueGetDouble(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  double *d = (double *)ekonAllocatorAlloc(v->a, sizeof(double));
  if (EKON_UNLIKELY(d == 0))
    return 0;
  *d = ekonStrToDouble(v->n->value.str);
  return d;
}

// get num as an int
const int *ekonValueGetInt(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  int *i = (int *)ekonAllocatorAlloc(v->a, sizeof(int));
  if (EKON_UNLIKELY(i == 0))
    return 0;
  *i = ekonStrToInt(v->n->value.str);
  return i;
}

// get value as an long
const long *ekonValueGetLong(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  long *i = (long *)ekonAllocatorAlloc(v->a, sizeof(long));
  if (EKON_UNLIKELY(i == 0))
    return 0;
  *i = ekonStrToLong(v->n->value.str);
  return i;
}

// get value as an long
const long long *ekonValueGetLongLong(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  long long *i = (long long *)ekonAllocatorAlloc(v->a, sizeof(long long));
  if (EKON_UNLIKELY(i == 0))
    return 0;
  *i = ekonStrToLongLong(v->n->value.str);
  return i;
}

// get value as bool
const bool *ekonValueGetBool(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_BOOL))
    return 0;
  bool *val = (bool *)malloc(1);
  *val = false;
  if (*(v->n->value.str) == 't')
    *val = true;
  return val;
}

// check value as null
bool ekonValueIsNull(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NULL))
    return false;
  return true;
}

// get the key
const char *ekonValueGetKey(struct EkonValue *v, uint8_t *option,
                            uint32_t *keyLength) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  struct EkonNode *node = v->n;
  uint32_t len = node->keyLen;

  char *str = ekonAllocatorAlloc(v->a, len + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(node->key, len, str);
  *option = node->option;
  str[len] = 0;
  *keyLength = len;
  return str;
}

// get the key as unescaped
const char *ekonValueGetUnEspaceKey(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->keyLen + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  uint32_t finalLen;
  ekonUnEscapeStr(v->n->key, v->n->keyLen, str, &finalLen);
  return str;
}

// get the key fast
const char *ekonValueGetKeyFast(const struct EkonValue *v, uint32_t *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  *len = v->n->keyLen;
  return v->n->key;
}

// get the value of the object
struct EkonValue *ekonValueObjGet(const struct EkonValue *v, const char *key) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT))
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
struct EkonValue *ekonValueObjGetLen(const struct EkonValue *v, const char *key,
                                     uint32_t len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT))
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
const EkonType *ekonValueType(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  return &v->n->ekonType;
}

// Get the size of the EKON Value
uint32_t ekonValueSize(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT &&
                    v->n->ekonType != EKON_TYPE_ARRAY))
    return 0;
  return v->n->len;
}

// Get the value as array
struct EkonValue *ekonValueArrayGet(const struct EkonValue *v, uint32_t index) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_ARRAY))
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
struct EkonValue *ekonValueBegin(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT &&
                    v->n->ekonType != EKON_TYPE_ARRAY))
    return 0;

  if (EKON_UNLIKELY(v->n->value.node != 0)) {
    struct EkonValue *retVal = ekonValueInnerNew(v->a, v->n->value.node);
    return retVal;
  }
  return 0;
}

// get the next value
struct EkonValue *ekonValueNext(const struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_LIKELY(v->n->next != 0)) {
    struct EkonValue *retVal = ekonValueInnerNew(v->a, v->n->next);
    return retVal;
  }
  return 0;
}

// Copy value from one pointer to another
bool ekonValueCopyFrom(struct EkonValue *v, const struct EkonValue *vv) {
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
    desNode->ekonType = node->ekonType;
    if (node->key != 0) {
      char *k = ekonAllocatorAlloc(a, node->keyLen);
      if (EKON_UNLIKELY(k == 0))
        return false;
      ekonCopy(node->key, node->keyLen, k);
      desNode->key = k;
      desNode->keyLen = node->keyLen;
    } else
      desNode->key = 0;

    switch (node->ekonType) {
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
struct EkonValue *ekonValueCopy(const struct EkonValue *v) {
  struct EkonValue *retVal = ekonValueNew(v->a);
  if (EKON_UNLIKELY(ekonValueCopyFrom(retVal, v) == false))
    return 0;
  return retVal;
}

// move a value
bool ekonValueMove(struct EkonValue *v) {
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
bool ekonValueSetNull(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->ekonType = EKON_TYPE_NULL;
  v->n->value.str = ekonStrNull;
  v->n->len = 4;
  return true;
}

// set a value to bool
bool ekonValueSetBool(struct EkonValue *v, bool b) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->ekonType = EKON_TYPE_BOOL;
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
bool ekonValueSetNumStrFast(struct EkonValue *v, const char *num) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set a num str with length fast
bool ekonValueSetNumStrLenFast(struct EkonValue *v, const char *num,
                               uint32_t len) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set a num str
bool ekonValueSetNumStr(struct EkonValue *v, const char *num) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set a num str with length
bool ekonValueSetNumStrLen(struct EkonValue *v, const char *num, uint32_t len) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set num to a value
bool ekonValueSetNum(struct EkonValue *v, const double d) {
  return ekonValueSetDouble(v, d);
}

// set double to a value
bool ekonValueSetDouble(struct EkonValue *v, const double d) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set int to a value
bool ekonValueSetInt(struct EkonValue *v, const int n) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set long to a value
bool ekonValueSetLong(struct EkonValue *v, const long n) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set long long to a value
bool ekonValueSetLongLong(struct EkonValue *v, const long long n) {
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
  v->n->ekonType = EKON_TYPE_NUMBER;
  v->n->value.str = num;
  v->n->len = len;
  return true;
}

// set str escaped character
bool ekonValueSetStrEscape(struct EkonValue *v, const char *str,
                           uint8_t option) {
  uint32_t finalLen;
  const char *es = ekonEscapeStr(str, v->a, &finalLen);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetStrFast(v, es, option);
}

// set str escaped characters with len
bool ekonValueSetStrLenEscape(struct EkonValue *v, const char *str,
                              uint32_t len, uint8_t option) {
  const char *es = ekonEscapeStrLen(str, v->a, len);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetStrFast(v, es, option);
}

// set str fast
bool ekonValueSetStrFast(struct EkonValue *v, const char *str, uint8_t option) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckStr(str, &len) == false)) {
    return false;
  }
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->option = option;
  v->n->ekonType = EKON_TYPE_STRING;
  v->n->value.str = str;
  v->n->len = len;
  return true;
}

// set str with length fast
bool ekonValueSetStrLenFast(struct EkonValue *v, const char *str,
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
  v->n->ekonType = EKON_TYPE_STRING;
  v->n->value.str = str;
  v->n->len = len;
  return true;
}

// set value set str
bool ekonValueSetStr(struct EkonValue *v, const char *str) {
  uint32_t len = 0;
  if (EKON_UNLIKELY(ekonCheckStr(str, &len) == false))
    return false;
  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(str, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
  }
  v->n->ekonType = EKON_TYPE_STRING;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set value to str with length
bool ekonValueSetStrLen(struct EkonValue *v, const char *str, uint32_t len) {
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
  v->n->ekonType = EKON_TYPE_STRING;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set key with escape chars
bool ekonValueSetKeyEscape(struct EkonValue *v, const char *key, uint8_t option,
                           bool replace) {
  uint32_t finalLen;
  const char *es = ekonEscapeStr(key, v->a, &finalLen);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetKeyFast(v, es, option, replace);
}

// escape and length
bool ekonValueSetKeyLenEscape(struct EkonValue *v, const char *key,
                              uint32_t len, uint8_t option, bool replace) {
  const char *es = ekonEscapeStrLen(key, v->a, len);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetKeyFast(v, es, option, replace);
}

// uses the same previous memory
bool ekonValueSetKeyFast(struct EkonValue *v, const char *key, uint8_t option,
                         bool replace) {
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
    v->n->ekonType = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else {
    if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT))
      return false;
  }

  v->n->key = key;
  v->n->keyLen = len;
  v->n->option = option;
  return true;
}

// fastest key setter. also doesn't copy the str.
bool ekonValueSetKeyLenFast(struct EkonValue *v, const char *key, uint32_t len,
                            bool replace) {
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, key, len) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->ekonType = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->ekonType != EKON_TYPE_OBJECT))
    return false;
  v->n->key = key;
  v->n->keyLen = len;
  return true;
}

// copies a key string to add to the value node
bool ekonValueSetKey(struct EkonValue *v, const char *key, bool replace) {
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
    v->n->ekonType = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->ekonType != EKON_TYPE_OBJECT))
    return false;

  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(key, len, s);
  v->n->key = s;
  v->n->keyLen = len;
  return true;
}

// copies a key string with length to add to the value
bool ekonValueSetKeyLen(struct EkonValue *v, const char *key, uint32_t len,
                        bool replace) {
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, key, len) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->ekonType = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->ekonType != EKON_TYPE_OBJECT))
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
bool ekonValueSetArray(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->ekonType = EKON_TYPE_ARRAY;
  v->n->value.node = 0;
  v->n->len = 0;
  return true;
}

// Set Object
bool ekonValueSetObj(struct EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (struct EkonNode *)ekonAllocatorAlloc(v->a, sizeof(struct EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->ekonType = EKON_TYPE_OBJECT;
  v->n->value.node = 0;
  v->n->len = 0;
  return true;
}

// set a value fast
bool ekonValueSetFast(struct EkonValue *v, struct EkonValue *vv) {
  if (EKON_UNLIKELY(ekonValueMove(vv) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = vv->n;
    vv->n = 0;
    return true;
  }
  v->n->ekonType = vv->n->ekonType;
  if (v->n->key != 0 && vv->n->key != 0) {
    v->n->key = vv->n->key;
    v->n->keyLen = vv->n->keyLen;
  }
  v->n->value = vv->n->value;
  v->n->len = vv->n->len;
  if (v->n->ekonType == EKON_TYPE_ARRAY || v->n->ekonType == EKON_TYPE_OBJECT) {
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
bool ekonValueSet(struct EkonValue *v, const struct EkonValue *vv) {
  struct EkonValue *cp = ekonValueCopy(vv);
  if (EKON_UNLIKELY(cp == 0))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = cp->n;
    return true;
  }
  v->n->ekonType = cp->n->ekonType;
  if (v->n->key != 0 && vv->n->key != 0) {
    v->n->key = cp->n->key;
    v->n->keyLen = cp->n->keyLen;
  }
  v->n->value = cp->n->value;
  v->n->len = cp->n->len;
  if (v->n->ekonType == EKON_TYPE_ARRAY || v->n->ekonType == EKON_TYPE_OBJECT) {
    v->n->end = vv->n->end;
    struct EkonNode *next = v->n->value.node;
    while (EKON_LIKELY(next != 0)) {
      next->father = v->n;
      next = next->next;
    }
  }
  return true;
}

// add obj fast
bool ekonValueObjAddFast(struct EkonValue *v, struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT))
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
bool ekonValueObjAdd(struct EkonValue *v, const struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT))
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

// Copy arrays from v to vv
bool ekonValueArrayAddFast(struct EkonValue *v, struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_ARRAY))
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
bool ekonValueArrayAdd(struct EkonValue *v, const struct EkonValue *vv) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_ARRAY))
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
bool ekonValueArrayDel(struct EkonValue *v, uint32_t index) {
  struct EkonValue *dv = ekonValueArrayGet(v, index);
  if (EKON_UNLIKELY(dv == 0))
    return false;
  return ekonValueMove(dv);
}

// 函数说明详见《API》
bool ekonValueObjDel(struct EkonValue *v, const char *key) {
  struct EkonValue *dv = ekonValueObjGet(v, key);
  if (EKON_UNLIKELY(dv == 0))
    return false;
  return ekonValueMove(dv);
}
