#ifndef _VALIDATE_SAML_SIGN_H_
#define _VALIDATE_SAML_SIGN_H_

bool
saml_SignatureProfileValidator_validate(signaturePtr sign, xmlDocPtr doc);

xmlNodePtr
getElementByID(xmlNodePtr node, xmlChar* id);

#endif
