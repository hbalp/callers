#ifndef _B_H_
#define _B_H_

#include <stdio.h>
#include "test.hpp"

//int c();
inline int c()
{
  printf("c()\n");
  // test des cycles
  main();
  return 0;
}

class B
{
public:
  B();
  ~B() {}
  
  int b();
};

#endif
