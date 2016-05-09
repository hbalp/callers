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

xmlNodePtr
xmlDocGetRootElement (const xmlDoc *doc)
{}

int
xmlStrcmp (const xmlChar *str1,
	   const xmlChar *str2)
{}

xmlNodePtr
getElementByID(xmlNodePtr node, xmlChar* id)
{}

bool ut1_saml_SignatureProfileValidator_validate()
{
  signaturePtr sign;
  xmlDocPtr doc;
  bool is_valid = saml_SignatureProfileValidator_validate(sign, doc);
  return(is_valid);
}

bool ut2_saml_SignatureProfileValidator_validate()
{
  signaturePtr sign;
  xmlDocPtr doc;
  bool is_valid = saml_SignatureProfileValidator_validate(sign, doc);
  return(is_valid);
}

int main()
{
  bool status = true;
  status = status && ut1_saml_SignatureProfileValidator_validate();
  status = status && ut2_saml_SignatureProfileValidator_validate();
  return(status);
}
