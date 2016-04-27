#ifndef _GDB_DEBUG_H_
#define _GDB_DEBUG_H_

#include <assert.h>

void gdb_print_trace();

#ifdef NDEBUG /* Release Mode */

# define ASSERT(cond) assert(cond)

#else /* Debug Mode */

# define ASSERT(cond)                                       \
  if(!(cond))                                               \
   {                                                        \
     gdb_print_trace();                                     \
     assert(cond);                                          \
   }

#endif /* ASSERT */

#endif
