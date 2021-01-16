#ifndef EKON__SCHEMA_H
#define EKON__SCHEMA_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
  EKON_SCHEMA_BOOL,
  EKON_SCHEMA_STRING,
  EKON_SCHEMA_NUMBER,
  EKON_SCHEMA_NULL,
  EKON_SCHEMA_ARRAY,
  EKON_SCHEMA_TUPLE,
  EKON_SCHEMA_OBJECT,
  EKON_SCHEMA_ENUM,
  EKON_SCHEMA_UNION,
  EKON_SCHEMA_SUM
} EkonSchemaType;

typedef uint32_t u32;
typedef uint8_t u8;

struct EkonSchemaNode {
  EkonSchemaType *nodeTypes; // array of types
  u32 numTypes;
  size_t typesSize;
  struct EkonSchemaNode **nodes;
  u32 numNodes;
  size_t nodesSize;
};
typedef struct EkonSchemaNode EkonSchemaNode;

// ----------- FUNCTION PROTOTYPES -------------

static inline void ekonParseSchema(char *schema, u32 len);
static inline void ekonSchemaNodeAddType(EkonSchemaNode *node,
                                         EkonSchemaType typ);
static inline void ekonSchemaNodeAddNode(EkonSchemaNode *father,
                                         EkonSchemaNode *child);
static inline void ekonParseSchema(char *schema, u32 len);
// ---------------------------------------------

static inline EkonSchemaNode *ekonInitSchemaNode() {
  EkonSchemaNode *node = (EkonSchemaNode *)malloc(sizeof(EkonSchemaNode));
  EkonSchemaType *types = (EkonSchemaType *)malloc(sizeof(EkonSchemaType) * 2);
  node->numTypes = 0;
  node->typesSize = 2;

  EkonSchemaNode **nodes =
      (EkonSchemaNode **)malloc(sizeof(EkonSchemaNode *) * 2);
  node->numNodes = 0;
  node->nodesSize = 2;

  return node;
}

static inline void ekonSchemaNodeAddType(EkonSchemaNode *node,
                                         EkonSchemaType typ) {
  if (node->numTypes == node->typesSize) {
    node->typesSize *= 2;
    node->nodeTypes = (EkonSchemaType *)realloc(
        node->nodeTypes, node->typesSize * sizeof(EkonSchemaType));
  }
  node->nodeTypes[node->numTypes++] = typ;
}

static inline void ekonSchemaNodeAddNode(EkonSchemaNode *parent,
                                         EkonSchemaNode *child) {}

static inline void ekonFreeSchemaNode(EkonSchemaNode *node) {
  free(node->nodes);
  free(node->nodeTypes);
  node->nodeTypes = NULL;
  node->nodes = NULL;
  node->nodesSize = node->numNodes = node->typesSize = node->numTypes = 0;
}

static inline void ekonParseSchema(char *schema, u32 len) {
  printf("schema: %s\n", schema);
}

#endif
