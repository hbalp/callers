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
 * COMPAT using xml-config --cflags to get the include path this will work with both
 */
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "parse_saml_response.h"
#include "validate_saml_sign.h"

/********************************************************************************/
/*                                    main                                      */
/********************************************************************************/
/* unitary test verifying parsing of SAML Response messages and SAML signature validation */

int main(int argc, char **argv) {
    int i;
    samlResponsePtr cur;

    /* COMPAT: Do not generate nodes for formatting spaces */
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
