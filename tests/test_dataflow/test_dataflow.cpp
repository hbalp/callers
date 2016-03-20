#include <stdio.h>
#include "test_dataflow.h"

int f(char *val, int denom, Toto toto)
{
  printf("int f(val=%s, denom=%d, toto.name=%s, toto.nb=%d)\n", val, denom, toto.name, toto.nb);
  // div by zero fault injection
  int div = 20/denom;
  return div;
}

int g(char *val, int denom, Toto toto)
{
  printf("int g(val=%s, denom=%d, toto.name=%s, toto.nb=%d)\n", val, denom, toto.name, toto.nb);
  int res = f(val, denom, toto);
  return res;
}

int main()
{
  printf("dataflow test\n");
  Toto toto;
  char tutu[] = "tutu";
  toto.name = tutu;
  int denom = 5;
  char label[] = "division: ";
  int ret = g(label, denom, toto);
  printf("div=20/%d=%d\n", denom, ret);
  return ret;
}
