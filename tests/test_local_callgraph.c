#include <stdio.h>

int c()
{
  printf("c()\n");
}

int b()
{
  printf("b()\n");
  c();
}

int a()
{
  printf("a()\n");
  b();
}

int main()
{
  a();
}

/* Local Variables: */
/* compile-command: "clang -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph test_local_callgraph.c" */
/* End: */
