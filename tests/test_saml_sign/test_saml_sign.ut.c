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

#define ASSERTION_ID "123";

bool maliciousSAMLResponse; // 0: false , 1 : true;

bool ut_saml_SignatureProfileValidator_validate(bool under_XSW_attack)
{
  // build the valid assertion
  xmlNode validAssertion;
  xmlNode maliciousAssertion;
  signature sign;

#if ADAPTED_CALL_CONTEXT
  xmlNode validAssertionIDval;
  validAssertionIDval.type = XML_TEXT_NODE;
  validAssertionIDval.content = ASSERTION_ID;
  validAssertionIDval.next = NULL;
  
  xmlAttr validAssertionIDattr;
  validAssertionIDattr.type = XML_ATTRIBUTE_NODE;
  validAssertionIDattr.name = "ID";
  validAssertionIDattr.children = &validAssertionIDval;

  validAssertion.type = XML_ELEMENT_NODE;
  validAssertion.name = "Assertion";
  validAssertion.properties = &validAssertionIDattr;

  xmlNodePtr parent = &validAssertion;
  sign.parent = parent;
  sign.signedInfo.reference.URI = ASSERTION_ID;

  // build the malicious assertion
  xmlNode maliciousAssertionIDval;
  maliciousAssertionIDval.type = XML_TEXT_NODE;
#ifdef FRAMA_C
#if FRAMA_C_VA_WIDENING
  //#include "__fc_builtin.h"
  void *Frama_C_alloc_size(size_t size);
  char Frama_C_char_interval(char min, char max);
  int i;
  int lengthId = Frama_C_interval(2, 20);
  maliciousAssertionIDval.content = Frama_C_alloc_size(lengthId); // malloc
  for (i = 0; i < lengthId-1; ++i)
    maliciousAssertionIDval.content[i] = Frama_C_char_interval(CHAR_MIN, CHAR_MAX);
  maliciousAssertionIDval.content[lengthId-1] = '\0';
#else
  maliciousAssertionIDval.content = ASSERTION_ID;  
#endif
#endif
  maliciousAssertionIDval.next = NULL;
  
  xmlAttr maliciousAssertionIDattr;
  maliciousAssertionIDattr.type = XML_ATTRIBUTE_NODE;
  maliciousAssertionIDattr.name = "ID";
  maliciousAssertionIDattr.children = &maliciousAssertionIDval;

  maliciousAssertion.type = XML_ELEMENT_NODE;
  maliciousAssertion.name = "Assertion";
  maliciousAssertion.properties = &maliciousAssertionIDattr;

#if USE_XML_MEM_TRACE
  setenv("XML_MEM_TRACE", "yes", 1);
#endif

#if USE_XML_MEM_BREAKPOINT
  setenv("XML_MEM_BREAKPOINT", "yes", 1);
#endif

#else
  printf("WARNING: unverified call context !\nIt is malformed and will probably crash !\n");
#endif

  xmlDoc doc;
  doc.intSubset = NULL;

  maliciousSAMLResponse = under_XSW_attack;
  if(maliciousSAMLResponse == false)
    {
      doc.children = &validAssertion;
    }
  else
    {
      doc.children = &maliciousAssertion;
    }
    
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
  status = status && ut_saml_SignatureProfileValidator_validate(false);
  status = status && ut_saml_SignatureProfileValidator_validate(true);
  return(status);
}
