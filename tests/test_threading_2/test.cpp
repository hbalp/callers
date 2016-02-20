#include "A.hpp"
#include "B.hpp"
#include "C.hpp"
#include "D.hpp"
#include "test.hpp"

#include <pthread.h>

int Global;

void *Thread1(void *x)
{
  Global = 42;
  A* a = new C();
  a->a();
  B* b = a->get_b();
  b->b();
  b->coucou2();
  a->c();
  // C* d = new C();
  // ::Newly::Added::D* d = new ::Newly::Added::D();
  C* d = new ::Newly::Added::D();

  //assert_callers(d, (D*) null_ptr, );
  //ACSL++ avec les methodes virtuelles
  /*@
    @ */
  d->c();
  delete d;
  delete a;
  return x;
}

int main() {

  pthread_t t1, t2;
  pthread_create(&t1, NULL, Thread1, NULL);
  Global = 43;
  pthread_create(&t2, NULL, Thread1, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  return Global;
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
