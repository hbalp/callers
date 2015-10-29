#include <stdio.h>
#include "D.hpp"

int ::Newly::Added::D::c()
{
  printf("D::c()\n");
  return 0;
}

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
