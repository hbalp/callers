#include "A.hpp"
#include "B.hpp"
#include "C.hpp"
#include "D.hpp"
#include "test.hpp"

#include <boost/algorithm/string/predicate.hpp>

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

namespace tintin {

  int coucou1()
  {
    std::string prefix = "hello";
    std::string who = "world";
    std::pair<std::string, std::string> info = std::make_pair(prefix, who);
    std::string msg = info.first + " " + info.second + " !";
    bool has_prefix = boost::algorithm::contains(msg, prefix);
    if (has_prefix)
    {
      std::cout << "coucou1 say hello: " << msg << std::endl;
    }
    else
    {
      std::cout << "coucou1 doesn't say hello..." << std::endl;
    }
    return 0;
  }
}

B* A::get_b()
{
  printf("get_b()\n");
  return &m_b;
}


/* Local Variables: */
/* compile-command: "clang++ -g  -I /c/ProgramFiles/MinGW/include -o test_local_callgraph_cpp test_local_callgraph.cpp" */
/* End: */
