#ifndef _A_H_
#define _A_H_

#include <stdio.h>
#include "B.hpp"

class B;

class A
{
public:
  A();
  virtual ~A() {}

  int a();

  virtual int c() = 0;

  B* get_b();

private:
  B m_b;
};

#endif
