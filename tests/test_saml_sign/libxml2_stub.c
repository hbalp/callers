
/*
 * COMPAT using xml-config --cflags to get the include path this will work with both
 */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

int
xmlStrcmp (const xmlChar *str1,
	   const xmlChar *str2)
{
  return 0;
}


xmlNodePtr
xmlDocGetRootElement (const xmlDoc *doc)
{
  xmlNodePtr node;
  return node;
}

xmlNodePtr
getElementByID(xmlNodePtr node, xmlChar* id)
{
  xmlNodePtr elementById;
  return elementById;
}
