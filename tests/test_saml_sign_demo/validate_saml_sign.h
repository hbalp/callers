#ifndef _VALIDATE_SAML_SIGN_H_
#define _VALIDATE_SAML_SIGN_H_

#define ON  1
#define OFF 0

#define USE_XSW_COUNTERMEASURE  ON
#define ADAPTED_CALL_CONTEXT    OFF
#define USE_XML_MEM_TRACE       OFF
#define USE_XML_MEM_BREAKPOINT  OFF
#define FRAMA_C_VA_WIDENING     OFF

bool
saml_SignatureProfileValidator_validate(signaturePtr sign, xmlDocPtr doc);

xmlNodePtr
getElementByID(xmlNodePtr node, xmlChar* id);

#endif
