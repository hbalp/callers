#include <stdio.h>
#include "B.hpp"
#include "test.hpp"

int coucou3()
{
  printf("coucou3()\n");
  return 0;
}

int c()
{
  printf("c()\n");
  // test des cycles
  //main();
  return 0;
}

B::B()
{
  //c();
}

int B::b()
{
  printf("b()\n");
  c();
  return 0;
}

int B::coucou2()
{
  printf("coucou2()\n");
  return 0;
}

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
