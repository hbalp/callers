#include "A.hpp"
#include "B.hpp"
#include "test.hpp"

int main()
{
  A* a = new A();
  a->a();
  B* b = a->get_b();
  b->b();
  return 0;
}

B* A::get_b()
{
  printf("get_b()\n");
  return &m_b;
}
