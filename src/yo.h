#include <stddef.h>
#include <stdlib.h>

typedef struct Yo {
  int a;
  struct Yo *yo;
} Yo;

Yo *funcyo(int a) {
  Yo *yo2 = (Yo *)malloc(sizeof(Yo));
  yo2->a = a + 4;
  yo2->yo = NULL;
  Yo *yo1 = (Yo *)malloc(sizeof(Yo));
  yo1->a = a;
  yo1->yo = yo2;
  return yo1;
}
