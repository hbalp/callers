/*
 * unittest_saml_sign.c: unit test of SAML signature validation function
 * copyright Thales Communications & Security 2013 to 2016
 * IST STANCE project
 * hugues.balp@thalesgroup.com
 */

#include "validate_saml_sign.h"

/********************************************************************************/
/*                                    main                                      */
/********************************************************************************/
/* unitary test verifying SAML signature validation */

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
