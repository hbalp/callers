// application.cpp

#define NEED_PARENT
#include "library.h"
#undef NEED_PARENT

int main(int argc, char** argv) {
   ParentCommon my_struct(3);
   return my_struct.getX();
}
