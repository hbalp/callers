#include <stdio.h>
#include "test_dummy.h"

int f() { return 0; }

int main()
{
  printf("dummy test\n");
  f();
  int zero = 0;

  // div by zero fault injection
  int div = 20/zero;
  return div;

  //return 0;
}
