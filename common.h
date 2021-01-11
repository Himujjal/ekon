#ifndef EKON_COMMON_H
#define EKON_COMMON_H

#include <stdbool.h>
#include <stdint.h>

#define u32 uint32_t
#define u16 uint16_t

struct _EkonNode;

struct _EkonANode {
  char *data;
  u32 size;
  u32 pos;
  struct _EkonANode *next;
};
#define EkonANode struct _EkonANode

// Allocator
struct _EkonAllocator {
  EkonANode *root;
  EkonANode *end;
};
#define EkonAllocator struct _EkonAllocator

// HashMapItem
struct hashmap_element_s {
  const char *key;
  u32 keyLen;
  bool inUse;
  struct _EkonNode *value;
};
#define EkonHashmapItem struct hashmap_element_s

// HashMap
struct hashmap_s {
  u32 tableSize; // max size
  u32 size;      // current size
  EkonHashmapItem *data;
};
#define EkonHashmap struct hashmap_s

// enumerate types with numbers
enum _EkonType {
  EKON_TYPE_BOOL,
  EKON_TYPE_ARRAY,
  EKON_TYPE_OBJECT,
  EKON_TYPE_STRING,
  EKON_TYPE_NULL,
  EKON_TYPE_NUMBER,
};
#define EkonType enum _EkonType

// The primary json node
struct _EkonNode {
  EkonType ekonType;
  u16 option; // holds values from EKON_NODE_OPTIONS
  const char *key;
  u32 keyLen;

  EkonHashmap *keymap; // duplicate prevention & faster retrieval of objects
  EkonHashmapItem *hashItem; // pointer to hashitem having current node
  union {
    struct _EkonNode *node;
    const char *str;
  } value;
  u32 len; // string length
  struct _EkonNode *next;
  struct _EkonNode *prev;
  struct _EkonNode *father;
  struct _EkonNode *end;
} _EkonNode;
#define EkonNode struct _EkonNode

#endif
