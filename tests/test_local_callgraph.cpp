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
  delete a;
  return 0;
}

/* Local Variables: */
/* compile-command: "clang++ -g -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* compile-command: "clang++ --analyze -Xanalyzer -analyzer-checker=debug.DumpCallGraph ./test_local_callgraph.cpp" */
/* compile-command: "clang++ --analyze -Xanalyzer -analyzer-checker=debug.ViewCallGraph ./test_local_callgraph.cpp" */
/* compile-command: "scan-build -V --use-analyzer=`which clang++` -enable-checker debug.DumpCallGraph clang++ ./test_local_callgraph.cpp -g -o test_local_callgraph_cpp_clang++" */
/* compile-command: "scan-build -V --use-analyzer=`which clang++` -enable-checker debug.ViewCallGraph clang++ ./test_local_callgraph.cpp -g -o test_local_callgraph_cpp_clang++" */
/* compile-command: "scan-build -V --use-analyzer=`which clang++` -enable-checker debug.ViewCallGraph g++ ./test_local_callgraph.cpp -g -o test_local_callgraph_cpp_g++" */
/* End: */
