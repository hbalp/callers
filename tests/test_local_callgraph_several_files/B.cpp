#include <stdio.h>
#include "B.hpp"

int c()
{
  printf("c()\n");
  return 0;
}

B::B() 
{
  // contournement du besoin d'ajouter les liens de type extcallers
  //c();
}

int B::b()
{
  printf("b()\n");
  c();
  return 0;
}

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
