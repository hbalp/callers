#ifndef _VALIDATE_SAML_SIGN_H_
#define _VALIDATE_SAML_SIGN_H_

#define ADAPTED_CALL_CONTEXT OFF

bool
saml_SignatureProfileValidator_validate(signaturePtr sign, xmlDocPtr doc);

xmlNodePtr
getElementByID(xmlNodePtr node, xmlChar* id);

#endif
