#include <stdio.h>
#include "D.hpp"

namespace Newly
{
  namespace Added {

    int D::c()
    {
      printf("D::c()\n");
      ::tintin::coucou1();
      return 0;
    }
  };
};

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
