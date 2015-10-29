#include <stdio.h>

class A
{
public:
  int f() { return 0; }
};

class B : public A
{
  
};

int main()
{
  printf("dummy test\n");
  B b;
  b.f();
  int zero = 0;

  // div by zero fault injection
  int div = 20/zero;
  return div;

  //return 0;
}
