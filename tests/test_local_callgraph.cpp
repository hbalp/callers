#include <stdio.h>

class A
{
public:
  A() {}
  ~A() {}
  
  int a();

private:
  int b();
};

int c()
{
  printf("c()\n");
  return 0;
}

int A::b()
{
  printf("b()\n");
  c();
  return 0;
}

int A::a()
{
  printf("a()\n");
  b();
  return 0;
}

int main()
{
  A* a = new A();
  a->a();
  return 0;
}

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
