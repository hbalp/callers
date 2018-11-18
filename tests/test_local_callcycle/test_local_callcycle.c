#include <stdio.h>

void a();

int c()
{
  printf("c()\n");
  a();
  return 0;
}

int b()
{
  printf("b()\n");
  c();
  return 0;
}

void a()
{
  printf("a()\n");
  b();
}

int main()
{
  a();
  return 0;
}

/* Local Variables: */
/* compile-command: "clang -g -o test_local_callcycle_c test_local_callcycle.c; valgrind --tool=callgrind ./test_local_callcycle_c" */
/* End: */
