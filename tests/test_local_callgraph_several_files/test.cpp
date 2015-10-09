#include "A.hpp"
#include "B.hpp"
#include "C.hpp"
#include "test.hpp"

int main()
{
  A* a = new C();
  a->a();
  B* b = a->get_b();
  b->b();
  a->c();
  delete a;
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
