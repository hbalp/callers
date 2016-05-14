
//#define FRAMA_C

#ifndef FRAMA_C

//#include <stdlib.h>
#include "stdlib.c"

int main(int argc, char** argv)
{
  char* go = malloc(10);
  go[0] = '\0';
  return go[0];
}

#else

#include "__fc_builtin.h"

int main(int argc, char** argv)
{
  char* go = Frama_C_alloc_size(10);
  go[0] = '\0';
  return go[0];
}

#endif
