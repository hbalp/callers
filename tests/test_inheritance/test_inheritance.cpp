#include <stdio.h>
#include "test_inheritance.h"

int main()
{
  printf("main()\n");

  A* a = new C();
  a->v();
  a->nv();
  delete a;

  // D* d = new D();
  // d->c();
  // delete d;

  return 0;
}

A::A (int toto)
  : toto(toto)
{}

/*
int A::v()
{
  printf("A::v()\n");
  return 0;
}
*/

int D::v()
{
  printf("D::v()\n");
  return 0;
}

int D::nv()
{
  printf("D::nv()\n");
  return 0;
}

C::C (int titi, int toto) :
  titi(titi),
  A(toto) {};

int C::nv()
{
  printf("C::nv()\n");
  return 0;
}

int C::v()
{
  printf("C::v()\n");
  return 0;
}

/* B* A::get_b() */
/* { */
/*   printf("get_b()\n"); */
/*   return &m_b; */
/* } */

