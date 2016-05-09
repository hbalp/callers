#ifndef _VALIDATE_SAML_SIGN_H_
#define _VALIDATE_SAML_SIGN_H_

/*
 * test_saml_sign.c: libxml c parsing of openSAML assertions
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
 * This example should compile and run indifferently with libxml-1.8.8 +
 * and libxml2-2.1.0 +
 * Check the COMPAT comments below
 */

/*
 * COMPAT using xml-config --cflags to get the include path this will work with both
 */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define DEBUG(x) /* printf(x) */

/********************************************************************************/
/*                                Signature                                     */
/********************************************************************************/

/*
 * A signature record
 * an xmlChar * is really an UTF8 encoded char string (0 terminated)
 */
typedef struct dsigReference {
    xmlChar *URI;
} dsigReference;

typedef struct signedInfo {
    dsigReference reference;
} dsigSignedInfo;

typedef struct signature {
    xmlChar *value;
    dsigSignedInfo signedInfo;
    xmlNodePtr parent;
} signature, *signaturePtr;

/*
 * And the code needed to parse it
 */
static signaturePtr parseSignature(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

/*
 * and to print it
 */
static void printSignature(signaturePtr cur);

/********************************************************************************/
/*                                Subject                                       */
/********************************************************************************/

/*
 * a Description for a Subject
 */
typedef struct subject {
    xmlChar *nameID;
} subject, *subjectPtr;

/*
 * And the code needed to parse it
 */
static subjectPtr parseSubject(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);
/*
 * and to print it
 */
static void printSubject(subjectPtr subject);

/********************************************************************************/
/*                              SAML Assertion                                  */
/********************************************************************************/

/*
 * a Description for a Assertion
 */
typedef struct assertion {
    xmlChar *issuer;
    xmlChar *id;
    subjectPtr subject;
    xmlChar *authStmt;
    signaturePtr signature;
} assertion, *assertionPtr;

/*
 * And the code needed to parse the assertion
 */
static assertionPtr
parseAssertion(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur);

static void printAssertion(assertionPtr cur);

/********************************************************************************/
/*                              SAML Response                                   */
/********************************************************************************/

/*
 * a Description for a SAML Response
 */
typedef struct samlResponse {
    xmlChar *issuer;
    int nbAssertions;
    assertionPtr assertions[2];
    // extension
    xmlDocPtr doc;
} samlResponse, *samlResponsePtr;

static samlResponsePtr parseSamlResponseFile(char *filename);

static void printSamlResponse(samlResponsePtr cur);

bool saml_SignatureProfileValidator_validate(signaturePtr sign, xmlDocPtr doc);

bool checkSamlAssertion(assertionPtr assertion, xmlDocPtr doc);

bool checkSamlResponse(samlResponsePtr response);

static void
handleSamlResponse(samlResponsePtr response);

#endif
