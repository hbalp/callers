#include <stdio.h>
#include "test_inheritance.h"

int main()
{
  A* a = new C();
  a->c();
  delete a;

  // D* d = new D();
  // d->c();
  // delete d;

  return 0;
}

A::A (int toto)
  : toto(toto)
{}

int A::c()
{
  printf("A::c()\n");
  return 0;
}

int D::c()
{
  printf("D::c()\n");
  return 0;
}

C::C (int titi, int toto) :
  titi(titi),
  A(toto) {};

/* B* A::get_b() */
/* { */
/*   printf("get_b()\n"); */
/*   return &m_b; */
/* } */

