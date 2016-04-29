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

/*
 * This example should compile and run indifferently with libxml-1.8.8 +
 * and libxml2-2.1.0 +
 * Check the COMPAT comments below
 */

/*
 * COMPAT using xml-config --cflags to get the include path this will
 * work with both 
 */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define DEBUG(x) printf(x)

/*
 * A signature record
 * an xmlChar * is really an UTF8 encoded char string (0 terminated)
 */
typedef struct signature {
    xmlChar *name;
    xmlChar *issueInstant;
    xmlChar *company;
    xmlChar *organisation;
    xmlChar *smail;
    xmlChar *webPage;
    xmlChar *phone;
} signature, *signaturePtr;

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

    /* We don't care what the top level element name is */
    /* COMPAT xmlChildrenNode is a macro unifying libxml1 and libxml2 names */
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"NameID")) &&
	    (cur->ns == ns))
	    ret->name = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"issueInstant")) &&
	    (cur->ns == ns))
	    ret->issueInstant = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	cur = cur->next;
    }

    return(ret);
}

/*
 * and to print it
 */
static void
printNameID(signaturePtr cur) {
    if (cur == NULL) return;
    printf("------ NameID\n");
    if (cur->name) printf("	name: %s\n", cur->name);
    if (cur->issueInstant) printf("	issueInstant: %s\n", cur->issueInstant);
    if (cur->company) printf("	company: %s\n", cur->company);
    if (cur->organisation) printf("	organisation: %s\n", cur->organisation);
    if (cur->smail) printf("	smail: %s\n", cur->smail);
    if (cur->webPage) printf("	Web: %s\n", cur->webPage);
    if (cur->phone) printf("	phone: %s\n", cur->phone);
    printf("------\n");
}

/*
 * a Description for a Assertion
 */
typedef struct assertion {
    xmlChar *issuer;
    signaturePtr subject;
    xmlChar *authStmt;
    signaturePtr signature;
} assertion, *assertionPtr;

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

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Project")) &&
	    (cur->ns == ns)) {
	    ret->issuer = xmlGetProp(cur, (const xmlChar *) "ID");
	    if (ret->issuer == NULL) {
		fprintf(stderr, "Project has no ID\n");
	    }
	}

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Subject")) &&
	    (cur->ns == ns))
	    ret->subject = parseSignature(doc, ns, cur);

	cur = cur->next;
    }

    return(ret);
}

/*
 * and to print it
 */
static void
printAssertion(assertionPtr cur) {
    int i;

    if (cur == NULL) return;
    printf("=======  Assertion\n");
    if (cur->issuer != NULL) printf("issuer: %s\n", cur->issuer);
    if (cur->subject != NULL) printNameID(cur->subject);
    if (cur->authStmt != NULL) printf("authStmt: %s\n", cur->authStmt);
    if (cur->signature != NULL) printNameID(cur->signature);
    printf("======= \n");
}

/*
 * a Description for a SAML Response
 */
typedef struct samlResponse {
    // extension
    int nbAssertions;
    assertionPtr assertions[500]; /* using dynamic alloc is left as an exercise */
} samlAssertion, *samlAssertionPtr;

static samlAssertionPtr
parseSamlResponseFile(char *filename) {
    xmlDocPtr doc;
    samlAssertionPtr ret;
    assertionPtr curassertion;
    xmlNsPtr ns_saml, ns_samlp, ns_dsig;
    xmlNodePtr cur;

#ifdef LIBXML_SAX1_ENABLED
    /*
     * build an XML tree from a the file;
     */
    doc = xmlParseFile(filename);
    if (doc == NULL) assert(0);
#else
    /*
     * the library has been compiled without some of the old interfaces
     */
    assert(0);
#endif /* LIBXML_SAX1_ENABLED */

    /*
     * Check the document is of the right kind
     */
    cur = xmlDocGetRootElement(doc);
    if (cur == NULL) {
        fprintf(stderr,"empty document\n");
	xmlFreeDoc(doc);
	assert(0);
    }

    if (!xmlStrcmp(cur->name, (const xmlChar *) "Response"))
    {
        ns_samlp = xmlSearchNsByHref(doc, cur,
                                    (const xmlChar *) "urn:oasis:names:tc:SAML:2.0:protocol");
        if (ns_samlp == NULL) {
            fprintf(stderr,
                    "document of the wrong type, samlp:Response namespace not found\n");
            xmlFreeDoc(doc);
            assert(0);
        }
    }
    else
        assert(0);

    /*
     * Allocate the structure to be returned.
     */
    ret = (samlAssertionPtr) malloc(sizeof(samlAssertion));
    if (ret == NULL) {
        fprintf(stderr,"out of memory\n");
	xmlFreeDoc(doc);
	assert(0);
    }
    memset(ret, 0, sizeof(samlAssertion));

    /*
     * Now, walk the tree.
     */
    /* First level we expect just SAML Responses */
    cur = cur->xmlChildrenNode;

    ns_saml = xmlSearchNsByHref(doc, cur,
                           (const xmlChar *) "urn:oasis:names:tc:SAML:2.0:assertion");
    if (ns_saml == NULL) {
        fprintf(stderr,
	        "document of the wrong type, OASIS SAML Namespace not found\n");
	xmlFreeDoc(doc);
	assert(0);
    }
    if (xmlStrcmp(cur->name, (const xmlChar *) "Assertion")) {
        fprintf(stderr,"document of the wrong type, root node != Assertion");
	xmlFreeDoc(doc);
	assert(0);
    }

    while ( cur && xmlIsBlankNode ( cur ) ) {
	cur = cur -> next;
    }
    if ( cur == 0 ) {
	xmlFreeDoc(doc);
	free(ret);
	assert(0);
    }

    if ((xmlStrcmp(cur->name, (const xmlChar *) "Signature")) &&
        (xmlStrcmp(cur->name, (const xmlChar *) "Assertion")))
    {
        fprintf(stderr,"document of the wrong type, was '%s', Assertion expected",
		cur->name);
	fprintf(stderr,"xmlDocDump follows\n");
#ifdef LIBXML_OUTPUT_ENABLED
	xmlDocDump ( stderr, doc );
	fprintf(stderr,"xmlDocDump finished\n");
#endif /* LIBXML_OUTPUT_ENABLED */
	xmlFreeDoc(doc);
	free(ret);
	assert(0);
    }

    /* Second level is a list of Assertion, but be laxist */
    /* cur = cur->xmlChildrenNode; */
    /* while (cur != NULL) { */

	// cur = cur -> next;

        if ((!xmlStrcmp(cur->name, (const xmlChar *) "Assertion")) &&
	    (cur->ns == ns_saml)) {
	    curassertion = parseAssertion(doc, ns_saml, cur);
	    if (curassertion != NULL)
	        ret->assertions[ret->nbAssertions++] = curassertion;
            // if (ret->nbAssertions >= 500) break;
	}

    /*     cur = cur->next; */
    /* } */

    return(ret);
}

static void
handleSamlResponse(samlAssertionPtr cur) {
    int i;

    /*
     * Do whatever you want and free the structure.
     */
    printf("%d Assertion(s) registered\n", cur->nbAssertions);
    for (i = 0; i < cur->nbAssertions; i++) printAssertion(cur->assertions[i]);
}

int main(int argc, char **argv) {
    int i;
    samlAssertionPtr cur;

    /* COMPAT: Do not genrate nodes for formatting spaces */
    LIBXML_TEST_VERSION
    xmlKeepBlanksDefault(0);

    for (i = 1; i < argc ; i++) {
	cur = parseSamlResponseFile(argv[i]);
	if ( cur )
	  handleSamlResponse(cur);
	else
	  fprintf( stderr, "Error parsing file '%s'\n", argv[i]);
    }

    /* Clean up everything else before quitting. */
    xmlCleanupParser();

    return(0);
}
