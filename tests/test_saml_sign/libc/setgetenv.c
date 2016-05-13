#include <string.h>

//static char* YES="yes";
static char* YES="yes";

int setenv(const char *name, const char *value, int overwrite)
{
  return 0;
}

char *getenv(const char *name)
{
  if (strcmp(name, "XML_MEM_BREAKPOINT") == 0)
    return strdup(YES);
  if (strcmp(name, "XML_MEM_TRACE") == 0)
    return strdup(YES);
  return (char*) 0;
}
