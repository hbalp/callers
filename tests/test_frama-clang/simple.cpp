
#include <stdio.h>
#include <iostream>
#include <cstdio>

#include "a.h"

int main(int argc, char** argv)
{
  bool oui = true;
  if ( oui == true )
    printf("coucou bool !!!\n");
  else
    printf("rejected bool !!!\n");    
  a(2);
  return 0;
}
