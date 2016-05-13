#include <string.h>

int setenv(const char *name, const char *value, int overwrite)
{
  return 0;
}

char *getenv(const char *name)
{ if (strcmp(name, "XML_MEM_BREAKPOINT") == 0)
    return "yes";
  if (strcmp(name, "XML_MEM_TRACE") == 0)
    return "yes";
  return (char*) 0;
}
