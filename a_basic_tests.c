#include "./ekon.h"
#include <stdio.h>
#include <string.h>

const char *readFromFile(const char *fileName) {
  FILE *fp;
  long lSize;
  char *buff;

  fp = fopen(fileName, "r");
  if (!fp)
    perror(strcat("Couldn't open ", fileName)), exit(1);

  fseek(fp, 0L, SEEK_END);
  lSize = ftell(fp);
  rewind(fp);

  // allocate memory
  buff = calloc(1, lSize + 1);
  if (1 != fread(buff, lSize, 1, fp))
    fclose(fp), free(buff), fputs("Entire read fails", stderr), exit(1);

  fclose(fp);
  return buff;
}

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
      ekonValueSetKeyFast(v, ekonValueGetKey(next));
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
    const char *str = ekonValueGetStr(src);
    if (str == 0)
      return;
    if (!ekonValueSetStrFast(des, str)) {
      return;
    }
    break;
  }
  }
}

bool EKON_Parse(Value *srcV, const char *srcEkon) {
  return ekonValueParseFast(srcV, srcEkon);
}

int main() {
  printf("-----\n");
  const char *srcEkon = readFromFile("specs.ekon");

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

  const char *src = ekonValueStringify(srcV);
  printf("srcEkon Stringified: %s\n", src);

  // Get and Set EKON
  getAndSet(srcV, desV);

  // DesEkon
  const char *des = ekonValueStringify(desV);
  printf("desEkon Stringified: %s\n", des);

  // ReleaseAllocator
  ekonAllocatorRelease(a);
  free((void *)srcEkon);
  /* free((void *)des); */
  /* free((void *)src); */

  return 0;
}
