#include <stdio.h>

int c()
{
  printf("c()\n");
  return 0;
}

int b()
{
  printf("b()\n");
  c();
  return 0;
}

int a()
{
  printf("a()\n");
  b();
  return 0;
}

int main()
{
  a();
  return 0;
}

/* Local Variables: */
/* compile-command: "clang -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph test_local_callgraph.c" */
/* End: */
