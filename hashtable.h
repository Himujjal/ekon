#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>

struct EkonNode;
struct EkonAllocator;

struct _EkonHashItem {
  char *key;
  struct EkonNode *value;
  struct _EkonHashItem *next;
};
typedef struct _EkonHashItem EkonHashItem;

struct _EkonHashTable {
  int size;
  struct _EkonHashItem **table;
};
typedef struct _EkonHashTable EkonHashTable;

uint32_t ekonHashString(EkonHashTable *hashTable, const char *key,
                        uint32_t len);
EkonHashTable *ekonHashInitTable(struct EkonAllocator *a, uint32_t size);
EkonHashItem *ekonHashNewPair(struct EkonAllocator *a, char *key,
                              uint32_t keyLen, struct EkonNode *value);
void ekonHashSet(struct EkonAllocator *a, EkonHashTable *hashTable, char *key,
                 uint32_t keyLen, struct EkonNode *value);
struct EkonNode *ekonHashGet(EkonHashTable *hashTable, char *key,
                             uint32_t keyLen);

#endif
