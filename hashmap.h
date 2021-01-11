// copied from https://github.com/sheredom/hashmap.h/blob/master/hashmap.h
// simple hashmap implementation
#ifndef HASHMAP_H
#define HASHMAP_H

// ----------------------------------------------------------
// INCLUDES & PLATFORM SPECIFIC MACROS
// ----------------------------------------------------------
#if defined(_MSC_VER)
// Workaround a bug in the MSVC runtime where it uses __cplusplus when not
// defined.
#pragma warning(push, 0)
#pragma warning(disable : 4668)
#endif
// ---INCLUDES--
#include "common.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

// ----------- Random macros -------------
// max chain length
#define HASHMAP_MAX_CHAIN_LENGTH (8)
#define u32 uint32_t
#define i8 int8_t

/// ---------------- Definition from ekon.h ----------------
char *ekonAllocatorAlloc(EkonAllocator *a, u32 size);
// --------------------------------------------------------

// ----------------------------------------------------------

// ----------------------------------------------------------
// Public API
// ----------------------------------------------------------

/* // HashMapItem */
/* struct hashmap_element_s { */
/*   const char *key; */
/*   u32 keyLen; */
/*   bool inUse; */
/*   EkonNode *value; */
/* }; */
/* #define EkonHashmapItem struct hashmap_element_s */

/* // HashMap */
/* struct hashmap_s { */
/*   u32 tableSize; // max size */
/*   u32 size;      // current size */
/*   EkonHashmapItem *data; */
/* }; */
/* #define EkonHashmap struct hashmap_s */

#if defined(__cplusplus)
extern "C" {
#endif

/// @brief Create a hashmap
/// @param initSize The initial size of the hashmap. Power of 2 otherwise error
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
///          'false' for error (stops iteration), 'true' for continue iterating
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

// ---------------------------------------------------------------------------

// some casting MACROS
#if defined(__cplusplus)
#define HASHMAP_CAST(type, x) static_cast<type>(x)
#define HASHMAP_PTR_CAST(type, x) reinterpret_cast<type>(x)
#else
#define HASHMAP_CAST(type, x) ((type)x)
#define HASHMAP_PTR_CAST(type, x) ((type)x)
#endif

bool ekonHashmapInit(EkonAllocator *a, const u32 initSize,
                     EkonHashmap *const outHashmap) {
  // 0 or power of 2
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

  *addr = m->data + index;

  // If the hashmap element was not
  // already in use, set that is being
  // used and bump size
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

  // Not found
  return NULL;
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

  /* Linear probing */
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

bool ekonHashmapMatchHelper(const struct hashmap_element_s *const element,
                            const char *const key, const unsigned len) {
  return (element->keyLen == len) && (memcmp(element->key, key, len) == 0);
}

bool ekonHashmapHashHelper(const EkonHashmap *const m, const char *const key,
                           const u32 len, u32 *const outIndex) {
  u32 curr;
  u32 i;

  // If full, return immediately
  if (m->size >= m->tableSize) {
    return true;
  }

  // Find the best index
  curr = ekonHashmapHashHelperIntHelper(m, key, len);

  // Linear probing
  for (i = 0; i < HASHMAP_MAX_CHAIN_LENGTH; i++) {
    if (!m->data[curr].inUse) {
      *outIndex = curr;
      return false;
    }

    if (m->data[curr].inUse &&
        ekonHashmapMatchHelper(&m->data[curr], key, len)) {
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

#endif
