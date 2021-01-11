#include "./ekon.h"
#include "./lsp/ekon_schema.h"
#include "./tests/c_api/utils.h"
#include "hashmap.h"
#include <stdio.h>
#include <string.h>

// Get the EKON and Set some EKON
void getAndSet(EkonValue *src, EkonValue *des) {
  // Set EKON Type
  const EkonType *t;
  t = ekonValueType(src);
  if (t == 0)
    return;

  switch (*t) {
  case EKON_TYPE_ARRAY: {
    ekonValueSetArray(des);
    EkonValue *next = ekonValueBegin(src);
    while (next != 0) {
      EkonValue *v = ekonValueNew(des->a);
      getAndSet(next, v);
      if (ekonValueArrayAddFast(des, v) != true) {
        return;
      }
      next = ekonValueNext(next);
    }
    break;
  }
  case EKON_TYPE_OBJECT: {
    ekonValueSetObj(des); // init
    EkonValue *next = ekonValueBegin(src);
    while (next != 0) {
      EkonValue *v = ekonValueNew(des->a);
      uint8_t option;
      uint32_t keyLen = 0;
      const char *key = ekonValueGetKey(next, &option, &keyLen);
      if (src->n->keymap != 0 &&
          ekonHashmapGet(src->n->keymap, key, keyLen) != NULL) {
        // key is already present
      }
      ekonValueSetKeyFast(v, key, option, true);
      getAndSet(next, v);
      if (ekonValueObjAddFast(des, v) != true)
        return;
      next = ekonValueNext(next);
    }
    break;
  }
  case EKON_TYPE_BOOL: {
    const bool *b = ekonValueGetBool(src);
    if (b == 0)
      return;
    ekonValueSetBool(des, *b);
    break;
  }
  case EKON_TYPE_NUMBER: {
    const char *str = ekonValueGetNumStr(src);
    if (str == 0)
      return;
    if (ekonValueSetNumStr(des, str) == false)
      return;
    break;
  }
  case EKON_TYPE_NULL: {
    if (ekonValueIsNull(src) == false)
      return;
    ekonValueSetNull(des);
    break;
  }
  case EKON_TYPE_STRING: {
    uint8_t option;
    const char *str = ekonValueGetStr(src, &option);
    if (str == 0)
      return;
    if (!ekonValueSetStrFast(des, str, option)) {
      return;
    }
    break;
  }
  }
}

bool EKON_Parse(EkonValue *srcV, const char *srcEkon) {
  char *errMessage = NULL;
  char *schema = NULL;
  bool success = ekonValueParseFast(srcV, srcEkon, &errMessage, &schema);
  ekonParseSchema(schema, strlen(schema));
  if (success == false)
    printf("Error Message: %s", errMessage);
  free(schema);
  return success;
}

int main() {
  printf("Size of EkonNode: %ld\n--", sizeof(EkonNode) + sizeof(EkonHashmap));
  const char *str = "specs1.ekon";
  const char *srcEkon = readFromFile(str);
  /* printf("%s\n", srcEkon); */
  // Allocate new chunk of memory
  EkonAllocator *a = ekonAllocatorNew();

  // Add a new value
  EkonValue *srcV = ekonValueNew(a);
  EkonValue *desV = ekonValueNew(a);

  // ... EKON ..
  printf("----- Parsing... ------\n");
  bool ret = EKON_Parse(srcV, srcEkon);
  if (!ret) {
    return 1;
  }

  printf("----- getAndSet... ----\n");
  /* printf("%s\n", src); */

  // Get and Set EKON
  getAndSet(srcV, desV);

  // DesEkon
  const char *des = ekonValueStringify(desV, true);
  printf("~>stringified:\n%s\n", (des));
  /* printf("------beauty--------\n"); */

  char *err2;
  const char *beautifiedDes = ekonBeautify(des, &err2,
                                           (EkonBeautifyOptions){
                                               false,
                                               false,
                                               false,
                                           });

  if (beautifiedDes == 0) {
    printf("Beautification Error: %s\n", err2);
    return 1;
  }
  printf("~~>Beautified:\n%s\n", beautifiedDes);

  /* const char *strUnes = ekonValueStringifyUnEscaped(desV); */
  /* if (strUnes == 0) { */
  /*   printf("Unescape Str Error\n"); */
  /*   return 1; */
  /* } */
  /* printf("~>Unescaped:%s\n", (strUnes)); */

  /* printf("------json------\n"); */
  /* const char *json = ekonValueStringifyToJSON(desV); */
  /* if (json == 0) { */
  /*   fprintf(stderr, "Error stringifying JSON"); */
  /*   return 1; */
  /* } */
  /* printf("~~JSON->%s\n", json); */

  // ReleaseAllocator
  ekonAllocatorRelease(a);
  free((void *)srcEkon);
  /* free((void *)des); */
  /* free((void *)src); */
  /* free(err2); */

  return 0;
}
