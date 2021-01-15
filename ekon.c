/**
 * =============== ekon.c =================
 * The main source file to either link statically or dynamically
 *
 * LICENSE: MIT
 *
 * Contents:
 * 1. Includes and Macros - line: ~(10 -> 20)
 * 2. Hashmap API - line: ~(20 -> 30)
 * 3. Ekon Utils functions - line: ~(30 -> 40)
 * 4. Ekon API functions - line: ~(30 -> 40)
 * =========================================
 * */

// ----------------------------------------------------------
// INCLUDES & PLATFORM SPECIFIC MACROS
// ----------------------------------------------------------

// ---INCLUDES--
#include "ekon.h"
#include <stdio.h>  // import snprintf
#include <stdlib.h> // import atof, atoi, atol, atoll, malloc, free
#include <string.h> // import memcpy, strcmp

// ---- MACROS -----
#if defined(_MSC_VER)
// Workaround a bug in the MSVC runtime where it uses __cplusplus when not
//  defined.
#pragma warning(push, 0)
#pragma warning(disable : 4668)
#endif

#if (defined(_MSC_VER) && defined(__AVX__)) ||                                 \
    (!defined(_MSC_VER) && defined(__SSE4_2__))
#define HASHMAP_SSE42
#endif

#if defined(HASHMAP_SSE42)
#include <nmmintrin.h>
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#if defined(_MSC_VER)
#pragma warning(push)
// Stop MSVC complaining about not inlining functions.
#pragma warning(disable : 4710)
// Stop MSVC complaining about inlining functions!
#pragma warning(disable : 4711)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#if defined(_MSC_VER)
#define HASHMAP_USED
#elif defined(__GNUC__)
#define HASHMAP_USED __attribute__((used))
#else
#define HASHMAP_USED
#endif

// max chain length
#define HASHMAP_MAX_CHAIN_LENGTH (8)
#define u32 uint32_t
#define i8 int8_t

#ifndef EKON_EXPECT_MODE
#define EKON_EXPECT_MODE 1
#endif
// checkout https://stackoverflow.com/q/7346929/6697318 for info
#if EKON_EXPECT_MODE == 1
// gcc/clang modifier.
#define EKON_LIKELY(x) __builtin_expect(x, 1)
#define EKON_UNLIKELY(x) __builtin_expect(x, 0)
#else
// msvc & other compilers
#define EKON_LIKELY(x) x
#define EKON_UNLIKELY(x) x
#endif

// some casting MACROS
#define HASHMAP_CAST(type, x) ((type)x)
#define HASHMAP_PTR_CAST(type, x) ((type)x)

// --------------------------------------------------------
// 3. Hashmap implementation
// ----------------------------------------------------------

#if defined(__cplusplus)
extern "C" {
#endif

/// @brief Create a hashmap
/// @param initSize The initial size of the hashmap. Power of 2 otherwise
/// error
/// @param outHashmap The storage for the created hashmap
/// @return On success `true` is returned
static bool ekonHashmapInit(EkonAllocator *a, const u32 initSize,
                            EkonHashmap *const outHashMap);

/// @brief Put an element into the hashmap
/// @param hashmap - The hashmap to insert into
/// @param key - String key - not copied
/// @param len - The length of the string key
/// @param value - the value to insert
/// @param addr - the address where to store the address of the value
/// @return On success `true` is return
static bool ekonHashmapPut(EkonAllocator *a, EkonHashmap *const m,
                           const char *const key, const u32 len,
                           EkonNode *const value,
                           EkonHashmapItem **addr) HASHMAP_USED;

/// @brief Get an element from the hashmap
/// @param hashmap The hashmap to get from
/// @param key The string key to use
/// @param len The length of the string key
/// @return (EkonNode* | NULL - not found) The previously set element
static EkonNode *ekonHashmapGet(const EkonHashmap *const hashmap,
                                const char *const key,
                                const u32 len) HASHMAP_USED;

/// @brief Remove an element from the hashmap
/// @param m The hashmap to remove from
/// @param key The string key to use
/// @param len The length of the string key
/// @return On success `true` is returned
static bool ekonHashmapRemove(EkonHashmap *const hashmap, const char *const key,
                              const u32 len) HASHMAP_USED;

/// @brief Iterate over all the elements in a hashmap.
/// @param `hashmap` The hashmap to iterate over.
/// @param `f` The function pointer to call on each element.
///          'false' for error (stops iteration), 'true' for continue
///          iterating
/// @param context The context to pass as the first argument to f.
/// @return returns `true` if iteration is successful and `false` if failure
static bool ekonHashmapIterate(const EkonHashmap *const hashmap,
                               bool (*f)(void *const context,
                                         EkonNode *const value),
                               void *const context) HASHMAP_USED;

/// @brief Iterate over all the elements in a hashmap.
/// @param hashmap The hashmap to iterate over.
/// @param f The function pointer to call on each element.
///         '1' - continue iterating, '0' - exit early, '-1' - remove element
///// @param context The context to pass as the first argument to f.
/// @return If the entire hashmap was iterated then 0 is returned.
static bool ekonHashmapIteratePairs(EkonAllocator *a,
                                    EkonHashmap *const hashmap,
                                    i8 (*f)(EkonAllocator *, void *const,
                                            EkonHashmapItem *const),
                                    void *const context) HASHMAP_USED;

/// @brief Get the size of the hashmap
/// @param hashmap The hashmap to get the size of
/// @return The size of the hashmap
static u32 ekonHashmapNumEntries(const EkonHashmap *const hashmap) HASHMAP_USED;

static u32 ekonHashmapCrc32Helper(const char *const s,
                                  const u32 len) HASHMAP_USED;
static u32 ekonHashmapHashHelperIntHelper(const EkonHashmap *const m,
                                          const char *const keystring,
                                          const unsigned len) HASHMAP_USED;
static bool ekonHashmapMatchHelper(const EkonHashmapItem *const element,
                                   const char *const key,
                                   const u32 len) HASHMAP_USED;
static bool ekonHashmapHashHelper(const EkonHashmap *const m,
                                  const char *const key, const u32 len,
                                  u32 *const outIndex) HASHMAP_USED;
static i8 ekonHashmapRehashIterator(EkonAllocator *a, void *const newHash,
                                    EkonHashmapItem *const e) HASHMAP_USED;
static bool ekonHashmapRehashHelper(EkonAllocator *a,
                                    EkonHashmap *const m) HASHMAP_USED;
#if defined(__cplusplus)
}
#endif

bool ekonHashmapInit(EkonAllocator *a, const u32 initSize,
                     EkonHashmap *const outHashmap) {
  if (initSize == 0 || (initSize & (initSize - 1)) != 0)
    return false;
  outHashmap->data =
      HASHMAP_CAST(EkonHashmapItem *,
                   ekonAllocatorAlloc(a, initSize * sizeof(EkonHashmapItem)));
  if (!outHashmap->data) {
    return false;
  }
  outHashmap->tableSize = initSize;
  outHashmap->size = 0;
  return true;
}

bool ekonHashmapPut(EkonAllocator *a, EkonHashmap *const m,
                    const char *const key, const u32 len, EkonNode *const value,
                    EkonHashmapItem **addr) {
  u32 index;

  // Find a place to put our value
  while (ekonHashmapHashHelper(m, key, len, &index) == true) {
    if (ekonHashmapRehashHelper(a, m) == false)
      return false;
  }

  // Set the data
  m->data[index].value = value;
  m->data[index].key = key;
  m->data[index].keyLen = len;

  if (addr != NULL && *addr != NULL)
    *addr = m->data + index;

  // If the hashmap element was not already in use, set that is being
  //    used and bump size
  if (m->data[index].inUse == false) {
    m->data[index].inUse = true;
    m->size++;
  }
  return true;
}

EkonNode *ekonHashmapGet(const EkonHashmap *const m, const char *const key,
                         const u32 len) {
  u32 curr;
  u32 i;
  // Find the data structure
  curr = ekonHashmapHashHelperIntHelper(m, key, len);
  // linear probing, if necessary
  for (i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
    if (m->data[curr].inUse) {
      if (ekonHashmapMatchHelper(&m->data[curr], key, len)) {
        return m->data[curr].value;
      }
    }
    curr = (curr + 1) % m->tableSize;
  }
  return NULL; // not found
}

bool ekonHashmapRemove(EkonHashmap *const m, const char *const key,
                       const u32 len) {
  u32 i;
  u32 curr;

  // Find key
  curr = ekonHashmapHashHelperIntHelper(m, key, len);

  // linear probing if necessary
  for (i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
    if (m->data[curr].inUse) {
      if (ekonHashmapMatchHelper(&m->data[curr], key, len)) {
        // blank out the fields including `inUse`
        memset(&m->data[curr], 0, sizeof(EkonHashmapItem));
        m->size--;
        return true;
      }
    }
    curr = (curr + 1) % m->tableSize;
  }
  return false;
}

bool ekonHashmapIterate(const EkonHashmap *const m,
                        bool (*f)(void *const context, EkonNode *const value),
                        void *const context) {
  u32 i;
  // Linear probing
  for (i = 0; i < m->tableSize; i++) {
    if (m->data[i].inUse) {
      if (f(context, m->data[i].value) == false) {
        return false;
      }
    }
  }
  return true;
}

bool ekonHashmapIteratePairs(EkonAllocator *a, EkonHashmap *const hashmap,
                             i8 (*f)(EkonAllocator *, void *const,
                                     EkonHashmapItem *const),
                             void *const context) {
  u32 i;
  struct hashmap_element_s *p;
  i8 r;
  // Linear probing
  for (i = 0; i < hashmap->tableSize; i++) {
    p = &hashmap->data[i];
    if (p->inUse) {
      r = f(a, context, p);
      switch (r) {
      case -1: // remove item
        memset(p, 0, sizeof(EkonHashmapItem));
        hashmap->size--;
        break;
      case 1: // continue iterating
        break;
      default: // early exit
        return false;
      }
    }
  }
  return true;
}

u32 ekonHashmapNumEntries(const EkonHashmap *const m) { return m->size; }

u32 ekonHashmapCrc32Helper(const char *const s, const u32 len) {
  u32 i;
  u32 crc32val = 0;

#if defined(HASHMAP_SSE42)
  for (i = 0; i < len; i++) {
    crc32val = _mm_crc32_u8(crc32val, HASHMAP_CAST(u32 char, s[i]));
  }

  return crc32val;
#else
  // Using polynomial 0x11EDC6F41 to
  // match SSE 4.2's crc function.
  static const u32 crc32_tab[] = {
      0x00000000U, 0xF26B8303U, 0xE13B70F7U, 0x1350F3F4U, 0xC79A971FU,
      0x35F1141CU, 0x26A1E7E8U, 0xD4CA64EBU, 0x8AD958CFU, 0x78B2DBCCU,
      0x6BE22838U, 0x9989AB3BU, 0x4D43CFD0U, 0xBF284CD3U, 0xAC78BF27U,
      0x5E133C24U, 0x105EC76FU, 0xE235446CU, 0xF165B798U, 0x030E349BU,
      0xD7C45070U, 0x25AFD373U, 0x36FF2087U, 0xC494A384U, 0x9A879FA0U,
      0x68EC1CA3U, 0x7BBCEF57U, 0x89D76C54U, 0x5D1D08BFU, 0xAF768BBCU,
      0xBC267848U, 0x4E4DFB4BU, 0x20BD8EDEU, 0xD2D60DDDU, 0xC186FE29U,
      0x33ED7D2AU, 0xE72719C1U, 0x154C9AC2U, 0x061C6936U, 0xF477EA35U,
      0xAA64D611U, 0x580F5512U, 0x4B5FA6E6U, 0xB93425E5U, 0x6DFE410EU,
      0x9F95C20DU, 0x8CC531F9U, 0x7EAEB2FAU, 0x30E349B1U, 0xC288CAB2U,
      0xD1D83946U, 0x23B3BA45U, 0xF779DEAEU, 0x05125DADU, 0x1642AE59U,
      0xE4292D5AU, 0xBA3A117EU, 0x4851927DU, 0x5B016189U, 0xA96AE28AU,
      0x7DA08661U, 0x8FCB0562U, 0x9C9BF696U, 0x6EF07595U, 0x417B1DBCU,
      0xB3109EBFU, 0xA0406D4BU, 0x522BEE48U, 0x86E18AA3U, 0x748A09A0U,
      0x67DAFA54U, 0x95B17957U, 0xCBA24573U, 0x39C9C670U, 0x2A993584U,
      0xD8F2B687U, 0x0C38D26CU, 0xFE53516FU, 0xED03A29BU, 0x1F682198U,
      0x5125DAD3U, 0xA34E59D0U, 0xB01EAA24U, 0x42752927U, 0x96BF4DCCU,
      0x64D4CECFU, 0x77843D3BU, 0x85EFBE38U, 0xDBFC821CU, 0x2997011FU,
      0x3AC7F2EBU, 0xC8AC71E8U, 0x1C661503U, 0xEE0D9600U, 0xFD5D65F4U,
      0x0F36E6F7U, 0x61C69362U, 0x93AD1061U, 0x80FDE395U, 0x72966096U,
      0xA65C047DU, 0x5437877EU, 0x4767748AU, 0xB50CF789U, 0xEB1FCBADU,
      0x197448AEU, 0x0A24BB5AU, 0xF84F3859U, 0x2C855CB2U, 0xDEEEDFB1U,
      0xCDBE2C45U, 0x3FD5AF46U, 0x7198540DU, 0x83F3D70EU, 0x90A324FAU,
      0x62C8A7F9U, 0xB602C312U, 0x44694011U, 0x5739B3E5U, 0xA55230E6U,
      0xFB410CC2U, 0x092A8FC1U, 0x1A7A7C35U, 0xE811FF36U, 0x3CDB9BDDU,
      0xCEB018DEU, 0xDDE0EB2AU, 0x2F8B6829U, 0x82F63B78U, 0x709DB87BU,
      0x63CD4B8FU, 0x91A6C88CU, 0x456CAC67U, 0xB7072F64U, 0xA457DC90U,
      0x563C5F93U, 0x082F63B7U, 0xFA44E0B4U, 0xE9141340U, 0x1B7F9043U,
      0xCFB5F4A8U, 0x3DDE77ABU, 0x2E8E845FU, 0xDCE5075CU, 0x92A8FC17U,
      0x60C37F14U, 0x73938CE0U, 0x81F80FE3U, 0x55326B08U, 0xA759E80BU,
      0xB4091BFFU, 0x466298FCU, 0x1871A4D8U, 0xEA1A27DBU, 0xF94AD42FU,
      0x0B21572CU, 0xDFEB33C7U, 0x2D80B0C4U, 0x3ED04330U, 0xCCBBC033U,
      0xA24BB5A6U, 0x502036A5U, 0x4370C551U, 0xB11B4652U, 0x65D122B9U,
      0x97BAA1BAU, 0x84EA524EU, 0x7681D14DU, 0x2892ED69U, 0xDAF96E6AU,
      0xC9A99D9EU, 0x3BC21E9DU, 0xEF087A76U, 0x1D63F975U, 0x0E330A81U,
      0xFC588982U, 0xB21572C9U, 0x407EF1CAU, 0x532E023EU, 0xA145813DU,
      0x758FE5D6U, 0x87E466D5U, 0x94B49521U, 0x66DF1622U, 0x38CC2A06U,
      0xCAA7A905U, 0xD9F75AF1U, 0x2B9CD9F2U, 0xFF56BD19U, 0x0D3D3E1AU,
      0x1E6DCDEEU, 0xEC064EEDU, 0xC38D26C4U, 0x31E6A5C7U, 0x22B65633U,
      0xD0DDD530U, 0x0417B1DBU, 0xF67C32D8U, 0xE52CC12CU, 0x1747422FU,
      0x49547E0BU, 0xBB3FFD08U, 0xA86F0EFCU, 0x5A048DFFU, 0x8ECEE914U,
      0x7CA56A17U, 0x6FF599E3U, 0x9D9E1AE0U, 0xD3D3E1ABU, 0x21B862A8U,
      0x32E8915CU, 0xC083125FU, 0x144976B4U, 0xE622F5B7U, 0xF5720643U,
      0x07198540U, 0x590AB964U, 0xAB613A67U, 0xB831C993U, 0x4A5A4A90U,
      0x9E902E7BU, 0x6CFBAD78U, 0x7FAB5E8CU, 0x8DC0DD8FU, 0xE330A81AU,
      0x115B2B19U, 0x020BD8EDU, 0xF0605BEEU, 0x24AA3F05U, 0xD6C1BC06U,
      0xC5914FF2U, 0x37FACCF1U, 0x69E9F0D5U, 0x9B8273D6U, 0x88D28022U,
      0x7AB90321U, 0xAE7367CAU, 0x5C18E4C9U, 0x4F48173DU, 0xBD23943EU,
      0xF36E6F75U, 0x0105EC76U, 0x12551F82U, 0xE03E9C81U, 0x34F4F86AU,
      0xC69F7B69U, 0xD5CF889DU, 0x27A40B9EU, 0x79B737BAU, 0x8BDCB4B9U,
      0x988C474DU, 0x6AE7C44EU, 0xBE2DA0A5U, 0x4C4623A6U, 0x5F16D052U,
      0xAD7D5351U};

  for (i = 0; i < len; i++) {
    crc32val = crc32_tab[(HASHMAP_CAST(unsigned char, crc32val) ^
                          HASHMAP_CAST(unsigned char, s[i]))] ^
               (crc32val >> 8);
  }
  return crc32val;
#endif
}

u32 ekonHashmapHashHelperIntHelper(const EkonHashmap *const m,
                                   const char *const keyString, const u32 len) {
  u32 key = ekonHashmapCrc32Helper(keyString, len);

  // Robert Jenkins' 32 bit Mix Function
  key += (key << 12);
  key ^= (key >> 22);
  key += (key << 4);
  key ^= (key >> 9);
  key += (key << 10);
  key ^= (key >> 2);
  key += (key << 7);
  key ^= (key >> 12);

  /* Knuth's Multiplicative Method */
  key = (key >> 3) * 2654435761;

  return key % m->tableSize;
}

/**
 *
 * */
bool ekonHashmapMatchHelper(const struct hashmap_element_s *const element,
                            const char *const key, const unsigned len) {
  return (element->keyLen == len) && (memcmp(element->key, key, len) == 0);
}

/**
 * @brief Finds a place to put our hashitem
 * @param m         hashmap
 * @param key       takes this key, checks its presence and gives out an index
 * @param len       length of the key in question
 * @param outIndex  returns the index of the free space is stored
 * @return          success/failure
 * */
bool ekonHashmapHashHelper(const EkonHashmap *const m, const char *const key,
                           const u32 keyLen, u32 *const outIndex) {
  u32 curr;
  u32 i;
  // If full, return immediately
  if (m->size >= m->tableSize) {
    return true;
  }

  // Find the best index
  curr = ekonHashmapHashHelperIntHelper(m, key, keyLen);
  // Linear probing
  for (i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
    if (!m->data[curr].inUse) {
      *outIndex = curr;
      return false;
    }

    if (m->data[curr].inUse &&
        ekonHashmapMatchHelper(&m->data[curr], key, keyLen)) {
      *outIndex = curr;
      return false;
    }

    curr = (curr + 1) % m->tableSize;
  }
  return true;
}

// returns true if success
i8 ekonHashmapRehashIterator(EkonAllocator *a, void *const newHash,
                             EkonHashmapItem *const e) {
  EkonHashmapItem *node =
      (EkonHashmapItem *)ekonAllocatorAlloc(a, sizeof(EkonHashmapItem *));
  return ekonHashmapPut(a, HASHMAP_PTR_CAST(EkonHashmap *, newHash), e->key,
                        e->keyLen, e->value, &node);
}

/* Doubles the size of the hashmap, and
 * rehashes all the elements */
bool ekonHashmapRehashHelper(EkonAllocator *a, EkonHashmap *const m) {
  // If this multiplication overflows hashmap_create will fail.
  u32 newSize = 2 * m->tableSize;

  EkonHashmap newHash;

  bool flag = ekonHashmapInit(a, newSize, &newHash);
  if (flag == false) // failure
    return false;

  // copy the old elements to the new table
  flag = ekonHashmapIteratePairs(a, m, ekonHashmapRehashIterator,
                                 HASHMAP_PTR_CAST(void *, &newHash));
  if (flag == false) // could not complete iteration
    return flag;

  // put new hash into old hash structure by copying
  memcpy(m, &newHash, sizeof(EkonHashmap));

  return true;
}

// ----------------------------------------------------
//  EKON API IMPLEMENTATION
// ----------------------------------------------------

// Macros and functions wrt memory management
#ifndef EKON_MEMORY_NODE
#define EKON_MEMORY_NODE 1
#endif
#if EKON_MEMORY_NODE == 1
// @return an allocated string
void *ekonNew(u32 size) { return malloc(size); }
// @brief frees memory
void ekonFree(void *pointer) { free(pointer); }
#elif EKON_MEMORY_NODE == 2
static u32 ekonAllocMemorySize = 0, ekonAllocMemoryCount = 0,
           ekonFreeMemoryCount = 0;

// @return an allocated string but this time with side-effects
void *ekonNew(u32 size) {
  return ekonAllocMemorySize += size, ekonAllocMemoryCount += 1, malloc(size);
}
void ekonFree(void *ptr) { freeMemoryCount += 1, free(ptr); }
#endif

// store constants in char arrays
static const char *ekonStrTrue = "true";
static const char *ekonStrFalse = "false";
static const char *ekonStrNull = "null";

// -------- declarations -----------
bool ekonCheckNum(const char *s, u32 *outLen);
// ---------------------------------

// check if character is quote
bool ekonIsQuote(const char c) { return c == '\'' || c == '"'; }

// check if character is a bracket
bool ekonIsBracket(const char c) {
  switch (c) {
  case '{':
  case '}':
  case ']':
  case '[':
    return true;
  default:
    return false;
  }
}

/**
 * @brief check if character is among: <space> \t \n \r [ ] { } ' " , : \0
 */
bool ekonIsNonUnquotedStrChar(const char c) {
  switch (c) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':
  case '[':
  case ']':
  case '{':
  case '}':
  case '"':
  case '\'':
  case '\0':
  case ',':
  case ':':
    return true;
  default:
    return false;
  };
}

/**
 * @brief copy a string from src to des using a certain length
 * @param src source string
 * @param len length of the string - note: '\0' is insignificant
 * @param des destination string
 * */
void ekonCopy(const char *src, const u32 len, char *des) {
  memcpy(des, src, len);
}

/**
 * @brief copy schema string of length with '\0' at the end
 * @param src       pointer to the start of the schema string
 *                  rightfully follows `
 * @param len       length of the string
 * @return des      destination string (appened with '\0')
 * */
char *ekonCopySchema(const char *src, const u32 len) {
  char *schema = (char *)malloc(sizeof(char) * (len + 1));
  memcpy(schema, src, len);
  schema[len] = '\0';
  return schema;
}

// length of string
u32 ekonStrLen(const char *str) { return (u32)strlen(str); }

// convert option from
EkonOption ekonValueOptionStrToKey(EkonOption option) {
  if ((option & EKON_IS_STR_SPACED) != 0) {
    option &= (~EKON_IS_STR_SPACED);
    option |= EKON_IS_KEY_SPACED;
  }
  if ((option & EKON_IS_STR_MULTILINED) != 0) {
    option &= (~EKON_IS_STR_MULTILINED);
    option |= EKON_IS_STR_MULTILINED;
  }
  if ((option & EKON_IS_STR_ESCAPABLE) != 0) {
    option &= (~EKON_IS_STR_ESCAPABLE);
    option |= EKON_IS_KEY_ESCAPABLE;
  }
  return option;
}

// math.pow
float power(float b, int e) {
  float final = b;
  int e2 = e;
  if (e < 0)
    e2 = -e;
  for (int i = 0; i < e2; i++)
    final *= b;
  return e2 < 0 ? 1 / final : final;
}

int parseInt(const char *str, int len) {
  int final = 0;
  int counter = 0;

  int i;
  for (i = len - 1; i >= 0; i--) {
    if (str[i] >= '0' && str[i] <= '9') {
      final += power(10, counter) * (str[i] - '0');
      counter++;
    }
  }

  if (str[i + 1] == '-')
    final = -final;
  return final;
}

/* str->int convert, returns false for error, outInt stores converted val */
bool ekonStrToInt(const char *str, int *outInt) {
  u32 len = 0;
  if (ekonCheckNum(str, &len) == false)
    return false;
  *outInt = parseInt(str, len) == false;
  return true;
}

/* str->long convert, returns false for error, outInt stores converted val */
bool ekonStrToLong(const char *str, long *outLong) {
  u32 counter = 0;
  u32 i = 0;
  u32 len = 0;
  long ii = 0;

  if (ekonCheckNum(str, &len) == false)
    return false;

  for (i = len - 1; i >= 0; i--) {
    if ((str[i] >= '0' && str[i] <= '9')) {
      ii += power(10, counter) * (str[i] - '0');
      counter++;
    }
  }
  if (str[i + 1] == '-')
    *outLong = -ii;
  return true;
}

/* str->ll convert, returns false for error, outInt stores converted val */
bool ekonStrToLongLong(const char *str, long long *outLongLong) {
  u32 counter = 0;
  u32 i = 0;
  u32 len = 0;
  long long ii = 0;

  if (ekonCheckNum(str, &len) == false)
    return false;

  for (i = len - 1; i >= 0; i--) {
    if ((str[i] >= '0' && str[i] <= '9')) {
      ii += power(10, counter) * (str[i] - '0');
      counter++;
    }
  }
  if (str[i + 1] == '-')
    *outLongLong = -ii;
  return true;
}

/* str->double convert, returns false for error, outInt stores converted val */
bool ekonStrToDouble(const char *str, f64 *outDouble) {
  u32 len = 0;
  if (ekonCheckNum(str, &len) == false)
    return false;
  bool isNegative = 0;
  bool isExpNegative = 0;
  int intRange[2] = {-1, -1};
  int decRange[2] = {-1, -1};
  int expRange[2] = {-1, -1};
  int i = 0;

  if (str[i] == '-' || str[i] == '+') {
    if (str[i] == '-')
      isNegative = 1;
    i++;
  }

  intRange[0] = i;
  while (i <= len) {
    if ((str[i] >= '0' && str[i] <= '9') || str[i] == '_')
      i++;
    else {
      intRange[1] = i - 1;
      break;
    }
  }

  if (str[i] == '.') {
    i++;
    decRange[0] = i;
    while (i <= len) {
      if ((str[i] >= '0' && str[i] <= '9') || str[i] == '_')
        i++;
      else {
        decRange[1] = i - 1;
        break;
      }
    }
  }

  if (str[i] == 'e' || str[i] == 'E') {
    i++;
    if (str[i] == '-' || str[i] == '+') {
      if (str[i] == '-')
        isExpNegative = true;
      i++;
    }

    expRange[0] = i;
    while (i <= len) {
      if ((str[i] >= '0' && str[i] <= '9') || str[i] == '_')
        i++;
      else {
        expRange[1] = i - 1;
        break;
      }
    }
  }

  int integer = parseInt(str + intRange[0], intRange[1] - intRange[0] + 1);
  int decimal = parseInt(str + decRange[0], decRange[1] - decRange[0] + 1);
  int exp = parseInt(str + expRange[0], expRange[1] - expRange[0] + 1);

  int decimalRange = 0;
  for (int i = decRange[0]; i <= decRange[1]; i++) {
    if (str[i] >= '0' && str[i] <= '9')
      decimalRange--;
  }

  float final = (integer + power(10, decimalRange) * decimal) * power(10, exp);

  return isNegative ? -final : final;
}

/* int->str  */
u32 ekonIntToStr(int n, char *buff) { return snprintf(buff, 12, "%d", n); }

/* long->str  */
u32 ekonLongToStr(long n, char *buff) { return snprintf(buff, 24, "%ld", n); }

/* longLong->str  */
u32 ekonLongLongToStr(long long n, char *buff) {
  return snprintf(buff, 24, "%lld", n);
}

/* double->str  */
u32 ekonDoubleToStr(f64 n, char *buff) {
  return snprintf(buff, 32, "%.17g", n);
}

bool ekonParseError(char **outMessage, const char *s, const u32 index) {
  u32 pos = 1;
  u32 line = 1;
  u32 cursor = 0;
  while (s[cursor] != 0 && cursor != index) {
    if (s[cursor] == '\n') {
      pos = 0;
      line++;
    }
    pos++;
    cursor++;
  }
  pos--;

  // allocate memory for the message
  *outMessage = (char *)malloc(sizeof(char) * 50);

  // error messages will be of format: "<line>:<pos>:<character>" with `:` as
  if (s[index] == 0)
    snprintf(*outMessage, 50, "%d:%d:0", line, pos);
  else
    snprintf(*outMessage, 50, "%d:%d:%c", line, pos, s[index - 1]);

  return false;
}

bool ekonDuplicateKeyError(char **outMessage, const char *s, const u32 index,
                           const u32 keyLen) {
  u32 pos = 1;
  u32 line = 1;
  u32 cursor = 0;
  while (s[cursor] != 0 && cursor != index) {
    if (s[cursor] == '\n') {
      pos = 0;
      line++;
    }
    pos++;
    cursor++;
  }

  *outMessage = (char *)malloc(sizeof(char) * (100 + keyLen));
  char *key = (char *)malloc((sizeof(char) * (keyLen + 1)));
  ekonCopy(s + index, keyLen, key);
  key[index] = '\0';

  // <line>:<pos>:<key>:<message>
  snprintf(*outMessage, 100, "%d:%d:%s:Duplicate key", line, pos, key);
  return false;
}

bool ekonStrIsEqual(const char *a, const char *b, u32 len) {
  u32 i;
  for (i = 0; EKON_LIKELY(i < len); ++i) {
    if (EKON_LIKELY(a[i] != b[i]))
      return false;
  }
  if (EKON_LIKELY(a[i] == 0))
    return true;
  return false;
}

bool ekonStrIsEqualLen(const char *a, u32 a_len, const char *b, u32 b_len) {
  if (EKON_LIKELY(a_len != b_len))
    return false;
  u32 i;
  for (i = 0; EKON_LIKELY(i < a_len); ++i) {
    if (EKON_LIKELY(a[i] != b[i]))
      return false;
  }
  return true;
}

EkonAllocator *ekonAllocatorNew() {
  void *ptr = ekonNew(sizeof(EkonAllocator) + sizeof(EkonANode) +
                      ekonAllocatorInitMemSize);
  if (EKON_UNLIKELY(ptr == 0))
    return 0;
  EkonAllocator *alloc = (EkonAllocator *)ptr;
  alloc->root = (EkonANode *)((char *)ptr + sizeof(EkonAllocator));
  alloc->end = alloc->root;

  alloc->root->size = ekonAllocatorInitMemSize;
  alloc->root->data = (char *)ptr + sizeof(EkonAllocator) + sizeof(EkonANode);
  alloc->root->pos = 0;
  alloc->root->next = 0;
  return alloc;
}

void ekonAllocatorRelease(EkonAllocator *rootAlloc) {
  EkonANode *next = rootAlloc->root->next;
  while (EKON_LIKELY(next != 0)) {
    EkonANode *nn = next->next;
    ekonFree((void *)next);
    next = nn;
  }
  ekonFree((void *)rootAlloc);
}

/**
 * @brief Append a child to EkonAllocator's EkonANode
 * @param init_size size of the data to be appended
 * @param alloc     Allocator to append child to
 * @return          success
 * */
bool ekonAllocatorAppendChild(u32 init_size, EkonAllocator *alloc) {
  void *ptr = ekonNew(sizeof(EkonANode) + init_size);
  if (EKON_UNLIKELY(ptr == 0))
    return false;

  EkonANode *node = (EkonANode *)ptr;
  node->size = init_size;
  node->data = (char *)ptr + sizeof(EkonANode);
  node->pos = 0;
  node->next = 0;
  alloc->end->next = node;
  alloc->end = node;
  return true;
}

char *ekonAllocatorAlloc(EkonAllocator *alloc, u32 size) {
  EkonANode *currNode = alloc->end;
  u32 s = currNode->size;
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

// consume a comment. both rangin multiple lines and single lines
bool ekonConsumeComment(const char *s, u32 *index);

static EkonString *ekonStringCache = 0;

// get a new string object
EkonString *ekonStringNew(EkonAllocator *alloc, u32 initSize) {
  EkonString *str =
      (EkonString *)ekonAllocatorAlloc(alloc, sizeof(EkonString) + initSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  str->size = initSize;
  str->data = (char *)str + sizeof(EkonString);
  str->pos = 0;
  str->a = alloc;
  return str;
}

void ekonStringReset(EkonString *str) { str->pos = 0; }

// append char* to EkonString. returns false if error
bool ekonStringAppendStr(EkonString *str, const char *s, u32 size) {
  u32 srcS = str->size;
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

// append char to EkonString, returns false if error
bool ekonStringAppendChar(EkonString *str, const char c) {
  u32 srcS = str->size;
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

// append '\0' to EkonString. returns false if error
bool ekonStringAppendEnd(EkonString *str) {
  return ekonStringAppendChar(str, 0);
}

const char *ekonStringStr(EkonString *str) { return str->data; }

// ----------------- DEBUG FUNCTIONS ------------------------

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

void debPrintNodeMin(const EkonNode *node) {
#ifdef DEBUG_FUNC
  if (node != NULL || node != 0) {
    printf("{EkonType:");
    debPrintEkonType(node->ekonType);
    printf("}");
  } else
    printf("NULL");
#endif
}
void debPrintStrM(const char *s, u32 index, const char *mes, u32 size) {
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

void printOption(u16 option, char *delimiter) {
#define TORF(x) (((option & (x)) != 0) == true ? "true" : "false")
  printf("%sisKeySpaced:%s,", delimiter, TORF(EKON_IS_KEY_SPACED));
  printf("%sisKeyMultilined:%s,", delimiter, TORF(EKON_IS_KEY_MULTILINED));
  printf("%sisStrSpaced:%s,", delimiter, TORF(EKON_IS_STR_SPACED));
  printf("%sisStrMultilined:%s,", delimiter, TORF(EKON_IS_STR_MULTILINED));
#undef TORF
}

void debPrintStr(const char *s, u32 index, u32 size) {
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
void debPrintStrMin(const char *s, u32 size) {
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

void debPrintNodeSing(const struct _EkonNode *n, int depth) {
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
    printOption(n->option, (char *)"\t");
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

void debPrintNode(const EkonNode *n, int depth) {
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

bool debPrintHashMapIterator(void *const context, EkonNode *const node) {
  u32 size = node->keyLen;
  if (size == 0)
    size = strlen(node->key);
  char *des = (char *)malloc(size + 1);
  strncpy(des, node->key, size);
  des[size] = 0;
  printf("'%s':", des);
  free(des);
  debPrintNodeMin(node);
  printf(",");
  return true;
}

void debPrintHashmap(const EkonHashmap *const hashmap) {
#ifdef DEBUG_FUNC
  printf("{");
  ekonHashmapIterate(hashmap, &debPrintHashMapIterator, NULL);
  printf("}\n");
#endif
}
// -----------------------------------------

EkonValue *ekonValueNew(EkonAllocator *alloc) {
  EkonValue *v = (EkonValue *)ekonAllocatorAlloc(alloc, sizeof(EkonValue));
  if (EKON_UNLIKELY(v == 0))
    return 0;
  v->a = alloc;
  v->n = 0;
  return v;
}

// Creates a wrapper EkonValue for EkonNode. allocates to alloc
EkonValue *ekonValueInnerNew(EkonAllocator *alloc, EkonNode *n) {
  EkonValue *v = (EkonValue *)ekonAllocatorAlloc(alloc, sizeof(EkonValue));
  if (EKON_UNLIKELY(v == 0))
    return 0;
  v->a = alloc;
  v->n = n;
  return v;
}

/*
 * @brief check if a character is a whitespace ('\r', '\t', '\n', ' ')
 * @param c character to be checked
 * @return true/false
 * */
bool ekonSkin(const char c) {
  return EKON_UNLIKELY(
      EKON_UNLIKELY(c == ' ') ||
      EKON_UNLIKELY(c == '\t' ||
                    EKON_UNLIKELY(c == '\n' || EKON_UNLIKELY(c == '\r'))));
}

/**
 * @brief consume whitespace characters including comments
 * @param s         src string
 * @param index     pointer to index to be updated
 * @return          success/failure
 * */
bool ekonConsumeWhiteChars(const char *s, u32 *index) {
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

/**
 * @brief get s[index] and index++
 * @param s             EKON string source
 * @param index         pointer to the index. updates it
 * @return              character at the current index
 * */
char ekonPeek(const char *s, u32 *index) {
  if (ekonConsumeWhiteChars(s, index) == false)
    return 0;
  return s[(*index)++];
}

// consume current character. returns false if no match
bool ekonConsume(const char c, const char *s, u32 *index) {
  if (s[*index] == c) {
    ++(*index);
    return true;
  }
  return false;
}

// consume using likely the next character. returns false if no match
bool ekonLikelyConsume(const char c, const char *s, u32 *index) {
  if (EKON_LIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// consume using unlikely the next character.  returns false if no match
bool ekonUnlikelyConsume(const char c, const char *s, u32 *index) {
  if (EKON_UNLIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// peek and consume the next non-whitespace character using likely. false for no
// match
bool ekonLikelyPeekAndConsume(const char c, const char *s, u32 *index) {
  if (ekonConsumeWhiteChars(s, index) == false)
    return false;

  if (EKON_LIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// peek and consume the next non-whitespace character using unlikely. false for
// no match
bool ekonUnlikelyPeekAndConsume(const char c, const char *s, u32 *index) {
  if (ekonConsumeWhiteChars(s, index) == false)
    return false;

  if (EKON_UNLIKELY(s[*index] == c)) {
    ++(*index);
    return true;
  }
  return false;
}

// consume a comment. also multiline comments
bool ekonConsumeComment(const char *s, u32 *index) {
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

// consume 'false' string. return false if not 'false'
bool ekonConsumeFalse(const char *s, u32 *index) {
  if (EKON_LIKELY(*((u32 *)("alse")) == *((u32 *)(s + *index)))) {
    *index += 4;
    return true;
  }
  return false;
}

// consume 'true' string. return false if not 'true'
bool ekonConsumeTrue(const char *s, u32 *index) {
  if (EKON_LIKELY(*((u32 *)ekonStrTrue) == *((u32 *)(s + *index - 1)))) {
    *index += 3;
    return true;
  }
  return false;
}

/**
 * @brief consume `null` string
 * @param s     EKON string
 * @param index pointer to index to be updated
 * @return success/failure
 * */
bool ekonConsumeNull(const char *s, u32 *index) {
  if (EKON_LIKELY(*((u32 *)ekonStrNull) == *((u32 *)(s + *index - 1)))) {
    *index += 3;
    return true;
  }
  return false;
}

// get the decimal value
u32 ekonHexCodePoint(const char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return 16;
}

// check if the number is '1' or '0'
bool ekonCharIsBinary(const char c) { return c == '1' || c == '0'; }
// check if the number is between '0' and '9'
bool ekonCharIsDecimal(const char c) { return c >= '0' && c <= '9'; }
// check if the number is between '0' and '9' || 'a' and 'z' || 'A' and 'Z'
bool ekonCharIsHex(const char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z');
}
// check if the number is between '0' and '7'
bool ekonCharIsOctal(const char c) { return c >= 0 && c <= '7'; }

// TODO: do more inquiry on this: ekonValueGetUnEspaceStr
u32 ekonHexCodePointForUnEscape(const char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return c - 'a' + 10;
}

// consume Hex one char, consume, now also update the same.
bool ekonConsumeHexOne(const char *s, u32 *index, u32 *cp) {
  u32 tcp = ekonHexCodePoint(s[*index]);
  if (EKON_LIKELY(tcp < 16)) {
    *cp = *cp << 4;
    *cp += tcp;
    ++(*index);
    return true;
  }
  return false;
}

// valueGetUnEscapeStr, again ekonValueGetUnEspaceStr
void ekonConsumeHexOneForUnEscape(const char *s, u32 *index, u32 *cp) {
  *cp = *cp << 4;
  *cp += ekonHexCodePointForUnEscape(s[*index]);
  ++(*index);
  return;
}

// check if the next 4 characters represent a hex number using cp
// if yes, return true, otherwise false
bool ekonConsumeHex(const char *s, u32 *index, u32 *cp) {
  if (EKON_LIKELY(EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)) &&
                  EKON_LIKELY(ekonConsumeHexOne(s, index, cp)))) {
    return true;
  }
  return false;
}

// consume Hex for unescaped strings - TODO: research later
void ekonConsumeHexForUnEscape(const char *s, u32 *index, u32 *cp) {
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
  ekonConsumeHexOneForUnEscape(s, index, cp);
}

// apend char to a string. not memory safe
void ekonAppend(char *s, u32 *index, char c) { s[(*index)++] = c; }

// append char array with length to another string
void ekonAppendLen(char *s, u32 *index, const char *str, u32 len) {
  ekonCopy(str, len, s + (*index));
  *index += len;
}

// Append UTF-8
void ekonAppendUTF8(char *s, u32 *index, u32 codepoint) {
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

// Append a '\0' at the end of a character array
void ekonAppendEnd(char *s, u32 *index) { ekonAppend(s, index, 0); }

// unescapes a str and stores
void ekonUnEscapeStr(const char *str, u32 len, char *s, u32 *finalLen) {
  u32 sIndex = 0;
  u32 index;
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
        u32 cp = 0;
        ekonConsumeHexForUnEscape(str, &index, &cp);
        if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xDBFF)) {
          u32 cp1 = 0;
          index += 2;
          ekonConsumeHexForUnEscape(str, &index, &cp1);
          cp = (((cp - 0xD800) << 10) | (cp1 - 0xDC00)) + 0x10000;
        }
        ekonAppendUTF8(s, &sIndex, cp);
        break;
      }
      case 'U': {
        index += 2;
        u32 cp = 0;
        ekonConsumeHexForUnEscape(str, &index, &cp);
        ekonConsumeHexForUnEscape(str, &index, &cp);
        if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xDBFF)) {
          u32 cp1 = 0;
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

// just a type for the next array
struct EkonEscapeChar {
  const char *str;
  u32 len;
};

// a list of characters that the replacement is for
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

// escapes the cahracters for you.
const char *ekonEscapeStr(const char *str, EkonAllocator *a, u32 *finalLen) {
  u32 len = 0;
  const char *src = str;
  while (EKON_LIKELY(*str != 0)) {
    len += ekonEscapeChars[(unsigned char)(*str)].len;
    ++str;
  }
  char *s = ekonAllocatorAlloc(a, len + 1);
  if (EKON_UNLIKELY(s == 0))
    return 0;
  u32 index = 0;
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

// escapes the characters. handles `"` differently than the above function
const char *ekonEscapeStrJSON(const char *str, EkonAllocator *a,
                              u32 *finalLen) {
  u32 len = 0;
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
  u32 index = 0;
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
const char *ekonEscapeStrLen(const char *str, EkonAllocator *a, u32 len) {
  u32 l = 0;
  const char *src = str;
  u32 srcLen = len;
  while (EKON_LIKELY(len != 0)) {
    l += ekonEscapeChars[(unsigned char)(*str)].len;
    ++str;
    --len;
  }
  char *s = ekonAllocatorAlloc(a, l + 1);
  if (EKON_UNLIKELY(s == 0))
    return 0;
  u32 index = 0;
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
bool ekonConsumeStr(const char *s, u32 *index, const char quoteType,
                    EkonOption *option) {
  char c = s[*index];
  while (EKON_LIKELY(c != 0)) {
    if (c == '\n') {
      if (EKON_LIKELY(quoteType != '\'')) {
        return false;
      } else {
        *option |= (EKON_IS_STR_SPACED | EKON_IS_STR_MULTILINED);
        c = s[++(*index)];
        continue;
      }
    }

    if (c == '\r' || c == ' ' || c == '\t') {
      *option |= EKON_IS_STR_SPACED;
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
        u32 cp = 0;
        if (EKON_LIKELY(ekonConsumeHex(s, index, &cp))) {
          // ... something to do with handling UTF16 characters
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0xDFFF))
            return false;
          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, index) &&
                            ekonLikelyConsume('u', s, index))) {
              u32 cp2 = 0;
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
        u32 cp = 0;
        if (EKON_LIKELY(ekonConsumeHexOne(s, index, &cp)) &&
            EKON_LIKELY(ekonConsumeHexOne(s, index, &cp))) {
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0XDFFF))
            return false;

          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, index) &&
                            ekonLikelyConsume('x', s, index))) {
              u32 cp2 = 0;
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

    /* if ((*option & EKON_IS_STR_SPACED) == 0 && */
    /*     (c == '[' || c == ']' || c == ':' || c == '{' || c == '}' || */
    /*      c == ',')) { */
    /*   *option |= EKON_IS_STR_SPACED; */
    /* } */

    if (EKON_UNLIKELY(c == quoteType)) {
      (*index)++;
      return true;
    }

    c = s[++(*index)];
  }
  return false;
}

/**
 * @brief Consumes schema.
 * @param s         EKON string
 * @param index     pointer to the index to be updated
 * @return          success/failure
 * */
bool ekonConsumeSchema(const char *s, u32 *index) {
  char c = s[*index];
  while (EKON_LIKELY(c != 0)) {
    if (EKON_UNLIKELY(c == '`')) {
      return true;
    }
    c = s[(*index)++];
  }
  return false;
}

/**
 * @brief consume an unquoted string from EKON String
 * @param s         EKON string
 * @param index     pointer to the index that is to be updated
 * @return          success/failure
 **/
bool ekonConsumeUnquotedStr(const char *s, u32 *index) {
  char c = s[(*index)];
  while (EKON_LIKELY(c != 0)) {
    if (ekonIsNonUnquotedStrChar(c))
      return true;
    if (EKON_UNLIKELY((unsigned char)c <= 0x1f))
      return false;
    c = s[++(*index)];
  }
  return true;
}

/**
 * @brief check if a string is valid
 * @param       s           Ekon String
 * @param[out]  len         length that will be output
 * @param[out]  option      option for the string
 * */
bool ekonCheckStr(const char *s, u32 *len, EkonOption *option) {
  u32 index = 0;
  char c = s[index++];
  while (EKON_LIKELY(c != 0)) {
    if (c == '\n') {
      *option = (*option) | EKON_IS_STR_SPACED | EKON_IS_STR_MULTILINED;
      c = s[++index];
      continue;
    }

    if (c == '\r' || c == ' ' || c == '\t') {
      *option = *option | EKON_IS_STR_SPACED;
      c = s[++index];
      continue;
    }
    if (EKON_UNLIKELY(EKON_UNLIKELY((unsigned char)c <= 0x1f))) {
      return false;
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
        u32 cp = 0;
        if (EKON_LIKELY(ekonConsumeHex(s, &index, &cp))) {
          // UTF16 and UNICODE support ....
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0xDFFFF))
            return false;
          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, &index) &&
                            ekonLikelyConsume('u', s, &index))) {
              u32 cp2 = 0;
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
        u32 cp = 0;
        if (EKON_LIKELY(ekonConsumeHexOne(s, &index, &cp)) &&
            EKON_LIKELY(ekonConsumeHexOne(s, &index, &cp))) {
          if (EKON_UNLIKELY(cp >= 0xDC00 && cp <= 0xDFFFF))
            return false;
          if (EKON_UNLIKELY(cp >= 0xD800 && cp <= 0xD8FF)) {
            if (EKON_LIKELY(ekonLikelyConsume('\\', s, &index) &&
                            ekonLikelyConsume('x', s, &index))) {
              u32 cp2 = 0;
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
bool ekonCheckStrLen(EkonAllocator *alloc, const char *s, u32 len,
                     EkonOption *option) {
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
  u32 avail_len;
  if (EKON_UNLIKELY(ekonCheckStr(ekonStringStr(ekonStringCache), &avail_len,
                                 option) == false))
    return false;
  if (EKON_UNLIKELY(avail_len != len))
    return false;
  return true;
}

/**
 * @brief check if a string is a number
 * @param s The start of the string you want to check
 * @param outLen Updates the length of the string
 * @return -1 means last letter is 0, 1 means success, 0 means failure
 * */
i8 ekonCheckNumWithoutEnd(const char *s, u32 *outLen, u16 *option) {
  u32 index = 0;

  if (s[index] == '-' || s[index] == '+')
    ++(index);

  // consume hex, binary, octal numbers
  if (ekonUnlikelyConsume('0', s, &index)) {
    // only 0
    if (ekonIsNonUnquotedStrChar(s[index])) {
      *option |= EKON_IS_NUM_INT;
      *outLen = index;
      if (s[index] == '\0')
        return -1;
      return 1;
    }

    // consume hex
    if (ekonLikelyConsume('x', s, &index) ||
        ekonLikelyConsume('X', s, &index)) {
      if (ekonCharIsHex(s[index++])) {
        while (ekonCharIsHex(s[index]) || s[index] == '_') {
          index++;
        }
        if (s[index - 1] == '_')
          return false; // return false if the last digit is '_'
        if (ekonIsNonUnquotedStrChar(s[index])) {
          *outLen = index;
          if (s[index] == '\0')
            return -1;
          *option |= EKON_IS_NUM_HEXADECIMAL;
          return 1;
        }
      }
      return 0;
    }

    // consume binary
    if (ekonLikelyConsume('b', s, &index)) {
      if (ekonCharIsBinary(s[index++])) {
        while (ekonCharIsBinary(s[index]) || s[index] == '_') {
          index++;
        }
        if (s[index - 1] == '_')
          return false; // return false if the last digit is '_'
        if (ekonIsNonUnquotedStrChar(s[index])) {
          *outLen = index;
          if (s[index] == '\0')
            return -1;
          *option |= EKON_IS_NUM_BINARY;
          return 1;
        }
      }
      return 0;
    }

    // consume octal numbers
    if (ekonLikelyConsume('o', s, &index)) {
      if (ekonCharIsOctal(s[index++])) {
        while (ekonCharIsOctal(s[index]) || s[index] == '_') {
          index++;
        }
        if (s[index - 1] == '_')
          return false; // return false if the last digit is '_'
        if (ekonIsNonUnquotedStrChar(s[index])) {
          *outLen = index;
          if (s[index] == '\0')
            return -1;
          *option |= EKON_IS_NUM_OCTAL;
          return 0;
        }
      }
      return 0;
    }

    if (s[index] != '.' || s[index] != 'e' || s[index] != 'E') {
      return false;
    }
  } else if (EKON_LIKELY(ekonCharIsDecimal(s[index]))) {
    char c = s[++index];
    while (EKON_LIKELY(ekonCharIsDecimal(c)) || c == '_')
      c = s[++index];
    if (s[index - 1] == '_') // last digit cannot be _
      return false;
    *option |= EKON_IS_NUM_INT;
  } else {
    return false;
  }

  if (ekonConsume('.', s, &index)) {
    char c = s[index];
    if (EKON_LIKELY(ekonCharIsDecimal(c))) {
      c = s[++index];
      while (EKON_LIKELY(ekonCharIsDecimal(c)) || c == '_')
        c = s[++index];
      if (s[index - 1] == '_')
        return false;
      *option &= ~EKON_IS_NUM_INT; // remove if int is there
      *option |= EKON_IS_NUM_FLOAT;
    } else
      return false;
  }

  if (s[index] == 'e' || s[index] == 'E') {
    char c = s[++index];
    if (c == '-' || c == '+')
      c = s[++index];

    if (EKON_LIKELY(ekonCharIsDecimal(c))) {
      c = s[++index];
      while (EKON_LIKELY(ekonCharIsDecimal(c)) || c == '_')
        c = s[++index];
      if (s[index - 1] == '_')
        return false;
    } else
      return false;
  }

  *outLen = index;
  return s[index] == '\0' ? -1 : 1;
}

// check a string is a number. note: string should end with '\0'
bool ekonCheckNum(const char *s, u32 *outLen) {
  u16 option = 0;
  i8 res = ekonCheckNumWithoutEnd(s, outLen, &option);
  if (res == 0 || res == 1) {
    return false;
  }
  return true;
}

// consume a number - hex,
bool ekonConsumeNum(const char *s, u32 *index, u16 *option) {
  u32 len = 0;
  if (ekonCheckNumWithoutEnd(s + *index, &len, option) != 0) {
    *index += len;
    return true;
  }
  return false;
}

// check for the length of a number
bool ekonCheckNumLen(EkonAllocator *alloc, const char *s, u32 len) {
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

  u32 avail_len;
  if (EKON_UNLIKELY(ekonCheckNum(ekonStringStr(ekonStringCache), &avail_len) ==
                    false))
    return false;

  if (EKON_UNLIKELY(avail_len != len))
    return false;
  return true;
}

/*
 * @brief fill a node as EKON_TYPE_STRING node
 * @param node      node to be filled in
 * @param s         char string to be inserted into the node
 * @param len       length of the string to be added
 * @param option    property of the string that is to be added
 */
void ekonNodeAddStr(EkonNode *node, const char *s, u32 len, u16 option) {
  node->option = option;
  node->value.str = s;
  node->len = len;
  node->ekonType = EKON_TYPE_STRING;
  if (node->father && node->father->ekonType == EKON_TYPE_OBJECT)
    (node->hashItem->value) = node;
}

/*
 * @brief fill a node as EKON_TYPE_NULL node
 */
void ekonNodeAddNull(EkonNode *node) {
  node->ekonType = EKON_TYPE_NULL;
  node->value.str = ekonStrNull;
  node->len = 4;
  if (node->father && node->father->ekonType == EKON_TYPE_OBJECT)
    node->hashItem->value = node;
}
/*
 * @brief fill a node as EKON_TYPE_NULL node
 */
void ekonNodeAddBoolean(EkonNode *node, bool val) {
  node->ekonType = EKON_TYPE_BOOL;
  if (val == true) {
    node->value.str = ekonStrTrue;
    node->len = 4;
  } else {
    node->value.str = ekonStrFalse;
    node->len = 5;
  }
  if (node->father && node->father->ekonType == EKON_TYPE_OBJECT)
    node->hashItem->value = node;
}

void ekonNodeAddNumber(EkonNode *node, const char *s, u32 len, u16 option) {
  node->value.str = s;
  node->len = len;
  node->option = option;
  node->ekonType = EKON_TYPE_NUMBER;
  if (node->father && node->father->ekonType == EKON_TYPE_OBJECT)
    node->hashItem->value = node;
}

/**
 * @brief throws in an error for srcNode
 * @param srcNode EkonNode
 * @param v EkonValue
 * @param s source string
 * @param errMessage errMessage to write the error to
 * @param index index of where the error occurred
 * */
bool ekonSrcNodeError(EkonNode *srcNode, EkonValue *v, const char *s,
                      char **errMessage, u32 index) {
  if (EKON_LIKELY(srcNode == 0))
    v->n = srcNode;
  else
    *v->n = *srcNode;
  return ekonParseError(errMessage, s, index);
}

typedef enum { EKON_OPT_IS_OBJ = 1, EKON_OPT_IS_ROOT_OBJ = 2 } EkonNodeOpt;

/**
 * @brief Create an EkonNode and add it to node.
 *          if empty array/object, node will be modified but not replaced
 *          node will be replaced with `node->value.node` if array/obj not empty
 * @param outNode       pointer where the new node will be stored
 * @param v             main value of the node, the allocator is relevant
 * @param srcNode       I still don't know what srcNode is. TODO
 * @param s             the original string of EKON text
 * @param index         index pointer to current cursor for s buffer
 * @param errMessage    pointer for the errMessage to be stored
 * @param addObjOpt     (EKON_OPT_IS_OBJ | EKON_OPT_IS_ROOT_OBJ)
 * @return              sucess/failure
 * */
bool ekonNodeAddObjOrArrNode(EkonNode **outNode, EkonValue *v,
                             EkonNode *srcNode, const char *s, u32 *index,
                             char **errMessage, const EkonNodeOpt addObjOpt) {
  const bool isObj = (addObjOpt & EKON_OPT_IS_OBJ) != 0;
  const bool isRootObj = (addObjOpt & EKON_OPT_IS_ROOT_OBJ);

  if (isObj == false) {
    (*outNode)->ekonType = EKON_TYPE_ARRAY;
    if (ekonUnlikelyPeekAndConsume(']', s, index)) {
      (*outNode)->value.node = 0;
      (*outNode)->len = 0;
      return true;
    }
  } else {
    (*outNode)->ekonType = EKON_TYPE_OBJECT;
    if (isRootObj == false && ekonUnlikelyPeekAndConsume('}', s, index)) {
      (*outNode)->value.node = 0;
      (*outNode)->len = 0;
      return true;
    }
  }

  if ((*outNode)->father && (*outNode)->father->ekonType == EKON_TYPE_OBJECT) {
    (*outNode)->hashItem->value = *outNode;
  }

  if (isObj == true) {
    EkonHashmap *map =
        (EkonHashmap *)ekonAllocatorAlloc(v->a, sizeof(EkonHashmap));
    if (ekonHashmapInit(v->a, 16, map) == false)
      return false;
    (*outNode)->keymap = map;
  }

  EkonNode *n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));

  if (EKON_UNLIKELY(n == 0))
    return ekonSrcNodeError(srcNode, v, s, errMessage, *index);

  n->father = *outNode;
  n->prev = 0;

  (*outNode)->value.node = n;
  (*outNode)->end = n;
  (*outNode)->len = 1;
  *outNode = n;
  return true;
}

/**
 * @brief add a key to the node
 * @param a             EkonAllocator where the memory allocation happens
 * @param node          EkonNode where the key is to be added
 * @param s             Ekon string which is to be parsed
 * @param index         index to be updated
 * @param option        key options (EKON_NODE_OPTIONS)
 * @param errMessage    errMessage where the error message is to be stored
 * @return              success/failure
 * */
bool ekonNodeAddKey(EkonAllocator *a, EkonNode *node, const char *s, u32 *index,
                    u16 *option, char **errMessage) {
  EkonHashmap *keymap = node->father->keymap;
  bool isKeyUnquoted = ekonIsQuote(s[(*index)]) == false;

  if (isKeyUnquoted) {
    u32 start = *index;
    if (ekonConsumeUnquotedStr(s, index)) {
      u32 keyLen = (*index) - start;
      const char *key = s + start;
      if (ekonHashmapGet(keymap, key, keyLen) != NULL) {
        return ekonDuplicateKeyError(errMessage, s, start, keyLen);
      } else {
        node->key = key;
        node->keyLen = keyLen;
        node->option = *option;
        EkonHashmapItem **item =
            (EkonHashmapItem **)(malloc(sizeof(EkonHashmapItem *)));
        ekonHashmapPut(a, keymap, key, keyLen, NULL, item);
        node->hashItem = *item;
      }
    } else {
      return ekonParseError(errMessage, s, *index);
    }
  } else {
    char quoteType = s[*index];
    if (quoteType == '"')
      *option |= EKON_IS_KEY_ESCAPABLE;

    u32 start = (++(*index));
    const char *key = s + start;

    if (EKON_UNLIKELY(ekonUnlikelyConsume(quoteType, s, index))) {
      if (ekonHashmapGet(keymap, s + start, 0) != 0) {
        ekonDuplicateKeyError(errMessage, s, *index, 0);
        return false;
      } else {
        *option |= EKON_IS_KEY_SPACED;
        node->key = key;
        node->keyLen = 0;
        node->option = *option;
        EkonHashmapItem **item =
            (EkonHashmapItem **)(malloc(sizeof(EkonHashmapItem *)));
        ekonHashmapPut(a, keymap, key, 0, NULL, item);
        node->hashItem = *item;
      }
    } else {
      if (EKON_UNLIKELY(ekonConsumeStr(s, index, quoteType, option) == false)) {
        return ekonParseError(errMessage, s, *index);
      }

      u32 keyLen = *index - start - 1;
      if (ekonHashmapGet(keymap, key, keyLen) != NULL) {
        ekonDuplicateKeyError(errMessage, key, *index, keyLen);
        return false;
      } else {
        if (*option & EKON_IS_STR_SPACED)
          *option |= EKON_IS_KEY_SPACED;
        if (*option & EKON_IS_STR_MULTILINED)
          *option |= EKON_IS_KEY_MULTILINED;
        *option &= ~EKON_IS_STR_MULTILINED;
        *option &= ~EKON_IS_STR_SPACED;
        node->key = key;
        node->keyLen = keyLen;
        node->option = *option;
        EkonHashmapItem **item =
            (EkonHashmapItem **)(malloc(sizeof(EkonHashmapItem *)));
        ekonHashmapPut(a, keymap, key, keyLen, NULL, item);
        node->hashItem = *item;
      }
    }
  }
  return true;
}

bool ekonValueParseFast(EkonValue *v, const char *s, char **errMessage,
                        char **schema) {
  if (EKON_UNLIKELY(s[0] == '\0')) {
    ekonParseError(errMessage, s, 0);
    return false;
  }

  EkonNode *srcNode;

  if (EKON_LIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
    if (EKON_UNLIKELY(v->n == 0)) {
      ekonParseError(errMessage, s, 0);
      return false;
    }
    v->n->prev = 0;
    v->n->next = 0;
    v->n->father = 0;
    v->n->key = 0;
    srcNode = 0;
  } else {
    srcNode = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
    if (EKON_UNLIKELY(srcNode == 0)) {
      ekonParseError(errMessage, s, 0);
      return false;
    }
    *srcNode = *v->n;
  }

  u32 index = 0;
  EkonNode *node = v->n;
  bool isRootNoCurlyBrace = false;
  char c = ekonPeek(s, &index);

  if (c == '`') {
    const u32 start = index;
    if (ekonConsumeSchema(s, &index) == false)
      return ekonParseError(errMessage, s, index);

    if (*schema == NULL)
      *schema = ekonCopySchema(s + start, index - start - 1);

    c = ekonPeek(s, &index);
  }

  const u32 ifRootStart = index - 1;

  switch (c) {
  case '[': {
    if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage,
                                (const EkonNodeOpt)0))
      break;
    return false;
  }
  case '{': {
    if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage,
                                EKON_OPT_IS_OBJ))
      break;
    return false;
  }
  case 'n': {
    u32 start = index - 1;
    if (EKON_LIKELY(ekonConsumeNull(s, &index))) {
      if (ekonIsNonUnquotedStrChar(s[index])) {
        ekonNodeAddNull(node);
        break;
      }
      if (ekonConsumeUnquotedStr(s, &index)) {
        u32 strEnd = index;
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
      u32 strEnd = index;
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
    u32 start = index - 1;
    if (EKON_LIKELY(ekonConsumeFalse(s, &index))) {
      if (ekonIsNonUnquotedStrChar(s[index])) {
        ekonNodeAddBoolean(node, false);
        break;
      }
      if (ekonConsumeUnquotedStr(s, &index)) {
        u32 strEnd = index;
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
      u32 strEnd = index;
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
    u32 start = index - 1;
    if (EKON_LIKELY(ekonConsumeTrue(s, &index))) {
      if (ekonIsNonUnquotedStrChar(s[index]) == true) {
        ekonNodeAddBoolean(node, true);
        break;
      }
      if (ekonConsumeUnquotedStr(s, &index) == true) {
        u32 strEnd = index;
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
      u32 strEnd = index;
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
    u32 start = index;
    if (EKON_UNLIKELY(ekonUnlikelyConsume(c, s, &index))) {
      ekonNodeAddStr(node, s + index, 0, EKON_IS_STR_SPACED);
      break;
    }

    u16 option = 0;
    if (c == '"')
      option |= EKON_IS_STR_ESCAPABLE;

    if (EKON_LIKELY(ekonConsumeStr(s, &index, c, &option))) {
      u32 strEnd = index - 1;
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
    u32 start = index;
    if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
      u16 option = 0;
      if (ekonConsumeNum(s, &index, &option)) {
        u32 numEnd = index;
        if (ekonUnlikelyPeekAndConsume(':', s, &index)) {
          isRootNoCurlyBrace = true;
          index = ifRootStart;
          break;
        }
        ekonNodeAddNumber(node, s + start, numEnd - start, option);
        break;
      } else {
        index = start;
        if (ekonConsumeUnquotedStr(s, &index)) {
          u32 numEnd = index;
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
      u32 strEnd = index;
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
    if (ekonNodeAddObjOrArrNode(
            &node, v, srcNode, s, &index, errMessage,
            (const EkonNodeOpt)(EKON_OPT_IS_OBJ | EKON_OPT_IS_ROOT_OBJ)) == 0) {
      return false;
    }
  }

  while (EKON_LIKELY(node != v->n)) {
    u16 option = 0;
    if (node->father->ekonType == EKON_TYPE_OBJECT) {
      if (ekonNodeAddKey(v->a, node, s, &index, &option, errMessage) == 0)
        return false;

      if (EKON_UNLIKELY(ekonLikelyPeekAndConsume(':', s, &index) == false))
        return ekonSrcNodeError(srcNode, v, s, errMessage, index);
    } else {
      node->key = 0;
    }

    c = ekonPeek(s, &index);
    switch (c) {
    case '[': {
      EkonNode *currNode = node;
      if (!ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage,
                                   (const EkonNodeOpt)0)) {
        return false;
      }

      if (currNode == node)
        break;

      if (ekonPeek(s, &index) == ',')
        return ekonParseError(errMessage, s, index);

      index--;
      continue;
    }
    case '{': {
      EkonNode *currNode = node;
      if (ekonNodeAddObjOrArrNode(&node, v, srcNode, s, &index, errMessage,
                                  EKON_OPT_IS_OBJ) == false) {
        return false;
      }

      if (currNode == node)
        break;

      char nextChar = ekonPeek(s, &index);
      if (nextChar == ':' || nextChar == ',')
        return ekonParseError(errMessage, s, index);

      index--;
      continue;
    }
    case 'n': {
      u32 start = index - 1;
      if (EKON_LIKELY(ekonConsumeNull(s, &index)) == true) {
        if (ekonIsNonUnquotedStrChar(s[index]) == true) {
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
      u32 start = index - 1;
      if (EKON_LIKELY(ekonConsumeFalse(s, &index))) {
        if (ekonIsNonUnquotedStrChar(s[index]) == true) {
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
      u32 start = index - 1;
      if (EKON_LIKELY(ekonConsumeTrue(s, &index))) {
        if (ekonIsNonUnquotedStrChar(s[index]) == true) {
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
      u32 start = index;
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
        return ekonParseError(errMessage, s, index);

      index--;
      u32 start = index;
      u16 option = 0;
      if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
        if (ekonConsumeNum(s, &index, &option)) {
          ekonNodeAddNumber(node, s + start, index - start, option);
          break;
        } else {
          index = start;
          if (ekonConsumeUnquotedStr(s, &index)) {
            u32 numEnd = index;
            option &= ~EKON_IS_STR_SPACED;
            option &= ~EKON_IS_STR_MULTILINED;
            ekonNodeAddStr(node, s + start, numEnd - start, option);
            break;
          }
        }
      }

      if (ekonConsumeUnquotedStr(s, &index)) {
        u32 strEnd = index;
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
        ekonParseError(errMessage, s, index);
        return false;
      }

      if (c == 0) {
        if (isRootNoCurlyBrace) {
          node->next = 0;
          return true;
        } else {
          ekonParseError(errMessage, s, index);
          return false;
        }
      }

      if (c == ':') {
        ekonParseError(errMessage, s, index);
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
        EkonNode *n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));

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
bool ekonValueParseLen(EkonValue *v, const char *s, u32 len, char **err,
                       char **schema) {
  char *str = ekonAllocatorAlloc(v->a, len + 1);
  if (EKON_UNLIKELY(str == 0))
    return false;
  ekonCopy(s, len, str);
  str[len] = 0;
  return ekonValueParseFast(v, s, err, schema);
}

// The main parser - API
bool ekonValueParse(EkonValue *v, const char *s, char **err, char **schema) {
  return ekonValueParseLen(v, s, ekonStrLen(s), err, schema);
}

// -------------- util functions for stringifying ----------------
// append Quotes
const bool ekonAppendQuote(const EkonNode *node, EkonString *str) {
  if (EKON_UNLIKELY(ekonStringAppendChar(str, '\'') == false))
    return false;
  return true;
}
bool ekonUtilAppendArray(EkonString *str, EkonNode **node) {
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

bool ekonUtilAppendObj(EkonString *str, EkonNode **node, bool isRootObj) {
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

bool ekonUtilAppendStr(EkonString *str, EkonNode *node, const EkonValue *v,
                       bool unEscapeString, const bool isJSON) {
  if ((node->option & EKON_IS_STR_SPACED) != 0 ||
      (node->option & EKON_IS_STR_MULTILINED) != 0) {
    if (APPEND_QUOTE(str, isJSON) == false)
      return false;
  }

  u32 finalLen = node->len;
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

    u32 finalLen2;
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
bool ekonUtilAppendKey(EkonString *str, EkonNode *node, const EkonValue *v,
                       bool unEscapeString, const bool isJSON) {
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
const char *ekonValueStringify(const EkonValue *v, bool unEscapeString) {
  if (EKON_UNLIKELY(v->n == 0))
    return "";

  EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  EkonNode *node = v->n;

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
      const EkonNode *currentNode = node;
      if (ekonUtilAppendArray(str, &node) == false)
        return 0;
      if (currentNode == node)
        break;
      continue;
    }
    case EKON_TYPE_OBJECT: {
      const EkonNode *currentNode = node;
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
const char *ekonValueStringifyToJSON(const EkonValue *v, bool unEscapeString) {
  if (EKON_UNLIKELY(v->n == 0))
    return "";

  EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  EkonNode *node = v->n;

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

// append a tab to a string
bool ekonStringAppendTab(EkonString *str, const char *s, u32 depth) {
  for (int i = 0; i < depth; i++) {
    if (EKON_UNLIKELY(ekonStringAppendStr(str, s, strlen(s))) == false)
      return false;
  }
  return true;
}

const char *ekonValueBeautify(EkonValue *v, char **err, bool unEscapeString,
                              bool asJSON) {
  EkonString *str = ekonStringNew(v->a, ekonStringInitMemSize);
  if (EKON_UNLIKELY(str == 0))
    return 0;

  EkonNode *node = v->n;
  u32 depth = 0;

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
      const EkonNode *currentNode = node;
      if (ekonUtilAppendArray(str, &node) == false)
        return 0;
      if (ekonStringAppendStr(str, "\n", 1) == false)
        return 0;
      if (currentNode == node)
        break;
      continue;
    }
    case EKON_TYPE_OBJECT: {
      depth++;
      const EkonNode *currentNode = node;
      if (ekonUtilAppendObj(str, &node, false) == false)
        return 0;
      if (ekonStringAppendStr(str, "\n", 1) == false)
        return 0;
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
  EkonAllocator *a = ekonAllocatorNew();
  EkonValue *v = ekonValueNew(a);

  char *schema = NULL;
  const bool ret = ekonValueParseFast(v, src, err, &schema);
  if (ret == false) {
    return 0;
  }

  if (EKON_UNLIKELY(v->n == 0))
    return 0;

  return ekonValueBeautify(v, err, unEscapeString, asJson);
}

const char *ekonValueGetStrFast(const EkonValue *v, u32 *outLen) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_STRING))
    return 0;
  *outLen = v->n->len;
  return v->n->value.str;
}

const char *ekonValueGetStr(EkonValue *v, EkonOption *outOption) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_STRING))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(v->n->value.str, v->n->len, str);
  str[v->n->len] = 0;
  *outOption = v->n->option;
  return str;
}

const char *ekonValueGetUnEspaceStr(EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_STRING))
    return 0;

  char *retStr = ekonAllocatorAlloc(v->a, v->n->len + 1);
  if (EKON_UNLIKELY(retStr == 0))
    return 0;
  u32 finalLen;
  ekonUnEscapeStr(v->n->value.str, v->n->len, retStr, &finalLen);
  return retStr;
}

const char *ekonValueGetNumFast(const EkonValue *v, u32 *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  *len = v->n->len;
  return v->n->value.str;
}

const char *ekonValueGetNumStr(EkonValue *v) {
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

const bool ekonValueGetDouble(EkonValue *v, f64 *d) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  d = (f64 *)ekonAllocatorAlloc(v->a, sizeof(f64));
  if (EKON_UNLIKELY(d == 0))
    return 0;
  if (ekonStrToDouble(v->n->value.str, d) == false)
    return false;
  return true;
}

// get num as a double
const bool ekonValueGetNum(EkonValue *v, f64 *d) {
  return ekonValueGetDouble(v, d);
}

const bool ekonValueGetInt(EkonValue *v, int *i) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  i = (int *)ekonAllocatorAlloc(v->a, sizeof(int));
  if (EKON_UNLIKELY(i == 0))
    return 0;
  if (ekonStrToInt(v->n->value.str, i) == false)
    return false;
  return true;
}

const bool ekonValueGetLong(EkonValue *v, long *l) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  l = (long *)ekonAllocatorAlloc(v->a, sizeof(long));
  if (EKON_UNLIKELY(l == 0))
    return 0;
  if (ekonStrToLong(v->n->value.str, l) == false)
    return false;
  return true;
}

const bool ekonValueGetLongLong(EkonValue *v, long long *ll) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NUMBER))
    return 0;
  ll = (long long *)ekonAllocatorAlloc(v->a, sizeof(long long));
  if (EKON_UNLIKELY(ll == 0))
    return 0;
  if (ekonStrToLongLong(v->n->value.str, ll) == false)
    return false;
  return true;
}

const bool ekonValueGetBool(const EkonValue *v, bool *outBool) {
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

bool ekonValueIsNull(const EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_NULL))
    return false;
  return true;
}

const char *ekonValueGetKey(EkonValue *v, uint8_t *outOption, u32 *outKeyLen) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  EkonNode *node = v->n;
  u32 len = node->keyLen;

  char *str = ekonAllocatorAlloc(v->a, len + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  ekonCopy(node->key, len, str);
  *outOption = node->option;
  str[len] = 0;
  *outKeyLen = len;
  return str;
}

const char *ekonValueGetUnEspacedKey(EkonValue *v, u32 *outLen) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  char *str = ekonAllocatorAlloc(v->a, v->n->keyLen + 1);
  if (EKON_UNLIKELY(str == 0))
    return 0;
  u32 finalLen;
  ekonUnEscapeStr(v->n->key, v->n->keyLen, str, &finalLen);
  *outLen = finalLen;
  return str;
}

// get the key fast
const char *ekonValueGetKeyFast(const EkonValue *v, u32 *len) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->key == 0))
    return 0;
  *len = v->n->keyLen;
  return v->n->key;
}

EkonValue *ekonValueObjGet(const EkonValue *v, const char *key) {
  if (EKON_UNLIKELY(v->n == 0))
    return NULL;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT))
    return NULL;
  EkonNode *val = ekonHashmapGet(v->n->keymap, key, strlen(key));
  if (val == NULL)
    return NULL;
  return ekonValueInnerNew(v->a, val);
}

EkonValue *ekonValueObjGetLen(const EkonValue *v, const char *key, u32 keyLen) {
  if (EKON_UNLIKELY(v->n == 0))
    return NULL;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT))
    return NULL;
  EkonNode *val = ekonHashmapGet(v->n->keymap, key, keyLen);
  if (val == NULL)
    return NULL;
  return ekonValueInnerNew(v->a, val);
}

const EkonType ekonValueType(const EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return (EkonType)0;
  return v->n->ekonType;
}

u32 ekonValueSize(const EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT &&
                    v->n->ekonType != EKON_TYPE_ARRAY))
    return 0;
  return v->n->len;
}

EkonValue *ekonValueArrayGet(const EkonValue *v, u32 index) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_ARRAY))
    return 0;
  EkonNode *next = v->n->value.node;
  u32 i = 0;
  while (EKON_LIKELY(next != 0)) {
    if (EKON_UNLIKELY(i == index)) {
      EkonValue *retVal = ekonValueInnerNew(v->a, next);
      return retVal;
    }
    next = next->next;
    ++i;
  }
  return 0;
}

EkonValue *ekonValueBegin(const EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_UNLIKELY(v->n->ekonType != EKON_TYPE_OBJECT &&
                    v->n->ekonType != EKON_TYPE_ARRAY))
    return 0;

  if (EKON_UNLIKELY(v->n->value.node != 0)) {
    EkonValue *retVal = ekonValueInnerNew(v->a, v->n->value.node);
    return retVal;
  }
  return 0;
}

EkonValue *ekonValueNext(const EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return 0;
  if (EKON_LIKELY(v->n->next != 0)) {
    EkonValue *retVal = ekonValueInnerNew(v->a, v->n->next);
    return retVal;
  }
  return 0;
}

bool ekonValueCopyFrom(EkonValue *desV, const EkonValue *srcV) {
  if (EKON_UNLIKELY(srcV->n == 0))
    return false;
  EkonAllocator *const a = desV->a;
  desV->n = (EkonNode *)ekonAllocatorAlloc(a, sizeof(EkonNode));
  if (EKON_UNLIKELY(desV->n == 0))
    return false;
  desV->n->prev = 0;
  desV->n->next = 0;
  desV->n->father = 0;

  EkonNode *node = srcV->n;
  EkonNode *desNode = desV->n;

  do {
    desNode->ekonType = node->ekonType;

    // ----- key copy -----
    if (node->key != 0) {
      char *k = ekonAllocatorAlloc(a, node->keyLen);
      if (EKON_UNLIKELY(k == 0))
        return false;
      ekonCopy(node->key, node->keyLen, k);
      desNode->key = k;
      desNode->keyLen = node->keyLen;
    } else
      desNode->key = 0;
    // -------------------

    // yeah. we have also put up with this shit!
    EkonNode *father = desNode->father;
    desNode->hashItem = NULL;
    if (father != 0 && father->ekonType == EKON_TYPE_OBJECT) {
      desNode->hashItem =
          (EkonHashmapItem *)ekonAllocatorAlloc(a, sizeof(EkonHashmapItem));
      ekonHashmapPut(a, father->keymap, desNode->key, desNode->keyLen, desNode,
                     &desNode->hashItem);
    }

    switch (node->ekonType) {
    case EKON_TYPE_OBJECT: {
      if (ekonHashmapInit(a, node->keymap->size, desNode->keymap) == false)
        return false;
      // moving on to the case EKON_TYPE_ARRAY ...
    }
    case EKON_TYPE_ARRAY: {
      desNode->len = node->len;
      if (EKON_LIKELY(node->value.node != 0)) {
        node = node->value.node;
        EkonNode *n = (EkonNode *)ekonAllocatorAlloc(a, sizeof(EkonNode));
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
    while (EKON_LIKELY(node != srcV->n)) {
      if (EKON_LIKELY(node->next != 0)) {
        node = node->next;
        EkonNode *n = (EkonNode *)ekonAllocatorAlloc(a, sizeof(EkonNode));
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
  } while (EKON_UNLIKELY(node != srcV->n));

  return true;
}

EkonValue *ekonValueCopy(EkonAllocator *a, const EkonValue *srcV) {
  EkonValue *retVal = ekonValueNew(a);
  if (EKON_UNLIKELY(ekonValueCopyFrom(retVal, srcV) == false))
    return 0;
  return retVal;
}

bool ekonValueMoveOutOfArrObj(EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0))
    return false;

  if (v->n->father != 0) {
    EkonNode *n = v->n;
    EkonNode *father = v->n->father;
    if (n->prev == 0) {
      father->value.node = n->next;
    } else {
      n->prev->next = n->next;
      n->prev = 0;
    }

    if (n->next == 0) {
      n->father->end = n->prev;
    } else {
      n->next->prev = n->prev;
      n->next = 0;
    }

    if (father->ekonType == EKON_TYPE_OBJECT) {
      if (ekonHashmapRemove(father->keymap, n->key, n->keyLen) == false)
        return false;

      n->hashItem = NULL;
    }

    --(father->len);
    n->father = 0;
  }
  return true;
}

bool ekonValueSetNull(EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetBool(EkonValue *v, bool b) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetNumStrFast(EkonValue *v, const char *num) {
  u32 len = 0;
  if (EKON_UNLIKELY(ekonCheckNum(num, &len) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetNumStrLenFast(EkonValue *v, const char *num, u32 len) {
  if (EKON_UNLIKELY(ekonCheckNumLen(v->a, num, len) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetNumStr(EkonValue *v, const char *num) {
  u32 len = 0;
  if (EKON_UNLIKELY(ekonCheckNum(num, &len) == false))
    return false;
  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(num, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetNumStrLen(EkonValue *v, const char *num, u32 len) {
  if (EKON_UNLIKELY(ekonCheckNumLen(v->a, num, len) == false))
    return false;
  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(num, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetDouble(EkonValue *v, const f64 d) {
  char *num = ekonAllocatorAlloc(v->a, 32);
  if (EKON_UNLIKELY(num == 0))
    return false;
  u32 len = ekonDoubleToStr(d, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetNum(EkonValue *v, const f64 d) {
  return ekonValueSetDouble(v, d);
}

bool ekonValueSetInt(EkonValue *v, const int n) {
  char *num = ekonAllocatorAlloc(v->a, 16);
  if (EKON_UNLIKELY(num == 0))
    return false;
  u32 len = ekonIntToStr(n, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetLong(EkonValue *v, const long l) {
  char *num = ekonAllocatorAlloc(v->a, 24);
  if (EKON_UNLIKELY(num == 0))
    return false;
  u32 len = ekonLongToStr(l, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetLongLong(EkonValue *v, const long long ll) {
  char *num = ekonAllocatorAlloc(v->a, 24);
  if (EKON_UNLIKELY(num == 0))
    return false;
  u32 len = ekonLongLongToStr(ll, num);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetStrEscape(EkonValue *v, const char *str) {
  u32 finalLen;
  const char *es = ekonEscapeStr(str, v->a, &finalLen);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetStrFast(v, es);
}

bool ekonValueSetStrLenEscape(EkonValue *v, const char *str, u32 len) {
  const char *es = ekonEscapeStrLen(str, v->a, len);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetStrFast(v, es);
}

bool ekonValueSetStrFast(EkonValue *v, const char *str) {
  u32 len = 0;
  u16 option = 0;
  if (EKON_UNLIKELY(ekonCheckStr(str, &len, &option) == false)) {
    return false;
  }
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetStrLenFast(EkonValue *v, const char *str, u32 len) {
  EkonOption option;
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, str, len, &option) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetStr(EkonValue *v, const char *str) {
  u32 len = 0;
  EkonOption option;
  if (EKON_UNLIKELY(ekonCheckStr(str, &len, &option) == false))
    return false;
  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(str, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
  }
  v->n->option = option;
  v->n->ekonType = EKON_TYPE_STRING;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

bool ekonValueSetStrLen(EkonValue *v, const char *str, u32 len) {
  EkonOption option;
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, str, len, &option) == false))
    return false;

  char *s = ekonAllocatorAlloc(v->a, len);
  if (EKON_UNLIKELY(s == 0))
    return false;
  ekonCopy(str, len, s);
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  v->n->option = option;
  v->n->ekonType = EKON_TYPE_STRING;
  v->n->value.str = s;
  v->n->len = len;
  return true;
}

// set key with escape chars
bool ekonValueSetKeyEscape(EkonValue *v, const char *key) {
  u32 finalLen;
  const char *es = ekonEscapeStr(key, v->a, &finalLen);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetKeyFast(v, es);
}

bool ekonValueSetKeyLenEscape(EkonValue *v, const char *key, u32 len) {
  const char *es = ekonEscapeStrLen(key, v->a, len);
  if (EKON_UNLIKELY(es == 0))
    return false;
  return ekonValueSetKeyFast(v, es);
}

bool ekonValueSetKeyFast(EkonValue *v, const char *key) {
  u32 len = 0;
  EkonOption option;
  if (EKON_UNLIKELY(ekonCheckStr(key, &len, &option) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->ekonType = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else {
    EkonNode *father = v->n->father;
    if (father != 0 && father->ekonType == EKON_TYPE_OBJECT) {
      if (ekonHashmapPut(v->a, v->n->keymap, key, len, NULL, NULL) == false)
        return false;
    }
  }

  EkonNode *n = v->n;

  n->hashItem->key = key;
  n->hashItem->keyLen = len;
  n->key = key;
  n->keyLen = len;
  n->option = option;
  return true;
}

bool ekonValueSetKeyLenFast(EkonValue *v, const char *key, u32 len) {
  EkonOption option;
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, key, len, &option) == false))
    return false;
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
    v->n->ekonType = EKON_TYPE_NULL;
    v->n->value.str = ekonStrNull;
    v->n->len = 4;
  } else if (v->n->father != 0 &&
             EKON_UNLIKELY(v->n->father->ekonType != EKON_TYPE_OBJECT)) {
    EkonNode *father = v->n->father;
    if (father != 0 && father->ekonType == EKON_TYPE_OBJECT) {
      if (ekonHashmapPut(v->a, v->n->keymap, key, len, NULL, NULL) == false)
        return false;
    }
  }

  EkonNode *n = v->n;

  n->hashItem->key = key;
  n->hashItem->keyLen = len;
  n->key = key;
  n->keyLen = len;
  n->option = option;
  return true;
}

bool ekonValueSetKey(EkonValue *v, const char *key) {
  u32 len = 0;
  EkonOption option;
  if (EKON_UNLIKELY(ekonCheckStr(key, &len, &option) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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
  v->n->option = option;
  v->n->key = s;
  v->n->keyLen = len;
  return true;
}

// copies a key string with length to add to the value
bool ekonValueSetKeyLen(EkonValue *v, const char *key, u32 len) {
  EkonOption option;
  if (EKON_UNLIKELY(ekonCheckStrLen(v->a, key, len, &option) == false))
    return false;

  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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
  v->n->option = option;
  v->n->key = s;
  v->n->keyLen = len;
  return true;
}

bool ekonValueSetArray(EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
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

bool ekonValueSetObj(EkonValue *v) {
  if (EKON_UNLIKELY(v->n == 0)) {
    v->n = (EkonNode *)ekonAllocatorAlloc(v->a, sizeof(EkonNode));
    if (EKON_UNLIKELY(v->n == 0))
      return false;
    v->n->key = 0;
    v->n->prev = 0;
    v->n->father = 0;
    v->n->next = 0;
  }
  if (ekonHashmapInit(v->a, 8, v->n->keymap) == false)
    return false;
  v->n->ekonType = EKON_TYPE_OBJECT;
  v->n->value.node = 0;
  v->n->len = 0;
  return true;
}

bool ekonValueSetFast(EkonValue *desV, EkonValue *srcV) {
  if (EKON_UNLIKELY(ekonValueMoveOutOfArrObj(srcV) == false))
    return false;

  if (EKON_UNLIKELY(desV->n == 0)) {
    desV->n = srcV->n;
    srcV->n = 0;
    return true;
  }

  EkonNode *desN = desV->n;
  EkonNode *srcN = srcV->n;

  desN->ekonType = srcN->ekonType;
  if (desN->key != 0 && srcN->key != 0) {
    desN->key = srcN->key;
    desN->keyLen = srcN->keyLen;
  }

  desN->value = srcN->value;
  desN->len = srcN->len;

  if (srcN->father != 0 && desN->father != 0 &&
      srcN->father->ekonType == EKON_TYPE_OBJECT &&
      desN->father->ekonType == EKON_TYPE_OBJECT) {
    srcN->hashItem = desN->hashItem;
  }

  if (desN->ekonType == EKON_TYPE_ARRAY || desN->ekonType == EKON_TYPE_OBJECT) {
    if (desN->keymap != 0 && srcN->keymap != 0)
      desN->keymap = srcN->keymap;

    desN->end = srcN->end;
    EkonNode *next = desN->value.node;
    while (EKON_LIKELY(next != 0)) {
      next->father = desV->n;
      next = next->next;
    }
  }
  srcV->n = 0;
  return true;
}

bool ekonValueSet(EkonValue *desV, const EkonValue *srcV) {
  EkonValue *cp = ekonValueCopy(srcV->a, srcV);
  if (EKON_UNLIKELY(cp == 0))
    return false;
  if (EKON_UNLIKELY(desV->n == 0)) {
    desV->n = cp->n;
    return true;
  }
  desV->n->ekonType = cp->n->ekonType;
  if (desV->n->key != 0 && srcV->n->key != 0) {
    desV->n->key = cp->n->key;
    desV->n->keyLen = cp->n->keyLen;
  }
  desV->n->value = cp->n->value;
  desV->n->len = cp->n->len;
  if (desV->n->ekonType == EKON_TYPE_ARRAY ||
      desV->n->ekonType == EKON_TYPE_OBJECT) {
    desV->n->end = srcV->n->end;
    EkonNode *next = desV->n->value.node;
    while (EKON_LIKELY(next != 0)) {
      next->father = desV->n;
      next = next->next;
    }
  }
  return true;
}

bool ekonValueObjAddFast(EkonValue *objV, EkonValue *childV) {
  if (EKON_UNLIKELY(objV->n == 0))
    return false;
  if (EKON_UNLIKELY(objV->n->ekonType != EKON_TYPE_OBJECT))
    return false;
  if (EKON_UNLIKELY(childV->n == 0))
    return false;
  if (EKON_UNLIKELY(childV->n->key == 0))
    return false;
  if (EKON_UNLIKELY(ekonValueMoveOutOfArrObj(childV) == false))
    return false;

  childV->n->father = objV->n;
  if (ekonHashmapPut(objV->a, objV->n->keymap, childV->n->key,
                     childV->n->keyLen, childV->n,
                     &childV->n->hashItem) == false) {
    return false;
  }

  if (EKON_UNLIKELY(objV->n->value.node == 0)) {
    objV->n->value.node = childV->n;
    objV->n->len = 1;
    objV->n->end = childV->n;
  } else {
    objV->n->end->next = childV->n;
    childV->n->prev = objV->n->end;
    objV->n->end = childV->n;
    ++objV->n->len;
  }
  childV->n = 0;
  return true;
}

bool ekonValueObjAdd(EkonValue *objV, const EkonValue *childV) {
  if (EKON_UNLIKELY(objV->n == 0))
    return false;
  if (EKON_UNLIKELY(objV->n->ekonType != EKON_TYPE_OBJECT))
    return false;
  if (EKON_UNLIKELY(childV->n == 0))
    return false;
  if (EKON_UNLIKELY(childV->n->key == 0))
    return false;
  EkonValue *cp = ekonValueCopy(childV->a, childV);
  if (EKON_UNLIKELY(cp == 0))
    return false;
  cp->n->father = objV->n;

  if (ekonHashmapPut(objV->a, objV->n->keymap, childV->n->key,
                     childV->n->keyLen, childV->n,
                     &childV->n->hashItem) == false) {
    return false;
  }
  if (EKON_UNLIKELY(objV->n->value.node == 0)) {
    objV->n->value.node = cp->n;
    objV->n->len = 1;
    objV->n->end = cp->n;
  } else {
    objV->n->end->next = cp->n;
    cp->n->prev = objV->n->end;
    objV->n->end = cp->n;
    ++objV->n->len;
  }
  return true;
}

bool ekonValueArrayAddFast(EkonValue *arrV, EkonValue *childV) {
  if (EKON_UNLIKELY(childV->n == 0))
    return false;
  if (EKON_UNLIKELY(childV->n->ekonType != EKON_TYPE_ARRAY))
    return false;
  if (EKON_UNLIKELY(ekonValueMoveOutOfArrObj(childV) == false))
    return false;
  childV->n->key = 0;
  childV->n->father = arrV->n;
  if (EKON_UNLIKELY(arrV->n->value.node == 0)) {
    arrV->n->value.node = childV->n;
    arrV->n->len = 1;
    arrV->n->end = childV->n;
  } else {
    arrV->n->end->next = childV->n;
    childV->n->prev = arrV->n->end;
    arrV->n->end = childV->n;
    ++arrV->n->len;
  }
  childV->n = 0;
  return true;
}

bool ekonValueArrayAdd(EkonValue *arrV, const EkonValue *childV) {
  if (EKON_UNLIKELY(arrV->n == 0))
    return false;
  if (EKON_UNLIKELY(arrV->n->ekonType != EKON_TYPE_ARRAY))
    return false;
  EkonValue *cp = ekonValueCopy(childV->a, childV);
  if (EKON_UNLIKELY(cp == 0))
    return false;
  cp->n->key = 0;
  cp->n->father = arrV->n;
  if (EKON_UNLIKELY(arrV->n->value.node == 0)) {
    arrV->n->value.node = cp->n;
    arrV->n->len = 1;
    arrV->n->end = cp->n;
  } else {
    arrV->n->end->next = cp->n;
    cp->n->prev = arrV->n->end;
    arrV->n->end = cp->n;
    ++arrV->n->len;
  }
  return true;
}

bool ekonValueArrayDel(EkonValue *arrV, u32 index) {
  EkonValue *dv = ekonValueArrayGet(arrV, index);
  if (EKON_UNLIKELY(dv == 0))
    return false;
  return ekonValueMoveOutOfArrObj(dv);
}

bool ekonValueObjDel(EkonValue *v, const char *key) {
  EkonValue *dv = ekonValueObjGet(v, key);
  if (EKON_UNLIKELY(dv == 0))
    return false;
  return ekonValueMoveOutOfArrObj(dv);
}

/**
 * @brief set key and value for an object
 * @param objVal the object EkonValue where newNode is a field
 * @param newNode newNode will already be allocated and filled
 *                ekonType, option, key, keyLen, keymap, value, len
 * */
bool ekonValueSetKeyValue(EkonValue *objVal, EkonNode *newNode) {
  EkonNode *oldNode =
      ekonHashmapGet(objVal->n->keymap, newNode->key, newNode->keyLen);
  if (oldNode == NULL) {
    // if the node doesn't exist, pui it at the end
    EkonNode *objN = objVal->n;

    newNode->prev = objN->end;
    newNode->next = NULL;
    objN->end->next = newNode;
    objN->end = newNode;
  } else {
    // if the node already exists, maintain the position of the node
    newNode->prev = oldNode->prev;
    newNode->next = oldNode->next;
    newNode->end = oldNode->end;
    newNode->father = oldNode->father;
  }

  // replace anyways
  newNode->hashItem = (EkonHashmapItem *)ekonAllocatorAlloc(
      objVal->a, sizeof(EkonHashmapItem *));
  if (ekonHashmapPut(objVal->a, objVal->n->keymap, newNode->key,
                     newNode->keyLen, newNode, &newNode->hashItem) == false)
    return false;

  return true;
}

/**
 * @brief provide `key`, `keyLen` and get objNode
 * @param objNode the object node whose field is the outNode
 * @param key key whose value to get. NOTE. key[0] is the start of the char
 * array
 * @param keyLen u32
 * @param outValue the pointer to (pointer to the outValue where the value is
 * stored)
 * @return `true` for success/found and `false` for failure/not-found
 * */
bool ekonValueGetValue(EkonValue *objNode, const char *key, const u32 keyLen,
                       EkonNode **outValue) {
  *outValue = ekonHashmapGet(objNode->n->keymap, key, keyLen);
  if (*outValue == NULL)
    return false;
  return true;
}
