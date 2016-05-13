#include "__fc_builtin.h"

int main(int argc, char** argv)
{
  char* go = Frama_C_alloc_size(10);
  go[0] = '\0';
  return go[0];
}
