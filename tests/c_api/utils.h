// This is a util only header.

#include <error.h>
#include <stdio.h>
#include <stdlib.h>
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
