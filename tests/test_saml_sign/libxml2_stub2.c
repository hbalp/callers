
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
getElementByID(xmlNodePtr node, xmlChar* id)
{
  xmlNodePtr elementById;
  return elementById;
}

xmlNodePtr getElementByID(xmlNodePtr node, xmlChar* id)
{
  xmlChar* curid = xmlGetProp(node, (const xmlChar *)"ID");

  // printf("getElementByID:DEBUG: curid=%s, id=%s\n", curid, id);

  if ((curid != NULL)&&(!xmlStrcmp(curid, (const xmlChar *) id)))
  {
    return node;
  }

  xmlNodePtr ch = node->xmlChildrenNode;
  while (ch != NULL) {
      xmlNodePtr result = getElementByID(ch, id);
      if(result != NULL)
      {
        return result;
      }
      ch = ch->next;
  }
  return NULL;
}

/**
 * xmlDocGetRootElement:
 * @doc:  the document
 *
 * Get the root element of the document (doc->children is a list
 * containing possibly comments, PIs, etc ...).
 *
 * Returns the #xmlNodePtr for the root or NULL
 */
xmlNodePtr
xmlDocGetRootElement(const xmlDoc *doc) {
    xmlNodePtr ret;

    if (doc == NULL) return(NULL);
    ret = doc->children;
    while (ret != NULL) {
	if (ret->type == XML_ELEMENT_NODE)
	    return(ret);
        ret = ret->next;
    }
    return(ret);
}
