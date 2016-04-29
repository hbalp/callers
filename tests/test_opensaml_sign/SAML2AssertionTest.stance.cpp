/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

// #include <cxxtest/TestSuite.h>
// #include <cxxtest/RealDescriptions.h>

//#include "samltest/signature/SAMLSignatureTestBase.h"
#include "saml/signature/SignatureProfileValidator.h"

#include <xmltooling/base.h>
#include <xmltooling/Lockable.h>
#include <xmltooling/XMLToolingConfig.h>

#include "xmltooling/util/ParserPool.h"

#include "xmltooling/signature/KeyInfo.h"
#include "xmltooling/signature/Signature.h"
#include "xmltooling/signature/SignatureValidator.h"

#include "xmltooling/security/Credential.h"
#include "xmltooling/security/CredentialCriteria.h"
#include "xmltooling/security/CredentialResolver.h"

#include <xercesc/dom/DOMDocument.hpp>

#include <saml/SAMLConfig.h> 
#include <saml2/core/Assertions.h>

#include <fstream>
#include <sstream>
#include <stdio.h>
//#include <memory>

XERCES_CPP_NAMESPACE_USE
using namespace opensaml::saml2;
using namespace xmltooling;
using namespace xmlsignature;
using namespace xercesc;
using namespace std;

extern string data_path;

//void initSAMLTestConfig();

    /** Location of file containing a single element with NO optional attributes */
    string singleElementFile;

    /** Location of file containing a single element with all optional attributes */
    string singleElementOptionalAttributesFile;

    /** Location of file containing a single element with child elements */
    string childElementsFile;

    /** The expected result of a marshalled single element with no optional attributes */
    DOMDocument* expectedDOM;

    /** The expected result of a marshalled single element with all optional attributes */
    DOMDocument* expectedOptionalAttributesDOM;

    /** The expected result of a marshalled single element with child elements */
    DOMDocument* expectedChildElementsDOM;

    /**
     * Unmarshalls an element file into its SAML XMLObject.
     * 
     * @return the SAML XMLObject from the file
     */
    XMLObject* unmarshallElement(string elementFile) {
        try {
            ParserPool& p=XMLToolingConfig::getConfig().getParser();
            ifstream fs(elementFile.c_str());
            DOMDocument* doc = p.parse(fs);
            const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
            return b->buildFromDocument(doc);
        }
        catch (XMLToolingException& e) {
            //TS_TRACE(typeid(e).name());
            //TS_TRACE(e.what());
            throw;
        }
    }

// class SAML2AssertionTest : public CxxTest::TestSuite, public SAMLSignatureTestBase {
// public:
//     void setUp() {
//         childElementsFile  = data_path + "signature/SAML2Assertion.xml";
//         SAMLSignatureTestBase::setUp();
//     }

//     void tearDown() {
//         SAMLSignatureTestBase::tearDown();
//     }

//     void testSignature() {

void assertEquals(const char* failMessage, DOMDocument* expectedDOM, XMLObject* xmlObject, bool canMarshall=true) {

  DOMElement* generatedDOM = xmlObject->getDOM();
  if (!generatedDOM) {
    if (!canMarshall) {
      //TSM_ASSERT("DOM not available", false);
      printf("DOM not available\n");
      exit(-1);
    }
    else {
      generatedDOM = xmlObject->marshall();
    }
  }
  if (!generatedDOM->isEqualNode(expectedDOM->getDocumentElement())) {
    string buf;
    XMLHelper::serialize(generatedDOM, buf);
    //TS_TRACE(buf.c_str());
    printf("\nPrint generated DOM\n");
    printf("%s", buf.c_str());
    
    buf.erase();
    XMLHelper::serialize(expectedDOM->getDocumentElement(), buf);
    //TS_TRACE(buf.c_str());
    printf("\nPrint expected DOM\n");
    printf("%s", buf.c_str());

    //TSM_ASSERT(failMessage, false);
    printf("\nTSM_ASSERT TBC...\n");
  }
}

int main()
{
  string data_path = "../samltest/data/";
  string config;
  string childElementsFile = data_path + "signature/SAML2Assertion.xml";
  CredentialResolver* m_resolver=nullptr;

  //initSAMLTestConfig();
  if (!opensaml::SAMLConfig::getConfig().init())
    {
      exit(17);
    }

  ParserPool& p=XMLToolingConfig::getConfig().getParser();

  if (!singleElementFile.empty()) {
    ifstream fs(singleElementFile.c_str());
    expectedDOM = p.parse(fs);
  }

  if (!singleElementOptionalAttributesFile.empty()) {
    ifstream fs(singleElementOptionalAttributesFile.c_str());
    expectedOptionalAttributesDOM = p.parse(fs);
  }

  if (!childElementsFile.empty()) {
    ifstream fs(childElementsFile.c_str());
    expectedChildElementsDOM = p.parse(fs);
  }

  config = data_path + "FilesystemCredentialResolver.xml";
  ifstream in(config.c_str());
  DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
  XercesJanitor<DOMDocument> janitor(doc);
  m_resolver = XMLToolingConfig::getConfig()
    .CredentialResolverManager
    .newPlugin(
               FILESYSTEM_CREDENTIAL_RESOLVER,
               doc->getDocumentElement()
               );

  auto_ptr_XMLCh issuer("issuer");
  auto_ptr_XMLCh issueInstant("1970-01-02T01:01:02.100Z");
  auto_ptr_XMLCh id("ident");
  auto_ptr_XMLCh method("method");
  auto_ptr_XMLCh nameid("John Doe");
        
  Issuer* is=IssuerBuilder::buildIssuer();
  is->setName(issuer.get());

  NameID* n=NameIDBuilder::buildNameID();
  n->setName(nameid.get());        
  Subject* subject=SubjectBuilder::buildSubject();
  subject->setNameID(n);

  AuthnStatement* statement=AuthnStatementBuilder::buildAuthnStatement();
  statement->setAuthnInstant(issueInstant.get());

  AuthnContext* ac=AuthnContextBuilder::buildAuthnContext();
  AuthnContextClassRef* acc=AuthnContextClassRefBuilder::buildAuthnContextClassRef();
  acc->setReference(method.get());
  ac->setAuthnContextClassRef(acc);
  statement->setAuthnContext(ac);
        
  auto_ptr<Assertion> assertion(AssertionBuilder::buildAssertion());
  assertion->setID(id.get());
  assertion->setIssueInstant(issueInstant.get());
  assertion->setIssuer(is);
  assertion->setSubject(subject);
  assertion->getAuthnStatements().push_back(statement);

  // Append a Signature.
  Signature* sig=SignatureBuilder::buildSignature();
  assertion->setSignature(sig);

  // Sign while marshalling.
  vector<Signature*> sigs(1,sig);
  CredentialCriteria cc;
  cc.setUsage(Credential::SIGNING_CREDENTIAL);

  ifstream in1(config.c_str());
  DOMDocument* doc1=XMLToolingConfig::getConfig().getParser().parse(in1);
  XercesJanitor<DOMDocument> janitor1(doc1);

  m_resolver = XMLToolingConfig::getConfig().CredentialResolverManager.newPlugin(FILESYSTEM_CREDENTIAL_RESOLVER,doc1->getDocumentElement());
  
  Locker locker(m_resolver);
  const Credential* cred = m_resolver->resolve(&cc);
  //TSM_ASSERT("Retrieved credential was null", cred!=nullptr);
  if(cred == NULL)
    {
      printf("\nRetrieved credential was null !");
      exit(3);
    };

  DOMElement* rootElement = nullptr;
  try {
    rootElement=assertion->marshall((DOMDocument*)nullptr,&sigs,cred);
  }
  catch (XMLToolingException& e) {
    //TS_TRACE(e.what());
    printf("\nTEST Exception: %s\n", e.what());
    throw;
  }
        
  string buf;
  XMLHelper::serialize(rootElement, buf);
  istringstream in2(buf);
  DOMDocument* doc2=XMLToolingConfig::getConfig().getParser().parse(in2);
  const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc2->getDocumentElement());
  
  if (!childElementsFile.empty()) {
    ParserPool& p=XMLToolingConfig::getConfig().getParser();
    ifstream fs(childElementsFile.c_str());
    expectedChildElementsDOM = p.parse(fs);
  }
  
  auto_ptr<XMLObject> assertion2(b->buildFromDocument(doc2));
  assertEquals("Unmarshalled assertion does not match", expectedChildElementsDOM, assertion2.get(), false);
        
  try {
    opensaml::SignatureProfileValidator spv;
    SignatureValidator sv(cred);
    spv.validate(assertion->getSignature());
    sv.validate(assertion->getSignature());
    // spv.validate(dynamic_cast<Assertion*>(assertion2.get())->getSignature());
    // sv.validate(dynamic_cast<Assertion*>(assertion2.get())->getSignature());
  }
  catch (XMLToolingException& e) {
    //TS_TRACE(e.what());
    printf("\nTEST Exception: %s\n", e.what());
    throw;
  }
};
