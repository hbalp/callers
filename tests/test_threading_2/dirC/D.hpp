#ifndef _D_H_
#define _D_H_

#include <stdio.h>
#include "C.hpp"

namespace Newly {

  namespace Added {

    class D : public C {

    public:
      //D();
      //~D();
      virtual int c();
    };
  };
};

#endif
