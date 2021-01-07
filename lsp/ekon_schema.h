#ifndef EKON__SCHEMA_H
#define EKON__SCHEMA_H

#include <stdint.h>
#include <stdio.h>

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
} EkonSchemaTypes;

// ----------- FUNCTION PROTOTYPES -------------

static inline void ekonParseSchema(char *schema, uint32_t len);

// ----------- ------------------- -------------
static inline void ekonParseSchema(char *schema, uint32_t len) {
  printf("schema: %s\n", schema);
}

#endif
