#include "../../ekon.h"
#include <stdio.h>

// Get the EKON and Set some EKON
void getAndSet(Value *src, Value *des) {
  // Set EKON Type
  const EkonType *t;
  t = Type(src);
  if (t == 0)
    return;

  switch (*t) {
  case EKONTYPEARRAY: {
    SetArray(des);
    Value *next = Begin(src);
    while (next != 0) {
      Value *v = NewValue(des->a);
      getAndSet(next, v);
      if (ArrayAddFast(des, v) != true)
        return;
      next = Next(next);
    }
    break;
  }
  case EKONTYPEOBJECT: {
    // For Object Type
    SetObj(des);
    Value *next = Begin(src);
    while (next != 0) {
      Value *v = NewValue(des->a);
      SetKeyFast(v, GetKey(next));
      getAndSet(next, v);
      if (ObjAddFast(des, v) != true)
        return;
      next = Next(next);
    }
    break;
  }
  case EKONTYPEBOOL: {
    const bool *b = GetBool(src);
    if (b == 0)
      return;
    SetBool(des, *b);
    break;
  }
  case EKONTYPENUMBER: {
    const char *str = GetNumStr(src);
    if (str == 0)
      return;
    if (SetNumStr(des, str) != true)
      return;
    break;
  }
  case EKONTYPENULL: {
    if (IsNull(src) == false)
      return;
    SetNull(des);
    break;
  }
  case EKONTYPESTRING: {
    // if the type is a string
    const char *str = GetStr(src);
    if (str == 0)
      return;
    if (SetStrFast(des, str) != true)
      return;
    break;
  }
  }
}

bool EKON_Parse(Value *srcV, const char *srcEkon) {
  return ParseFast(srcV, srcEkon);
}

int main() {
  const char *srcEkon = "[{\"key\":\"value\"}]";

  /* const char *srcEkon =
   * "[{\"key\":true},false,{\"key1\":true},[null,false,[]," */
  /*                       "true],[\"\",123,\"str\"],null]"; */

  // Allocate new chunk of memory
  Allocator *a = NewAllocator();

  // Add a new value
  Value *srcV = NewValue(a);
  Value *desV = NewValue(a);

  // ... EKON ..
  bool ret = EKON_Parse(srcV, srcEkon);
  if (!ret) {
    printf("ParseFast failed\n");
    return 1;
  }

  // Get and Set EKON
  getAndSet(srcV, desV);

  const char *desEkon = Stringify(desV);

  printf("srcEkon: \n\t%s", srcEkon);
  if (desEkon != 0)
    printf("desEkon:%s\n", desEkon);

  // ReleaseAllocator
  ReleaseAllocator(a);

  return 0;
}
