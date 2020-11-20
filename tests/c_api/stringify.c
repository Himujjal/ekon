#include "../../ekon.h"
#include "utils.h"

bool EKON_Parse(Value *srcV, const char *srcEkon) {
  char *message = "";
  bool success = ekonValueParseFast(srcV, srcEkon, &message);
  printf("Error Message: %s\n", message);
  return success;
}

int main() {
  printf("-----\n");
  const char *srcEkon = readFromFile("test.ekon");
  // Allocate new chunk of memory
  Allocator *a = ekonAllocatorNew();

  // Add a new value
  Value *srcV = ekonValueNew(a);

  if (!EKON_Parse(srcV, srcEkon))
    return 1;

  const char *src = ekonValueStringify(srcV, true);

  printf("----src----\n%s\n-----\n", src);

  // ReleaseAllocator
  ekonAllocatorRelease(a);
  free((void *)srcEkon);
}
