#include "A.hpp"
#include "B.hpp"
#include "C.hpp"
#include "D.hpp"
#include "test.hpp"

bool coucou = true;

int main()
{
  A* a = new C();
  a->a();
  if(coucou)
  {
    B* b = a->get_b();
    b->b();
    b->coucou2();
  }
  a->c();
  // C* d = new C();
  // ::Newly::Added::D* d = new ::Newly::Added::D();
  C* d = new ::Newly::Added::D();
  //C* d = new D();

  //assert_callers(d, (D*) null_ptr, );
  //ACSL++ avec les methodes virtuelles
  /*@
    @ */
  d->c();
  delete d;
  delete a;
  return 0;
}

B* A::get_b()
{
  printf("get_b()\n");
  return &m_b;
}

int coucou1()
{
  printf("coucou1()\n");
  return 0;
}


/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
