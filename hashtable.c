// hand writte hash table that uses EkonAllocator
// based on https://gist.github.com/tonious/1377667
#include "hashtable.h"
#include "ekon.h"
#include "string.h"

#include <limits.h>

uint32_t ekonHashString(EkonHashTable *hashTable, const char *key,
                        uint32_t len) {
  uint64_t hashVal;
  int i = 0;

  // Convert our string to an integer
  while (hashVal < ULONG_MAX && i < len) {
    hashVal = hashVal << 8;
    hashVal += key[i];
    i++;
  }

  return hashVal % hashTable->size;
}

/**
 * a - EkonAllocator to allocate on a
 * size - inititial size of the hashTable
 * returns - NULL | EkonHashTable
 * */
EkonHashTable *ekonHashInitTable(struct EkonAllocator *a, uint32_t size) {
  EkonHashTable *hashTable = NULL;
  if (size < 1)
    return NULL;

  // Allocate the table itself
  if ((hashTable = (EkonHashTable *)ekonAllocatorAlloc(
           a, sizeof(EkonHashTable))) == NULL)
    return NULL;

  // Allocate pointers to head nodes
  if ((hashTable->table = (EkonHashItem **)ekonAllocatorAlloc(
           a, sizeof(EkonHashItem *) * size)) == NULL)
    return NULL;

  for (uint32_t i = 0; i < size; i++)
    hashTable->table[i] = NULL;

  hashTable->size = size;

  return hashTable;
}

/*
 * Generates a new key-value pair
 * returns - (EkonHashItem *)?
 */
EkonHashItem *ekonHashNewPair(struct EkonAllocator *a, char *key,
                              uint32_t keyLen, struct EkonNode *value) {
  EkonHashItem *newPair;

  if ((newPair = (EkonHashItem *)ekonAllocatorAlloc(a, sizeof(EkonHashItem))) ==
      NULL) {
    return NULL;
  }

  if ((newPair->key = strndup(key, keyLen)) == NULL)
    return NULL;

  newPair->value = value;
  newPair->next = NULL;

  return newPair;
}

/**
 * Insert a new pair into the hash table
 * */
void ekonHashSet(struct EkonAllocator *a, EkonHashTable *hashTable, char *key,
                 uint32_t keyLen, struct EkonNode *value) {
  uint32_t bin = 0;
  EkonHashItem *newPair = NULL;
  EkonHashItem *next = NULL;
  EkonHashItem *last = NULL;

  bin = ekonHashString(hashTable, key, keyLen);
  next = hashTable->table[bin];

  next = hashTable->table[bin];

  while (next != NULL && next->key != NULL && strcmp(key, next->key) > 0) {
    last = next;
    next = next->next;
  }

  // replace an existing node or grow a new pair
  if (next != NULL && next->key != NULL && strcmp(key, next->key) == 0) {
    next->value = value;
  } else {
    newPair = ekonHashNewPair(a, key, keyLen, value);

    // start of linked list in this bin
    if (next == hashTable->table[bin]) {
      newPair->next = next;
      hashTable->table[bin] = newPair;
    } else if (next == NULL) {
      last->next = newPair;
      // We are in the middle of the list.
    } else {
      newPair->next = next;
      last->next = newPair;
    }
  }
}

/**
 * Retrieve a key-value pair from a hash table
 * returns - (struct EkonNode*)?
 * */
struct EkonNode *ekonHashGet(EkonHashTable *hashTable, char *key,
                             uint32_t keyLen) {
  int bin = 0;
  EkonHashItem *pair;

  bin = ekonHashString(hashTable, key, keyLen);

  // Step through the bin, looking for our value
  while (pair != NULL && pair->key != NULL && strcmp(key, pair->key) > 0) {
    pair = pair->next;
  }

  // Did we actually find anything?
  if (pair == NULL || pair->key == NULL || strcmp(key, pair->key) != 0) {
    return NULL;
  } else {
    return pair->value;
  }
}
