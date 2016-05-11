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

#include "parse_saml_response.h"
#include "validate_saml_sign.h"

/********************************************************************************/
/*                                Signature                                     */
/********************************************************************************/

/*
 * And the code needed to parse it
 */
static signaturePtr
parseSignature(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
    signaturePtr ret = NULL;

    DEBUG("parseSignature\n");
    /*
     * allocate the struct
     */
    ret = (signaturePtr) malloc(sizeof(signature));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	assert(0);
    }
    memset(ret, 0, sizeof(signature));

    ret->parent = cur->parent;

    /* We don't care what the top level element name is */
    /* COMPAT xmlChildrenNode is a macro unifying libxml1 and libxml2 names */
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SignedInfo")) &&
	    (cur->ns == ns))
            {
              xmlNodePtr ch = cur->xmlChildrenNode;
              while (ch != NULL) {
                  if ((!xmlStrcmp(ch->name, (const xmlChar *)"Reference")) &&
                      (ch->ns == ns))
                      {
                        ret->signedInfo.reference.URI = xmlGetProp(ch, (const xmlChar *) "URI");
                      }
                  ch = ch->next;
              }
            }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"SignatureValue")) &&
	    (cur->ns == ns))
	    ret->value = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	cur = cur->next;
    }

    return(ret);
}

/*
 * and to print it
 */
static void
printSignature(signaturePtr cur) {
    if (cur == NULL) return;
    printf("- Signature\n");
    if (cur->signedInfo.reference.URI) printf("   URI: %s\n", cur->signedInfo.reference.URI);
    if (cur->value) printf("    value: %s\n", cur->value);
}

/********************************************************************************/
/*                                Subject                                       */
/********************************************************************************/

/*
 * And the code needed to parse it
 */
static subjectPtr
parseSubject(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
    subjectPtr ret = NULL;

    DEBUG("parseSubject\n");
    /*
     * allocate the struct
     */
    ret = (subjectPtr) malloc(sizeof(subject));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	assert(0);
    }
    memset(ret, 0, sizeof(subject));

    /* We don't care what the top level element name is */
    /* COMPAT xmlChildrenNode is a macro unifying libxml1 and libxml2 names */
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"NameID")) &&
	    (cur->ns == ns))
	    ret->nameID = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        // if ((!xmlStrcmp(cur->name, (const xmlChar *)"issueInstant")) &&
	//     (cur->ns == ns))
	//     ret->issueInstant = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	cur = cur->next;
    }

    return(ret);
}

/*
 * and to print it
 */
static void
printSubject(subjectPtr subject) {
    if (subject == NULL) return;
    printf("- Subject ");
    if (subject->nameID) printf("NameID: %s\n", subject->nameID);
}

/********************************************************************************/
/*                              SAML Assertion                                  */
/********************************************************************************/

/*
 * And the code needed to parse the assertion
 */
static assertionPtr
parseAssertion(xmlDocPtr doc, xmlNsPtr ns, xmlNodePtr cur) {
    assertionPtr ret = NULL;
    xmlNsPtr ns_dsig;
    signaturePtr curSign;

    DEBUG("parseAssertion\n");

    /*
     * allocate the struct
     */
    ret = (assertionPtr) malloc(sizeof(assertion));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	assert(0);
    }
    memset(ret, 0, sizeof(assertion));

    ret->id = xmlGetProp(cur, (const xmlChar *) "ID");
    if (ret->id == NULL) {
        fprintf(stderr, "Assertion has no ID !\n");
    }

    /* We don't care what the top level element name is */
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if (!xmlStrcmp(cur->name, (const xmlChar *) "Signature"))
        {
            ns_dsig = xmlSearchNsByHref(doc, cur,
                                      (const xmlChar *) "http://www.w3.org/2000/09/xmldsig#");
            if (ns_dsig == NULL) {
                fprintf(stderr,
                        "document of the wrong type, xmldsig Namespace not found\n");
                xmlFreeDoc(doc);
                assert(0);
            }

            if (cur->ns == ns_dsig) {
                curSign = parseSignature(doc, ns_dsig, cur);
                if (curSign != NULL)
                    ret->signature = curSign;
                // if (ret->nbAssertions >= 500) break;
            }

        }

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Issuer")) &&
	    (cur->ns == ns))
	    ret->issuer = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Subject")) &&
	    (cur->ns == ns))
	    ret->subject = parseSubject(doc, ns, cur);

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "AuthnStatement")) &&
	    (cur->ns == ns))
	    ret->authStmt = xmlGetProp(cur, (const xmlChar *) "AuthnInstant");

	cur = cur->next;
    }

    return(ret);
}

static void
printAssertion(assertionPtr cur) {
    int i;

    if (cur == NULL) return;
    printf("=======  Assertion\n");
    if (cur->issuer != NULL) printf("- Issuer: %s\n", cur->issuer);
    if (cur->id != NULL) printf("- ID: %s\n", cur->id);
    if (cur->signature != NULL) printSignature(cur->signature);
    if (cur->subject != NULL) printSubject(cur->subject);
    if (cur->authStmt != NULL) printf("- AuthStmt: %s\n", cur->authStmt);
    printf("======= \n");
}

/********************************************************************************/
/*                              SAML Response                                   */
/********************************************************************************/

samlResponsePtr
parseSamlResponseFile(char *filename) {

    samlResponsePtr ret;
    assertionPtr curassertion;
    xmlNsPtr ns_saml, ns_samlp, ns_dsig;
    xmlNodePtr cur, ch;

    /*
     * Allocate the structure to be returned.
     */
    ret = (samlResponsePtr) malloc(sizeof(samlResponse));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	xmlFreeDoc(ret->doc);
	assert(0);
    }
    memset(ret, 0, sizeof(samlResponse));

#ifdef LIBXML_SAX1_ENABLED
    /*
     * build an XML tree from a the file;
     */
    ret->doc = xmlParseFile(filename);
    if (ret->doc == NULL) assert(0);
#else
    /*
     * the library has been compiled without some of the old interfaces
     */
    printf("ERROR: the libxml2 library has been compiled without some of the old interfaces\n");
    assert(0);
#endif /* LIBXML_SAX1_ENABLED */

    // Could be better
    //assert(ret->doc->refs != NULL);

    /*
     * Check the document is of the right kind
     */
    cur = xmlDocGetRootElement(ret->doc);
    if (cur == NULL) {
        fprintf(stderr,"empty document\n");
	xmlFreeDoc(ret->doc);
	assert(0);
    }

    if (!xmlStrcmp(cur->name, (const xmlChar *) "Response"))
    {
        ns_samlp = xmlSearchNsByHref(ret->doc, cur,
                                    (const xmlChar *) "urn:oasis:names:tc:SAML:2.0:protocol");
        if (ns_samlp == NULL) {
            fprintf(stderr,
                    "document of the wrong type, samlp:Response namespace not found\n");
            xmlFreeDoc(ret->doc);
            assert(0);
        }
    }
    else
        assert(0);

    /*
     * Now, walk the tree.
     */
    /* First level we expect just SAML Responses */
    cur = cur->xmlChildrenNode;

    ns_saml = xmlSearchNsByHref(ret->doc, cur,
                           (const xmlChar *) "urn:oasis:names:tc:SAML:2.0:assertion");
    if (ns_saml == NULL) {
        fprintf(stderr,
	        "document of the wrong type, OASIS SAML Namespace not found\n");
	xmlFreeDoc(ret->doc);
	assert(0);
    }
    if (xmlStrcmp(cur->name, (const xmlChar *) "Issuer")) {
        fprintf(stderr,"document of the wrong type, root node != Issuer");
	xmlFreeDoc(ret->doc);
	assert(0);
    }

    while ( cur && xmlIsBlankNode ( cur ) ) {
	cur = cur -> next;
    }
    if ( cur == 0 ) {
	xmlFreeDoc(ret->doc);
	free(ret);
	assert(0);
    }

    if ((xmlStrcmp(cur->name, (const xmlChar *) "Issuer")) &&
        (xmlStrcmp(cur->name, (const xmlChar *) "Extension")) &&
        (xmlStrcmp(cur->name, (const xmlChar *) "Assertion")))
    {
        fprintf(stderr,"document of the wrong type, was '%s', Assertion expected",
		cur->name);
	fprintf(stderr,"xmlDocDump follows\n");
#ifdef LIBXML_OUTPUT_ENABLED
	xmlDocDump ( stderr, ret->doc );
	fprintf(stderr,"xmlDocDump finished\n");
#endif /* LIBXML_OUTPUT_ENABLED */
	xmlFreeDoc(ret->doc);
	free(ret);
	assert(0);
    }

    /* Second level is a list of Assertion, but be laxist */
    /* cur = cur->xmlChildrenNode; */
    while (cur != NULL) {

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Issuer")) &&
	    (cur->ns == ns_saml))
	    ret->issuer = xmlNodeListGetString(ret->doc, cur->xmlChildrenNode, 1);

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Extensions")) &&
	    (cur->ns == ns_samlp))
        {
            ch = cur->xmlChildrenNode;
	    curassertion = parseAssertion(ret->doc, ns_saml, ch);
	    if (curassertion != NULL)
	        ret->assertions[ret->nbAssertions++] = curassertion;
            // if (ret->nbAssertions >= 500) break;
	}

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Assertion")) &&
	    (cur->ns == ns_saml)) {
	    curassertion = parseAssertion(ret->doc, ns_saml, cur);
	    if (curassertion != NULL)
	        ret->assertions[ret->nbAssertions++] = curassertion;
            // if (ret->nbAssertions >= 500) break;
	}

        cur = cur->next;
     }

    return(ret);
}

void
printSamlResponse(samlResponsePtr cur) {
    int i;
    printf("=======  SAML Response\n");

    if (cur->issuer != NULL) printf("- Issuer: %s\n", cur->issuer);

    printf("%d Assertion(s) registered\n", cur->nbAssertions);
    for (i = 0; i < cur->nbAssertions; i++) printAssertion(cur->assertions[i]);
}

// xmlNodePtr getElementByID(xmlNodePtr node, xmlChar* id)
// {
//   xmlChar* curid = xmlGetProp(node, (const xmlChar *)"ID");

//   // printf("getElementByID:DEBUG: curid=%s, id=%s\n", curid, id);

//   if ((curid != NULL)&&(!xmlStrcmp(curid, (const xmlChar *) id)))
//   {
//     return node;
//   }

//   xmlNodePtr ch = node->xmlChildrenNode;
//   while (ch != NULL) {
//       xmlNodePtr result = getElementByID(ch, id);
//       if(result != NULL)
//       {
//         return result;
//       }
//       ch = ch->next;
//   }
//   return NULL;
// }

/* bool saml_SignatureProfileValidator_validate(signaturePtr sign, xmlDocPtr doc) */
/* { */
/*     assert(doc != NULL); */
/*     if(sign == NULL) */
/*     { */
/*       return false; */
/*     } */
/*     bool is_valid = false; */
/*     xmlNodePtr root = xmlDocGetRootElement(doc); */

/*     printf("=======  Validate SAML signature: \n"); */

/*     // Ensure that the signature parent node is well a SAML Assertion */
/*     if (xmlStrcmp(sign->parent->name, (const xmlChar *) "Assertion")) */
/*     { */
/*       return false; */
/*     } */

/*     // Get all XML elements with same ID as the one pointed to by the signature URI */
/*     // xmlListPtr refs = xmlGetRefs(doc, sign->signedInfo.reference.URI); */
/*     xmlNodePtr assertionByID = getElementByID(root, sign->signedInfo.reference.URI); */
/*     if(assertionByID == NULL) */
/*     { */
/*       return false; */
/*     } */

/*     // Countermeasure against XSW attacks */
/*     // Check wether signature parent node is same as assertion by ID */
/*     if(sign->parent == assertionByID) */
/*     { */
/*       is_valid = true; */
/*     } */
/*     return is_valid; */
/* } */

bool checkSamlAssertion(assertionPtr assertion, xmlDocPtr doc)
{
    printf("=======  Check SAML Assertion\n");
    bool result = saml_SignatureProfileValidator_validate(assertion->signature, doc);
    if (result == false)
      printf("FALSE\n");
    else
    {
      printf("TRUE\n");
      // decode assertion
      printAssertion(assertion);
    }
    return result;
}

bool checkSamlResponse(samlResponsePtr response)
{
    printf("=======  Check SAML Response\n");
    bool result = false;
    int a;
    for(a = 0; !result &&  a < response->nbAssertions; a++)
    {
      assertionPtr assertion = response->assertions[a];
      result = checkSamlAssertion(assertion, response->doc);
    }
    return result;
}

void
handleSamlResponse(samlResponsePtr response) {
    int i;

    /*
     * Do whatever you want and free the structure.
     */
    printSamlResponse(response);

    checkSamlResponse(response);
}
