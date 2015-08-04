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
/* compile-command: "clang -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_c test_local_callgraph.c; valgrind --tool=callgrind ./test_local_callgraph_c" */
/* End: */
