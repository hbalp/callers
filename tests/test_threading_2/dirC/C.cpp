#include <stdio.h>
#include "C.hpp"

C::C()
{
  //c();
}

int C::c()
{
  printf("C::c()\n");
  return 0;
}

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
