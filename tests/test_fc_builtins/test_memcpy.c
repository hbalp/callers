#include <stdio.h>

#define OFF 0
#define ON 1

#ifndef FRAMA_C
#include "string.h"
#include "stdlib.h"
#else
#include "string.c"
#include "stdlib.c"
#endif

#define LENGTH 20

char src[LENGTH] = "hello world";
char *dest;
//char dest[LENGTH] = "";

int main()
{
  char* dest = malloc(LENGTH * sizeof(char));
  char* dst = memcpy(dest, src, LENGTH);
  printf("src=%s\ndest=%s\n", src, dest);
  return 0;
}
