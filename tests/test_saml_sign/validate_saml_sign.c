/*
 * test_saml_sign.c: libxml c parsing of openSAML assertions
 * copyright Thales Communications & Security 2013 to 2016
 * IST STANCE project
 * hugues.balp@thalesgroup.com
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
 * COMPAT using xml-config --cflags to get the include path this will work with both
 */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "parse_saml_response.h"
#include "validate_saml_sign.h"

/********************************************************************************/
/*                  Validate SAML Signature                                     */
/********************************************************************************/

bool saml_SignatureProfileValidator_validate(signaturePtr sign, xmlDocPtr doc)
{
    assert(doc != NULL);
    if(sign == NULL)
    {
      return false;
    }
    bool is_valid = false;
    xmlNodePtr root = xmlDocGetRootElement(doc);

    printf("=======  Validate SAML signature: \n");

    // Ensure that the signature parent node is well a SAML Assertion
    if (xmlStrcmp(sign->parent->name, (const xmlChar *) "Assertion"))
    {
      return false;
    }

    // Get all XML elements with same ID as the one pointed to by the signature URI
    // xmlListPtr refs = xmlGetRefs(doc, sign->signedInfo.reference.URI);
    xmlNodePtr assertionByID = getElementByID(root, sign->signedInfo.reference.URI);
    if(assertionByID == NULL)
    {
      return false;
    }

    // Countermeasure against XSW attacks
    // Check wether signature parent node is same as assertion by ID
    if(sign->parent == assertionByID)
    {
      is_valid = true;
    }
    return is_valid;
}

/* xmlNodePtr */
/* getElementByID(xmlNodePtr node, xmlChar* id) */
/* { */
/*   xmlNodePtr elementById; */
/*   return elementById; */
/* } */

xmlNodePtr
getElementByID(xmlNodePtr node, xmlChar* id)
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
