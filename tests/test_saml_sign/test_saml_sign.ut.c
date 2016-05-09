/*
 * unittest_saml_sign.c: unit test of SAML signature validation function
 * copyright Thales Communications & Security 2013 to 2016
 * IST STANCE project
 * hugues.balp@thalesgroup.com
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * COMPAT using xml-config --cflags to get the include path this will work with both
 */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "parse_saml_response.h"
#include "validate_saml_sign.h"

/********************************************************************************/
/*                                    main                                      */
/********************************************************************************/
/* unitary test verifying SAML signature validation */

static xmlNode rootElement;
static xmlNode validAssertion;
static xmlNode maliciousAssertion;

//bool maliciousSAMLResponse; // = true;
int maliciousSAMLResponse; // 0: false , else : true;

xmlNodePtr
xmlDocGetRootElement (const xmlDoc *doc)
{
  return &rootElement;
}

int
xmlStrcmp (const xmlChar *str1,
	   const xmlChar *str2)
{
  return 0;
}

xmlNodePtr
getElementByID(xmlNodePtr node, xmlChar* id)
{
  xmlNodePtr elementByID;  
  if(maliciousSAMLResponse != 0)
    {
      elementByID = &maliciousAssertion;
    }
  else
    {
      elementByID = &validAssertion;
    }
  return elementByID;
}

bool ut_saml_SignatureProfileValidator_validate(int under_XSW_attack)
{
  xmlNodePtr parent = &validAssertion;
  signature sign;
  xmlDoc doc;
  parent->name = "Assertion";
  sign.parent = parent;

  maliciousSAMLResponse = under_XSW_attack;
  
  bool is_valid = saml_SignatureProfileValidator_validate(&sign, &doc);
  if(is_valid)
    printf("TRUE\n");
  else
    printf("FALSE\n");
  return(is_valid);
}

int main()
{
  bool status = true;
  int maliciousSAMLResponse = 0;
  status = status && ut_saml_SignatureProfileValidator_validate(0);
  status = status && ut_saml_SignatureProfileValidator_validate(1);
  return(status);
}
