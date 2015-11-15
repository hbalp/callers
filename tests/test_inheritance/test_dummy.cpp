#include <stdio.h>

class A
{
public:
  virtual int f() { return 0; }
};

class B : public A
{
public:
  int f() { return 1; }  
};

int main()
{
  printf("dummy test\n");

  // division by zero
  // div by zero fault injection
  //A b; 
  //A* b = new A();
  // no division by zero
  //B b; 
  //int denom = b.f();
  A* b = new B();

  int denom = b->f();
  int div = 20/denom;
  return div;
}
