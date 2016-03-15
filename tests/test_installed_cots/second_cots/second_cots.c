#include <stdio.h>
#include "test_dummy.h"

int main()
{
  printf("dummy test\n");
  f();
  //int zero = 0;
  int zero = 1;

  // div by zero fault injection
  int div = 20/zero;
  return div;

  //return 0;
}
