
#include <stdio.h>
#include "A.hpp"

A::A()
{
  c();
}

int A::a()
{
  printf("a()\n");
  get_b()->b();
  return 0;
}

B* A::get_b()
{
  printf("get_b()\n");
  return &m_b;
}

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
