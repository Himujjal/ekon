#include <stdio.h>
#include <string.h>
#include "../../ekon.h"

int main() {
    char* s = "himujjal-/on-wocks@upa_dhyaya.com";
    printf("valid: %s\n", ekonCheckUnquotedString(s, strlen(s)) ? "true" : "false");
    return 0;
}