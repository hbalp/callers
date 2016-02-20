#ifndef _C_H_
#define _C_H_

#include <stdio.h>
#include "A.hpp"

class C : public A
{
public:
  C();
//~C();

  virtual int c();
};

#endif
