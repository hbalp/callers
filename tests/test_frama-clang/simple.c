
#include <stdbool.h>
#include <stdio.h>
//#include <iostream>
//#include <cstdio>

#include "a.h"

int main(int argc, char** argv)
{
  _Bool oui = true;
  if ( oui == true )
    printf("coucou bool !!!\n");
  else
    printf("rejected bool !!!\n");    
  a(1);
  return 0;
}
