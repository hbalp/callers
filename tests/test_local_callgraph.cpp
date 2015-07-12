#include <stdio.h>

class B
{
public:
  B() {}
  ~B() {}
  
  int b();
};

class A
{
public:
  A() {}
  ~A() {}
  
  int a();

  B* get_b();

private:
  B m_b;
};

int c()
{
  printf("c()\n");
  return 0;
}

int B::b()
{
  printf("b()\n");
  c();
  return 0;
}

int A::a()
{
  printf("a()\n");
  get_b()->b();
  return 0;
}

B* A::get_b()
{
  printf("get_b()\n");
  return &m_b;
}

int main()
{
  A* a = new A();
  a->a();
  B* b = a->get_b();
  b->b();
  return 0;
}

/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
