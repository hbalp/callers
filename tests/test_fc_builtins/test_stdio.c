
//#define EOF (-1)

#include <stdio.h>

int test_eof()
{
  printf("EOF: %d\n", EOF);
  return 0;
}

#ifndef FRAMA_C
int main()
{
  test_eof();
}
#endif
