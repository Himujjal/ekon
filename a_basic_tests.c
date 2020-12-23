#include "./ekon.h"
#include "./tests/c_api/utils.h"
#include <stdio.h>
#include <string.h>

// Get the EKON and Set some EKON
void getAndSet(Value *src, Value *des) {
  // Set EKON Type
  const EkonType *t;
  t = ekonValueType(src);
  if (t == 0)
    return;

  switch (*t) {
  case EKON_TYPE_ARRAY: {
    ekonValueSetArray(des);
    struct EkonValue *next = ekonValueBegin(src);
    while (next != 0) {
      struct EkonValue *v = ekonValueNew(des->a);
      getAndSet(next, v);
      if (ekonValueArrayAddFast(des, v) != true) {
        return;
      }
      next = ekonValueNext(next);
    }
    break;
  }
  case EKON_TYPE_OBJECT: {
    ekonValueSetObj(des);
    Value *next = ekonValueBegin(src);
    while (next != 0) {
      Value *v = ekonValueNew(des->a);
      uint8_t option;
      const char *key = ekonValueGetKey(next, &option);
      ekonValueSetKeyFast(v, key, option);
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
    if (ekonValueSetNumStr(des, str) != true)
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

bool EKON_Parse(Value *srcV, const char *srcEkon) {
  char *message;
  bool success = ekonValueParseFast(srcV, srcEkon, &message);
  if (success == false)
    printf("Error Message: %s", message);
  return success;
}

int main() {
  printf("-----\n");
  const char *str = "specs1.ekon";
  const char *srcEkon = readFromFile(str);
  /* printf("%s\n", srcEkon); */

  // Allocate new chunk of memory
  Allocator *a = ekonAllocatorNew();

  // Add a new value
  Value *srcV = ekonValueNew(a);
  Value *desV = ekonValueNew(a);

  // ... EKON ..
  bool ret = EKON_Parse(srcV, srcEkon);

  if (!ret) {
    printf("Parse-Fast Failed\n");
    return 1;
  }

  /* const char *src = ekonValueStringify(srcV); */
  printf("---------------\n");
  /* printf("%s\n", src); */

  // Get and Set EKON
  getAndSet(srcV, desV);

  // DesEkon
  const char *des = ekonValueStringify(desV);
  printf("%s\n", des);

  // ReleaseAllocator
  ekonAllocatorRelease(a);
  free((void *)srcEkon);
  /* free((void *)des); */
  /* free((void *)src); */

  return 0;
}
