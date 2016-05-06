#include <stdio.h>
#include "test_saml_sign.h"

int division(int a, int b)
{
  int div = a / b;
  return div;
}

int main()
{
  int a, b;
  // int min = -10;
  int min = -11;
  int max = 30;
  int somme = 0;
  for(a = min; a < max - 5; a++)
  {
    for(b = min + 2; b < max; b++)
    {
      somme += division(a, b);
      b++;
    }
  }

  return somme;
}
