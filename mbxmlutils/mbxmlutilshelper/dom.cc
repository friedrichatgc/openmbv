#include <config.h>
#include "dom.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/scope_exit.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMTypeInfo.hpp>
#include <xercesc/dom/DOMProcessingInstruction.hpp>
#include <xercesc/dom/DOMComment.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMXPathNSResolver.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/framework/psvi/PSVIElement.hpp>
#include <xercesc/framework/psvi/PSVIAttributeList.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include "thislinelocation.h"
#include <fmatvec/toString.h>
#include <boost/spirit/include/qi.hpp>
#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

// we need some internal xerces classes (here the XMLScanner to get the current line number during parsing)
#include <xercesc/internal/XMLScanner.hpp>

using namespace std;
using namespace std::placeholders;
using namespace xercesc;
using namespace boost::filesystem;

namespace boost {

// convenience: convert e.g. "[3;7;7.9]" to std::vector<double>(3,7,7.9)
template<>
vector<double> lexical_cast(const string& str) {
  namespace qi = boost::spirit::qi;

  static qi::rule<string::const_iterator, vector<double>(), boost::spirit::qi::space_type> vec =
    '[' >> (qi::double_ % ';') >> ']' >> qi::eoi;

  vector<double> v;
  if(!qi::phrase_parse(str.begin(), str.end(), vec, qi::space, v))
    throw runtime_error("'"+str+"' does not contain a double vector.");
  return v;
}

// convenience: convert e.g. "[3,7;9,7.9]" to std::vector<std::vector<double> >
template<>
vector<vector<double>> lexical_cast(const string& str) {
  namespace qi = boost::spirit::qi;

  static qi::rule<string::const_iterator, vector<vector<double>>(), boost::spirit::qi::space_type> vec =
    '[' >> ((qi::double_ % ',') % ';') >> ']' >> qi::eoi;

  vector<vector<double>> v;
  if(!qi::phrase_parse(str.begin(), str.end(), vec, qi::space, v))
    throw runtime_error("'"+str+"' does not contain a double matrix.");
  return v;
}

// convenience: convert e.g. "[3;7;7.9]" to std::vector<int>(3,7,7.9)
template<>
vector<int> lexical_cast(const string& str) {
  namespace qi = boost::spirit::qi;

  static qi::rule<string::const_iterator, vector<int>(), boost::spirit::qi::space_type> vec =
    '[' >> (qi::int_ % ';') >> ']' >> qi::eoi;

  vector<int> v;
  if(!qi::phrase_parse(str.begin(), str.end(), vec, qi::space, v))
    throw runtime_error("'"+str+"' does not contain a int vector.");
  return v;
}


// convenience: convert e.g. "[3,7;9,7.9]" to std::vector<std::vector<int> >
template<>
vector<vector<int>> lexical_cast(const string& str) {
  namespace qi = boost::spirit::qi;

  static qi::rule<string::const_iterator, vector<vector<int>>(), boost::spirit::qi::space_type> vec =
    '[' >> ((qi::int_ % ',') % ';') >> ']' >> qi::eoi;

  vector<vector<int>> v;
  if(!qi::phrase_parse(str.begin(), str.end(), vec, qi::space, v))
    throw runtime_error("'"+str+"' does not contain a int matrix.");
  return v;
}

template<>
bool lexical_cast<bool>(const string& arg)
{
  string s(boost::trim_copy(arg));
  if(s=="0") return false;
  if(s=="1") return true;
  if(s=="false") return false;
  if(s=="true") return true;
  throw std::runtime_error("Input is not bool:\n"+arg);
}

}

namespace MBXMLUtils {

ThisLineLocation domLoc;

namespace {
  InitXerces initXerces;

  // START: ugly hack to call a protected/private method from outside
  // (from http://bloglitb.blogspot.de/2010/07/access-to-private-members-thats-easy.html)
  template<typename Tag>
  struct result {
    using type = typename Tag::type;
    static type ptr;
  };
  
  template<typename Tag>
  typename result<Tag>::type result<Tag>::ptr;
  
  template<typename Tag, typename Tag::type p>
  struct rob : result<Tag> {
    struct filler {
      filler() { result<Tag>::ptr = p; }
    };
    static filler filler_obj;
  };
  
  template<typename Tag, typename Tag::type p>
  typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;
  // END: ugly hack to call a protected/private method from outside
  // (from http://bloglitb.blogspot.de/2010/07/access-to-private-members-thats-easy.html)

  path toRelativePath(path absPath, const path& relTo=current_path()) {
    if(!absPath.is_absolute())
      throw runtime_error("First argument of toRelativePath must be a absolute path.");
    path::iterator curIt, absIt;
    auto toLower=[](std::string p) {
#ifdef _WIN32 // Windows filesystem is case-insensitive -> compare lower case names
      transform(p.begin(), p.end(), p.begin(), ::tolower);
#endif
      return p;
    };
    for(curIt=relTo.begin(), absIt=absPath.begin();
        curIt!=relTo.end() && toLower(curIt->string())==toLower(absIt->string());
        ++curIt, ++absIt);
    if(curIt==relTo.end()) {
      path relPathRet;
      for(; absIt!=absPath.end(); ++absIt)
        relPathRet/=*absIt;
      return relPathRet;
    }
    return absPath;
  }
}

bool DOMErrorPrinter::handleError(const DOMError& e)
{
  string type;
  switch(e.getSeverity()) {
    case DOMError::DOM_SEVERITY_WARNING:     type="Warning";     break; // we handle warnings as errors
    case DOMError::DOM_SEVERITY_ERROR:       type="Error";       break;
    case DOMError::DOM_SEVERITY_FATAL_ERROR: type="Fatal error"; break;
  }
  // save the error
  errorSet=true;
  auto* loc=e.getLocation();
  if(loc)
    error=DOMEvalException(type+": "+X()%e.getMessage(), *loc);
  else
    error=DOMEvalException(type+": "+X()%e.getMessage(), nullptr);
  return false; // do not continue parsing
}

template<> const DOMElement *DOMElementWrapper<      DOMElement>::dummyArg=nullptr;
template<> const DOMElement *DOMElementWrapper<const DOMElement>::dummyArg=nullptr;

template<typename DOMElementType>
const DOMElement *DOMElementWrapper<DOMElementType>::getFirstElementChildNamed(const FQN &name) const {
  for(DOMElement *ret=me->getFirstElementChild(); ret; ret=ret->getNextElementSibling())
    if(E(ret)->getTagName()==name)
      return ret;
  return nullptr;
};
template const DOMElement *DOMElementWrapper<const DOMElement>::getFirstElementChildNamed(const FQN &name) const; // explicit instantiate const variant

template<typename DOMElementType>
DOMElement *DOMElementWrapper<DOMElementType>::getFirstElementChildNamed(const FQN &name) {
  for(DOMElement *ret=me->getFirstElementChild(); ret; ret=ret->getNextElementSibling())
    if(E(ret)->getTagName()==name)
      return ret;
  return nullptr;
};

template<typename DOMElementType>
const DOMElement *DOMElementWrapper<DOMElementType>::getNextElementSiblingNamed(const FQN &name) const {
  for(DOMElement *ret=me->getNextElementSibling(); ret; ret=ret->getNextElementSibling())
    if(E(ret)->getTagName()==name)
      return ret;
  return nullptr;
};
template const DOMElement *DOMElementWrapper<const DOMElement>::getNextElementSiblingNamed(const FQN &name) const; // explicit instantiate const variant

template<typename DOMElementType>
DOMElement *DOMElementWrapper<DOMElementType>::getNextElementSiblingNamed(const FQN &name) {
  for(DOMElement *ret=me->getNextElementSibling(); ret; ret=ret->getNextElementSibling())
    if(E(ret)->getTagName()==name)
      return ret;
  return nullptr;
};

template<typename DOMElementType>
const DOMProcessingInstruction *DOMElementWrapper<DOMElementType>::getFirstProcessingInstructionChildNamed(const string &target) const {
  for(DOMNode *ret=me->getFirstChild(); ret; ret=ret->getNextSibling()) {
    if(ret->getNodeType()!=DOMNode::PROCESSING_INSTRUCTION_NODE)
      continue;
    if(X()%static_cast<DOMProcessingInstruction*>(ret)->getTarget()==target)
      return static_cast<DOMProcessingInstruction*>(ret);
  }
  return nullptr;
}
template const DOMProcessingInstruction *DOMElementWrapper<const DOMElement>::getFirstProcessingInstructionChildNamed(const string &target) const; // explicit instantiate const variant

template<typename DOMElementType>
DOMProcessingInstruction *DOMElementWrapper<DOMElementType>::getFirstProcessingInstructionChildNamed(const string &target) {
  for(DOMNode *ret=me->getFirstChild(); ret; ret=ret->getNextSibling()) {
    if(ret->getNodeType()!=DOMNode::PROCESSING_INSTRUCTION_NODE)
      continue;
    if(X()%static_cast<DOMProcessingInstruction*>(ret)->getTarget()==target)
      return static_cast<DOMProcessingInstruction*>(ret);
  }
  return nullptr;
}

template<typename DOMElementType>
const DOMComment *DOMElementWrapper<DOMElementType>::getFirstCommentChild() const {
  for(DOMNode *ret=me->getFirstChild(); ret; ret=ret->getNextSibling()) {
    if(ret->getNodeType()==DOMNode::COMMENT_NODE)
      return static_cast<DOMComment*>(ret);
  }
  return nullptr;
}
template const DOMComment *DOMElementWrapper<const DOMElement>::getFirstCommentChild() const; // explicit instantiate const variant

template<typename DOMElementType>
DOMComment *DOMElementWrapper<DOMElementType>::getFirstCommentChild() {
  for(DOMNode *ret=me->getFirstChild(); ret; ret=ret->getNextSibling()) {
    if(ret->getNodeType()==DOMNode::COMMENT_NODE)
      return static_cast<DOMComment*>(ret);
  }
  return nullptr;
}

template<typename DOMElementType>
const DOMText *DOMElementWrapper<DOMElementType>::getFirstTextChild() const {
  DOMText *validTextNode;
  DOMText *lastTextNode = nullptr;
  int noneEmptyTextNodeCount = 0;
  for(DOMNode *n=me->getFirstChild(); n; n=n->getNextSibling())
    if(n->getNodeType()==DOMNode::TEXT_NODE || n->getNodeType()==DOMNode::CDATA_SECTION_NODE) {
      lastTextNode = static_cast<DOMText*>(n);
      if(!boost::trim_copy(X()%lastTextNode->getData()).empty()) {
        validTextNode=lastTextNode;
        noneEmptyTextNodeCount++;
      }
    }
  if(noneEmptyTextNodeCount == 0 && lastTextNode)
    return lastTextNode;
  if(noneEmptyTextNodeCount != 1)
    return nullptr;
  return validTextNode;
}
template const DOMText *DOMElementWrapper<const DOMElement>::getFirstTextChild() const; // explicit instantiate const variant

template<typename DOMElementType>
DOMText *DOMElementWrapper<DOMElementType>::getFirstTextChild() {
  DOMText *validTextNode;
  DOMText *lastTextNode = nullptr;
  int noneEmptyTextNodeCount = 0;
  for(DOMNode *n=me->getFirstChild(); n; n=n->getNextSibling())
    if(n->getNodeType()==DOMNode::TEXT_NODE || n->getNodeType()==DOMNode::CDATA_SECTION_NODE) {
      lastTextNode = static_cast<DOMText*>(n);
      if(!boost::trim_copy(X()%lastTextNode->getData()).empty()) {
        validTextNode=lastTextNode;
        noneEmptyTextNodeCount++;
      }
    }
  if(noneEmptyTextNodeCount == 0 && lastTextNode)
    return lastTextNode;
  if(noneEmptyTextNodeCount != 1)
    return nullptr;
  return validTextNode;
}

template<typename DOMElementType>
path DOMElementWrapper<DOMElementType>::getOriginalFilename(bool skipThis, const DOMElement *&found) const {
  found=nullptr;
  const DOMElement *e;
  if(skipThis) {
    if(me->getParentNode() && me->getParentNode()->getNodeType()==DOMNode::ELEMENT_NODE)
      e=static_cast<DOMElement*>(me->getParentNode());
    else
      e=nullptr;
  }
  else
    e=me;
  while(e) {
    if(E(e)->getFirstProcessingInstructionChildNamed("OriginalFilename")) {
      if(e->getOwnerDocument()->getDocumentElement()!=e)
        found=e;
      return X()%E(e)->getFirstProcessingInstructionChildNamed("OriginalFilename")->getData();
    }
    auto *p=e->getParentNode();
    e = p && p->getNodeType()==DOMNode::ELEMENT_NODE ? static_cast<DOMElement*>(e->getParentNode()) : nullptr;
  }
  if(!me)
    throw runtime_error("Invalid call. Null pointer dereference.");
  return D(me->getOwnerDocument())->getDocumentFilename();
}
template path DOMElementWrapper<const DOMElement>::getOriginalFilename(bool skipThis, const DOMElement *&found) const; // explicit instantiate const variant

template<typename DOMElementType>
void DOMElementWrapper<DOMElementType>::setOriginalFilename(path orgFileName) {
  if(orgFileName.empty())
    orgFileName=E(me)->getOriginalFilename();
  DOMProcessingInstruction *filenamePI=me->getOwnerDocument()->createProcessingInstruction(X()%"OriginalFilename",
    X()%orgFileName.string());
  me->insertBefore(filenamePI, me->getFirstChild());
}

template<typename DOMElementType>
path DOMElementWrapper<DOMElementType>::convertPath(const path &relPath) const {
  if(relPath.is_absolute())
    return relPath;
  return toRelativePath(absolute(relPath, getOriginalFilename().parent_path()));
}
template path DOMElementWrapper<const DOMElement>::convertPath(const path &relPath) const; // explicit instantiate const variant

template<typename DOMElementType>
string DOMElementWrapper<DOMElementType>::getAttribute(const FQN &name) const {
  return X()%me->getAttributeNS(X()%name.first, X()%name.second);
}
template string DOMElementWrapper<const DOMElement>::getAttribute(const FQN &name) const; // explicit instantiate const variant

template<typename DOMElementType>
FQN DOMElementWrapper<DOMElementType>::getAttributeQName(const FQN &name) const {
  string str=E(me)->getAttribute(name);
  size_t c=str.find(':');
  if(c==string::npos)
    return FQN(X()%me->lookupNamespaceURI(nullptr), str);
  else
    return FQN(X()%me->lookupNamespaceURI(X()%str.substr(0,c)), str.substr(c+1));
}
template FQN DOMElementWrapper<const DOMElement>::getAttributeQName(const FQN &name) const; // explicit instantiate const variant

template<typename DOMElementType>
const DOMAttr* DOMElementWrapper<DOMElementType>::getAttributeNode(const FQN &name) const {
  return me->getAttributeNodeNS(X()%name.first, X()%name.second);
}
template const DOMAttr* DOMElementWrapper<const DOMElement>::getAttributeNode(const FQN &name) const; // explicit instantiate const variant

template<typename DOMElementType>
DOMAttr* DOMElementWrapper<DOMElementType>::getAttributeNode(const FQN &name) {
  return me->getAttributeNodeNS(X()%name.first, X()%name.second);
}

template<typename DOMElementType>
void DOMElementWrapper<DOMElementType>::setAttribute(const FQN &name, const FQN &value) {
  if(me->isDefaultNamespace(X()%value.first))
    // value is the default namespace set value without a prefix
    setAttribute(name, value.second);
  else {
    // value is not the default namespace set value with a prefix
    const XMLCh *prefix=me->lookupPrefix(X()%value.first);
    if(prefix)
      // the namespace of value has already a prefix -> use this prefix
      setAttribute(name, X()%prefix+":"+value.second);
    else {
      // the namespace of value has no prefix assigned yet -> create a new xmlns attribute with the mapping

      // get a list of all used prefixed of this element
      set<string> usedPrefix;
      DOMNamedNodeMap *attr=me->getAttributes();
      for(size_t i=0; i<attr->getLength(); i++) {
        auto *a=static_cast<DOMAttr*>(attr->item(i));
        string name=X()%a->getName();
        if(name.substr(0,6)!="xmlns:") continue;
        usedPrefix.insert(name.substr(6));
      }
      // search an unused prefix
      int unusedPrefixNr=1;
      while(usedPrefix.find("ns"+fmatvec::toString(unusedPrefixNr))!=usedPrefix.end()) unusedPrefixNr++;
      // set the unsuded prefix
      string unusedPrefix("ns"+fmatvec::toString(unusedPrefixNr));
      setAttribute(XMLNS%("xmlns:"+unusedPrefix), value.first);

      setAttribute(name, unusedPrefix+":"+value.second);
    }
  }
}

template<typename DOMElementType>
int DOMElementWrapper<DOMElementType>::getLineNumber() const {
  const DOMProcessingInstruction *pi=getFirstProcessingInstructionChildNamed("LineNr");
  if(pi)
    return boost::lexical_cast<int>((X()%pi->getData()));
  return 0;
}
template int DOMElementWrapper<const DOMElement>::getLineNumber() const; // explicit instantiate const variant

template<typename DOMElementType>
void DOMElementWrapper<DOMElementType>::removeAttribute(const FQN &name) {
  me->removeAttributeNS(X()%name.first, X()%name.second);
}

template<typename DOMElementType>
int DOMElementWrapper<DOMElementType>::getEmbedCountNumber() const {
  const DOMProcessingInstruction *pi=getFirstProcessingInstructionChildNamed("EmbedCountNr");
  if(pi)
    return boost::lexical_cast<int>((X()%pi->getData()));
  return 0;
}
template int DOMElementWrapper<const DOMElement>::getEmbedCountNumber() const; // explicit instantiate const variant

template<typename DOMElementType>
void DOMElementWrapper<DOMElementType>::setEmbedCountNumber(int embedCount) {
  stringstream str;
  str<<embedCount;
  DOMProcessingInstruction *embedCountPI=me->getOwnerDocument()->createProcessingInstruction(X()%"EmbedCountNr", X()%str.str());
  me->insertBefore(embedCountPI, me->getFirstChild());
}

template<typename DOMElementType>
int DOMElementWrapper<DOMElementType>::getEmbedXPathCount() const {
  const DOMProcessingInstruction *pi=getFirstProcessingInstructionChildNamed("EmbedXPathCount");
  if(pi)
    return boost::lexical_cast<int>((X()%pi->getData()));
  return 0;
}
template int DOMElementWrapper<const DOMElement>::getEmbedXPathCount() const; // explicit instantiate const variant

template<typename DOMElementType>
void DOMElementWrapper<DOMElementType>::setEmbedXPathCount(int xPathCount) {
  stringstream str;
  str<<xPathCount;
  DOMProcessingInstruction *embedXPathCountPI=me->getOwnerDocument()->createProcessingInstruction(X()%"EmbedXPathCount", X()%str.str());
  me->insertBefore(embedXPathCountPI, me->getFirstChild());
}

template<typename DOMElementType>
string DOMElementWrapper<DOMElementType>::getRootXPathExpression() const {
  const DOMElement *e=me;
  const DOMElement *root=e->getOwnerDocument()->getDocumentElement();
  string xpath;
  while(true) {
    // get tag name and namespace uri
    FQN fqn=E(e)->getTagName();
    // get xpath count
    int count=1;
    for(const DOMElement* ee=e->getPreviousElementSibling(); ee; ee=ee->getPreviousElementSibling()) {
      int eeCount=E(ee)->getEmbedXPathCount();
      if(eeCount>0) {
        count=eeCount+1;
        break;
      }
      if(E(ee)->getTagName()==fqn && !E(e)->getFirstProcessingInstructionChildNamed("OriginalFilename"))
        count++;
    }
    // break or continue
    int eCount=E(e)->getEmbedXPathCount();
    if(root==e || E(e)->getFirstProcessingInstructionChildNamed("OriginalFilename")) {
      xpath=string("/{").append(fqn.first).append("}").append(fqn.second).append("[1]").append(xpath); // extend xpath
      if(root==e && eCount>0)
        xpath=string("/{").append(PV.getNamespaceURI()).append("}Embed[1]").append(xpath);
      break;
    }
    else {
      if(eCount>0)
        count=1;
      xpath=string("/{").append(fqn.first+"}").append(fqn.second).append("[").append(to_string(count)).append("]").append(xpath); // extend xpath
      if(eCount>0)
        xpath=string("/{").append(PV.getNamespaceURI()).append("}Embed[").append(to_string(eCount)).append("]").append(xpath);
    }
    e=static_cast<const DOMElement*>(e->getParentNode());
    if(!e) { // it may happen that e is nullptr -> we cannot return an xpath in this case since the full abs path is not known
      xpath="";
      break;
    }
  }
  return xpath;
}
template string DOMElementWrapper<const DOMElement>::getRootXPathExpression() const; // explicit instantiate const variant

template<typename DOMElementType>
int DOMElementWrapper<DOMElementType>::getOriginalElementLineNumber() const {
  const DOMProcessingInstruction *pi=getFirstProcessingInstructionChildNamed("OriginalElementLineNr");
  if(pi)
    return boost::lexical_cast<int>((X()%pi->getData()));
  return 0;
}
template int DOMElementWrapper<const DOMElement>::getOriginalElementLineNumber() const; // explicit instantiate const variant

template<typename DOMElementType>
void DOMElementWrapper<DOMElementType>::setOriginalElementLineNumber(int lineNr) {
  stringstream str;
  str<<lineNr;
  DOMProcessingInstruction *embedCountPI=me->getOwnerDocument()->createProcessingInstruction(X()%"OriginalElementLineNr", X()%str.str());
  me->insertBefore(embedCountPI, me->getFirstChild());
}

template<typename DOMElementType>
void DOMElementWrapper<DOMElementType>::workaroundDefaultAttributesOnImportNode() {
  // rest all default attributes to the default value exlicitly: this removed the default "flag"
  DOMNamedNodeMap *attr=me->getAttributes();
  for(int i=0; i<attr->getLength(); i++) {
    auto *a=static_cast<DOMAttr*>(attr->item(i));
    if(!a->getSpecified())
      a->setValue(X()%(X()%a->getValue()));
  }
  // loop over all child elements recursively
  for(DOMElement *c=me->getFirstElementChild(); c!=nullptr; c=c->getNextElementSibling())
    E(c)->workaroundDefaultAttributesOnImportNode();
}

template<typename DOMElementType>
bool DOMElementWrapper<DOMElementType>::hasAttribute(const FQN &name) const {
  return me->hasAttributeNS(X()%name.first, X()%name.second);
}
template bool DOMElementWrapper<const DOMElement>::hasAttribute(const FQN &name) const; // explicit instantiate const variant

bool isDerivedFrom(const DOMNode *me, const FQN &baseTypeName) {
  shared_ptr<DOMParser> parser=*static_cast<shared_ptr<DOMParser>*>(me->getOwnerDocument()->getUserData(X()%DOMParser::domParserKey));

  const DOMTypeInfo *type;
  if(me->getNodeType()==DOMNode::ELEMENT_NODE)
    type=static_cast<const DOMElement*>(me)->getSchemaTypeInfo();
  else
    type=static_cast<const DOMAttr*>(me)->getSchemaTypeInfo();
  FQN typeName(X()%type->getTypeNamespace(), X()%type->getTypeName());

  auto it=parser->typeMap.find(typeName);
  if(it==parser->typeMap.end())
    throw runtime_error("Internal error: Type {"+typeName.first+"}"+typeName.second+" not found.");
  return it->second->derivedFrom(X()%baseTypeName.first, X()%baseTypeName.second);
}

template<typename DOMElementType>
bool DOMElementWrapper<DOMElementType>::isDerivedFrom(const FQN &baseTypeName) const {
  return MBXMLUtils::isDerivedFrom(me, baseTypeName);
}
template bool DOMElementWrapper<const DOMElement>::isDerivedFrom(const FQN &baseTypeName) const; // explicit instantiate const variant

// Explicit instantiate none const variante. Note the const variant should only be instantiate for const members.
template class DOMElementWrapper<DOMElement>;

template<typename DOMAttrType>
bool DOMAttrWrapper<DOMAttrType>::isDerivedFrom(const FQN &baseTypeName) const {
  return MBXMLUtils::isDerivedFrom(me, baseTypeName);
}
template bool DOMAttrWrapper<const DOMAttr>::isDerivedFrom(const FQN &baseTypeName) const; // explicit instantiate const variant

template<typename DOMAttrType>
string DOMAttrWrapper<DOMAttrType>::getRootXPathExpression() const {
  return E(me->getOwnerElement())->getRootXPathExpression()+"/@"+X()%me->getNodeName();
}
template string DOMAttrWrapper<const DOMAttr>::getRootXPathExpression() const; // explicit instantiate const variant

// Explicit instantiate none const variante. Note the const variant should only be instantiate for const members.
template class DOMAttrWrapper<DOMAttr>;

template<typename DOMDocumentType>
void DOMDocumentWrapper<DOMDocumentType>::validate() {
  // normalize document
  D(me)->normalizeDocument();

  // serialize to memory
  DOMImplementation *impl=DOMImplementationRegistry::getDOMImplementation(X()%"");
  shared_ptr<DOMLSSerializer> ser(impl->createLSSerializer(), [](auto && PH1) { if(PH1) PH1->release(); });
  shared_ptr<XMLCh> data(ser->writeToString(me), &X::releaseXMLCh); // serialize to data being UTF-16
  if(!data.get())
    throw runtime_error("Serializing the document to memory failed.");
  // count number of words (16bit blocks); UTF-16 multi word characters are counted as 2 words
  int dataByteLen=0;
  while(data.get()[dataByteLen]!=0) { dataByteLen++; }
  dataByteLen*=2; // a word has 2 bytes

  // parse from memory
  shared_ptr<DOMParser> parser=*static_cast<shared_ptr<DOMParser>*>(me->getUserData(X()%DOMParser::domParserKey));
  MemBufInputSource memInput(reinterpret_cast<XMLByte*>(data.get()), dataByteLen, X()%D(me)->getDocumentFilename().string(), false);
  Wrapper4InputSource domInput(&memInput, false);
  parser->errorHandler.resetError();
  shared_ptr<DOMDocument> newDoc(parser->parser->parse(&domInput), [](auto && PH1) { if(PH1) PH1->release(); });
  if(parser->errorHandler.hasError())
    throw parser->errorHandler.getError();

  // replace old document element with new one
  E(newDoc->getDocumentElement())->workaroundDefaultAttributesOnImportNode();// workaround
  DOMNode *newRoot=me->importNode(newDoc->getDocumentElement(), true);
  me->replaceChild(newRoot, me->getDocumentElement())->release();
}

template<typename DOMDocumentType>
xercesc::DOMElement* DOMDocumentWrapper<DOMDocumentType>::createElement(const FQN &name) {
  if(name.first.empty()) // we interprete a empty namespace (string "") as no namespace
    return me->createElement(X()%name.second);
  else
    return me->createElementNS(X()%name.first, X()%name.second);
}

template<typename DOMDocumentType>
void DOMDocumentWrapper<DOMDocumentType>::normalizeDocument() {
  // there is a xerces-c bug which prevents e.g. the following XML code from normalizeDocument:
  // <root xmlns="http://a" xmlns:pre="http://a">
  //   <child xmlns="http://b" xmlns:pre="http://a"/>
  // </root>
  // See bug report: https://issues.apache.org/jira/browse/XERCESC-2244
  // As a workaround we disable namespace processing during normalization
  bool doNamespaces=static_cast<bool>(me->getDOMConfig()->getParameter(XMLUni::fgDOMNamespaces));
  me->getDOMConfig()->setParameter(XMLUni::fgDOMNamespaces, false);
  BOOST_SCOPE_EXIT_TPL(me, doNamespaces) {
    me->getDOMConfig()->setParameter(XMLUni::fgDOMNamespaces, doNamespaces);
  } BOOST_SCOPE_EXIT_END
  me->normalizeDocument();
}

template<typename DOMDocumentType>
shared_ptr<DOMParser> DOMDocumentWrapper<DOMDocumentType>::getParser() const {
  return *static_cast<shared_ptr<DOMParser>*>(me->getUserData(X()%DOMParser::domParserKey));
}
template shared_ptr<DOMParser> DOMDocumentWrapper<const DOMDocument>::getParser() const; // explicit instantiate const variant

template<typename DOMDocumentType>
path DOMDocumentWrapper<DOMDocumentType>::getDocumentFilename() const {
  string uri=X()%me->getDocumentURI();
  // handle in-memory-files
  if(uri.empty())
    return {};
  // handle (the original xerces) schema for local files
  static const string fileScheme="file://";
  if(uri.substr(0, fileScheme.length())==fileScheme) {
#ifdef _WIN32
    int addChars = 1; // Windows uses e.g. file:///c:/path/to/file.txt -> file:/// must be removed
#else
    int addChars = 0; // Linux uses e.g. file:///path/to/file.txt -> file:// must be removed
#endif
    return uri.substr(fileScheme.length() + addChars);
  }
  // handle mbxmlutilsfile schema
  static const string mbxmlutilsfileSchema="mbxmlutilsfile://";
  if(uri.substr(0, mbxmlutilsfileSchema.length())==mbxmlutilsfileSchema)
    return uri.substr(mbxmlutilsfileSchema.length());
  // all other schemas are errors
  throw runtime_error("Only local filename schemas and the special mbxmlutilsfile schema is allowed.");
}
template path DOMDocumentWrapper<const DOMDocument>::getDocumentFilename() const; // explicit instantiate const variant

template<typename DOMDocumentType>
DOMNode* DOMDocumentWrapper<DOMDocumentType>::evalRootXPathExpression(string xpathExpression, DOMElement* context) {
  // we cannot use std::regex here since named groups and conditionals are not supported by std::regex
  static const boost::regex re(R"q(/{([^}]+)}([^[]+)\[([0-9]+)\](.*))q");
  if(!context) context=me->getDocumentElement();
  DOMElement *p=context;
  boost::smatch m;
  bool first=true;
  while(true) {
    if(!boost::regex_match(xpathExpression, m, re)) break;
    // special handling of the first element (the root element itself)
    if(first && E(p)->getTagName()!=FQN(m.str(1), m.str(2)))
      throw runtime_error("No matching node found for XPath expression.");
    if(!first) {
      p=E(p)->getFirstElementChildNamed(FQN(m.str(1), m.str(2)));
      if(!p)
        throw runtime_error("No matching node found for XPath expression.");
      int count=boost::lexical_cast<int>(m.str(3));
      for(int c=2; c<=count; ++c) {
        p=E(p)->getNextElementSiblingNamed(FQN(m.str(1), m.str(2)));
        if(!p)
          throw runtime_error("No matching node found for XPath expression.");
      }
    }
    xpathExpression=m.str(4);
    first=false;
  }
  // finished?
  if(xpathExpression.empty())
    return p;
  // handle attribute
  if(xpathExpression.substr(0, 2)=="/@") {
    DOMAttr *a=E(p)->getAttributeNode(xpathExpression.substr(2));
    if(!a)
      throw runtime_error("No matching node found for XPath expression.");
    return a;
  }
  throw runtime_error("No matching node found for XPath expression.");
}

// Explicit instantiate none const variante. Note the const variant should only be instantiate for const members.
template class DOMDocumentWrapper<DOMDocument>;

DOMEvalException::DOMEvalException(const string &errorMsg_, const DOMNode *n) {
  // store error message
  errorMsg=errorMsg_;
  // create a DOMLocator stack (by using embed elements (OriginalFilename processing instructions))
  if(n)
    appendContext(n);
}

DOMEvalException::DOMEvalException(const std::string &errorMsg_, const xercesc::DOMLocator &loc) {
  // store error message
  errorMsg=errorMsg_;

  // location with a stack
  const DOMNode *n=loc.getRelatedNode();
  if(!n)
    return;
  if(n->getNodeType()==DOMNode::ELEMENT_NODE)
    appendContext(n, loc.getLineNumber());
  else if(n->getNodeType()==DOMNode::ATTRIBUTE_NODE)
    appendContext(n, loc.getLineNumber());
  else if(n->getNodeType()==DOMNode::TEXT_NODE)
    appendContext(n->getParentNode(), loc.getLineNumber());
  else if(n->getNodeType()==DOMNode::DOCUMENT_NODE)
    appendContext(n, loc.getLineNumber());
  else
    assert(false && "DOMEvalException can only be called with a DOMLocator of node type element, attribute or text.");
}

void DOMEvalException::appendContext(const DOMNode *n, int externLineNr) {
  string xpath;
  path filename;
  int lineNr;
  int embedCount;
  const DOMElement *found=nullptr;
  if(n->getNodeType()==DOMNode::ELEMENT_NODE) {
    const DOMElement *ee=static_cast<const DOMElement*>(n);
    xpath=E(ee)->getRootXPathExpression();
    filename=E(ee)->getOriginalFilename(false, found);
    lineNr=E(ee)->getLineNumber();
    embedCount=E(ee)->getEmbedCountNumber();
  }
  else if(n->getNodeType()==DOMNode::ATTRIBUTE_NODE) {
    const DOMElement *ee=static_cast<const DOMAttr*>(n)->getOwnerElement();
    xpath=A(static_cast<const DOMAttr*>(n))->getRootXPathExpression();
    filename=E(ee)->getOriginalFilename(false, found);
    lineNr=E(ee)->getLineNumber();
    embedCount=E(ee)->getEmbedCountNumber();
  }
  else if(n->getNodeType()==DOMNode::DOCUMENT_NODE) {
    auto *doc=static_cast<const xercesc::DOMDocument*>(n);
    xpath="/";
    filename=D(doc)->getDocumentFilename();
    lineNr=0;
    embedCount=0;
  }
  else
    throw runtime_error("DOMEvalException::appendContext can only be called for element and attribute nodes.");

  locationStack.emplace_back(filename, externLineNr>0 ? externLineNr : lineNr, embedCount, xpath);
  auto ee=found;
  while(ee) {
    string xpath;
    if(ee->getParentNode())
      xpath=E(static_cast<const DOMElement*>(ee->getParentNode()))->getRootXPathExpression()+"/{"+PV.getNamespaceURI()+"}Embed["+
        to_string(E(ee)->getEmbedXPathCount())+"]";
    else
      xpath="[no xpath available]";
    locationStack.emplace_back(E(ee)->getOriginalFilename(true, found),
      E(ee)->getOriginalElementLineNumber(),
      E(ee)->getEmbedCountNumber(),
      xpath);
    ee=found;
  }
}

string DOMEvalException::convertToString(const EmbedDOMLocator &loc, const std::string &message, bool subsequentError) {
  // get MBXMLUTILS_ERROROUTPUT
  const char *ev=getenv("MBXMLUTILS_ERROROUTPUT");
  string format(ev?ev:"GCC");
  if(format=="GCC" || format=="GCCTTY" || format=="GCCNONE") {
#ifdef _WIN32
    bool stdoutIsTTY=GetFileType(GetStdHandle(STD_OUTPUT_HANDLE))==FILE_TYPE_CHAR;
#else
    bool stdoutIsTTY=isatty(1)==1;
#endif
    if((format=="GCC" && stdoutIsTTY) || format=="GCCTTY")
      format=R"|(\e]8;;file://$+{urifile}\a\e[1m$+{file}\e[0m\e]8;;\a\e[1m:(?{line}$+{line}\::)(?{ecount} [ecount=$+{ecount}]:)\e[0m (?{sse}:\e[31;1m)$+{msg}\e[0m)|";
    else
      format=R"|($+{file}:(?{line}$+{line}\::)(?{ecount} [ecount=$+{ecount}]:) $+{msg})|";
  }
  else if(format=="HTMLFILELINE")
    format=R"|(<span class="MBXMLUTILS_ERROROUTPUT(?{sse} MBXMLUTILS_SSE:)"><a href="$+{file}(?{line}\?line=$+{line}:)">$+{file}(?{line}\:$+{line}:)</a>(?{ecount} [ecount=<span class="MBXMLUTILS_ECOUNT">$+{ecount}</span>]:) <span class="MBXMLUTILS_MSG">$+{msg}</span></span>)|";
  else if(format=="HTMLXPATH")
    format=R"|(<span class="MBXMLUTILS_ERROROUTPUT(?{sse} MBXMLUTILS_SSE:)"><a href="$+{file}?xpath=$+{xpath}(?{ecount}&ecount=$+{ecount}:)(?{line}\&line=$+{line}:)">$+{file}</a>: <span class="MBXMLUTILS_MSG">$+{msg}</span></span>)|";

  // Generate a boost::match_results object.
  // To avoid using boost internal inoffizial functions to create a match_results object we use the foolowing
  // string and regex to generate it implicitly.
  string str;
  str+="@msg"+message; // @msg includes a new line at the end
  str+="@file"+X()%loc.getURI();
  str+="@absfile"+absolute(X()%loc.getURI()).string();
  str+="@urifile"+absolute(X()%loc.getURI()).string();//mfmf uriencode
  str+="@line"+(loc.getLineNumber()>0?to_string(loc.getLineNumber()):"");
  str+="@xpath"+loc.getRootXPathExpression();
  str+="@ecount"+(loc.getEmbedCount()>0?to_string(loc.getEmbedCount()):"");
  str+="@sse"+(subsequentError?string("x"):"");
  static const boost::regex re(
    R"q(^@msg(?<msg>.+)?@file(?<file>.+)?@absfile(?<absfile>.+)?@urifile(?<urifile>.+)?@line(?<line>.+)?@xpath(?<xpath>.+)?@ecount(?<ecount>.+)?@sse(?<sse>.+)?$)q"
  );
  // apply substitutions
  return boost::regex_replace(str, re, format, boost::regex_constants::format_all);
}

const char* DOMEvalException::what() const noexcept {
  // note the laste line break is skipped
  whatStr.clear();
  if(!locationStack.empty()) {
    auto it=locationStack.begin();
    whatStr+=convertToString(*it, errorMsg+(next(it)!=locationStack.end()?"\n":""), subsequentError);
    for(it++; it!=locationStack.end(); it++)
      whatStr+=convertToString(*it, string("included from here")+(next(it)!=locationStack.end()?"\n":""), true);
  }
  else
    whatStr+=errorMsg;

  return whatStr.c_str();
}

const string LocationInfoFilter::lineNumberKey("http://www.mbsim-env.de/dom/MBXMLUtils/lineNumber");

// START: call protected AbstractDOMParser::getScanner from outside, see above
struct GETSCANNER { using type = XMLScanner *(AbstractDOMParser::*)() const; };
namespace { template struct rob<GETSCANNER, &AbstractDOMParser::getScanner>; }
// END: call protected AbstractDOMParser::getScanner from outside, see above
DOMLSParserFilter::FilterAction LocationInfoFilter::startElement(DOMElement *e) {
  // store the line number of the element start as user data
  auto *abstractParser=dynamic_cast<AbstractDOMParser*>(parser->parser.get());
  int lineNr=(abstractParser->*result<GETSCANNER>::ptr)()->getLocator()->getLineNumber();
  e->setUserData(X()%lineNumberKey, new int(lineNr), nullptr);
  return FILTER_ACCEPT;
}

DOMLSParserFilter::FilterAction LocationInfoFilter::acceptNode(DOMNode *n) {
  auto* e=static_cast<DOMElement*>(n);
  // get the line number of the element start from user data and reset user data
  shared_ptr<int> lineNrPtr(static_cast<int*>(e->getUserData(X()%lineNumberKey)));
  e->setUserData(X()%lineNumberKey, nullptr, nullptr);
  // if no LineNr processing instruction exists create it using the line number from user data
  DOMProcessingInstruction *pi=E(e)->getFirstProcessingInstructionChildNamed("LineNr");
  if(!pi) {
    stringstream str;
    str<<*lineNrPtr;
    DOMProcessingInstruction *lineNrPI=e->getOwnerDocument()->createProcessingInstruction(X()%"LineNr", X()%str.str());
    e->insertBefore(lineNrPI, e->getFirstChild());
  }
  // return (lineNrPtr is deleted implicitly)
  return FILTER_ACCEPT;
}

DOMNodeFilter::ShowType LocationInfoFilter::getWhatToShow() const {
  return DOMNodeFilter::SHOW_ELEMENT;
}

void TypeDerivativeHandler::handleElementPSVI(const XMLCh *const localName, const XMLCh *const uri, PSVIElement *info) {
  XSTypeDefinition *type=info->getTypeDefinition();
  if(!type) // no type found for this element just return
    return;
  FQN name(X()%type->getNamespace(), X()%type->getName());
  parser->typeMap[name]=type;
}

void TypeDerivativeHandler::handleAttributesPSVI(const XMLCh *const localName, const XMLCh *const uri, PSVIAttributeList *psviAttributes) {
  for(int i=0; i<psviAttributes->getLength(); i++) {
    PSVIAttribute *info=psviAttributes->getAttributePSVIAtIndex(i);
    // the xmlns attribute has not type -> skip it (maybe a bug in xerces, but this attribute is not needed)
    if((X()%psviAttributes->getAttributeNamespaceAtIndex(i)).empty() && X()%psviAttributes->getAttributeNameAtIndex(i)=="xmlns")
      continue;

    XSTypeDefinition *type=info->getTypeDefinition();
    if(!type) // no type found for this attribute just return
      return;
    FQN name(X()%type->getNamespace(), X()%type->getName());
    parser->typeMap[name]=type;
  }
}

void DOMParserUserDataHandler::handle(DOMUserDataHandler::DOMOperationType operation, const XMLCh* const key,
  void *data, const DOMNode *src, DOMNode *dst) {
  if(X()%key==DOMParser::domParserKey) {
    if(operation==NODE_DELETED) {
      delete static_cast<shared_ptr<DOMParser>*>(data);
      return;
    }
    // handle xerces bugs!?
    if((operation==NODE_IMPORTED && src->getNodeType()==DOMNode::TEXT_NODE && dst->getNodeType()==DOMNode::TEXT_NODE) ||
       (operation==NODE_IMPORTED && src->getNodeType()==DOMNode::ATTRIBUTE_NODE && dst->getNodeType()==DOMNode::ATTRIBUTE_NODE) ||
       (operation==NODE_IMPORTED && src->getNodeType()==DOMNode::PROCESSING_INSTRUCTION_NODE && dst->getNodeType()==DOMNode::PROCESSING_INSTRUCTION_NODE) ||
       (operation==NODE_IMPORTED && src->getNodeType()==DOMNode::ELEMENT_NODE && dst->getNodeType()==DOMNode::ELEMENT_NODE) ||
       (operation==NODE_IMPORTED && src->getNodeType()==DOMNode::CDATA_SECTION_NODE && dst->getNodeType()==DOMNode::CDATA_SECTION_NODE) ||
       (operation==NODE_IMPORTED && src->getNodeType()==DOMNode::COMMENT_NODE && dst->getNodeType()==DOMNode::COMMENT_NODE))
      return;
  }
  throw runtime_error("Internal error: Unknown user data handling: op="+fmatvec::toString(operation)+", key="+X()%key+
                      ", src="+fmatvec::toString(src!=nullptr)+", dst="+fmatvec::toString(dst!=nullptr)+
                      (src ? ", srcType="+fmatvec::toString(src->getNodeType()) : "")+
                      (dst ? ", dstType="+fmatvec::toString(dst->getNodeType()) : ""));
}

const string DOMParser::domParserKey("http://www.mbsim-env.de/dom/MBXMLUtils/domParser");
DOMParserUserDataHandler DOMParser::userDataHandler;

shared_ptr<DOMParser> DOMParser::create(const variant<path, DOMElement*> &xmlCatalog) {
  return shared_ptr<DOMParser>(new DOMParser(xmlCatalog));
}

InputSource* EntityResolver::resolveEntity(XMLResourceIdentifier *resourceIdentifier) {
  // handle only schema import
  if(resourceIdentifier->getResourceIdentifierType()!=XMLResourceIdentifier::SchemaImport)
    return nullptr;
  // handle schema import -> map namespace to local file
  string ns=X()%resourceIdentifier->getNameSpace();
  path file;
  static boost::filesystem::path installPath(boost::filesystem::path(domLoc()).parent_path().parent_path());
  path SCHEMADIR=installPath/"share"/"mbxmlutils"/"schema";
  // handle namespaces known by MBXMLUtils
  if(ns==XINCLUDE.getNamespaceURI())
    file=SCHEMADIR/"http___www_w3_org/XInclude.xsd";
  else if(ns==PV.getNamespaceURI())
    file=SCHEMADIR/"http___www_mbsim-env_de_MBXMLUtils/mbxmlutils.xsd";
  // handle namespaces registered to the parser
  else {
    // search for a registered namespace
    auto it=parser->registeredGrammar.find(ns);
    // not found -> return nullptr
    if(it==parser->registeredGrammar.end())
      return nullptr;
    // return registered schema file
    file=it->second;
  }
  return new LocalFileInputSource(X()%file.string());
}

DOMParser::DOMParser(const variant<path, DOMElement*> &xmlCatalog) {
  // get DOM implementation and create parser
  domImpl=DOMImplementationRegistry::getDOMImplementation(X()%"");
  parser.reset(domImpl->createLSParser(DOMImplementation::MODE_SYNCHRONOUS, XMLUni::fgDOMXMLSchemaType),
    [](auto && PH1) { if(PH1) PH1->release(); });
  // convert parser to AbstractDOMParser and store in parser filter
  auto *abstractParser=dynamic_cast<AbstractDOMParser*>(parser.get());
  if(!abstractParser)
    throw runtime_error("Internal error: Parser is not of type AbstractDOMParser.");
  locationFilter.setParser(this);
  parser->setFilter(&locationFilter);
  // entity resolver (do never load entities using network)
  abstractParser->setDisableDefaultEntityResolution(true);
  // configure the parser
  DOMConfiguration *config=parser->getDomConfig();
  config->setParameter(XMLUni::fgDOMErrorHandler, &errorHandler);
  config->setParameter(XMLUni::fgXercesUserAdoptsDOMDocument, true);
  config->setParameter(XMLUni::fgXercesDoXInclude, false);
  //config->setParameter(XMLUni::fgDOMCDATASections, false); // is not implemented by xercesc handle it by DOMParser
  // configure validating parser
  auto *xmlCatalogFile=std::get_if<path>(&xmlCatalog);
  auto *xmlCatalogEle=std::get_if<DOMElement*>(&xmlCatalog);
  if((xmlCatalogFile && !xmlCatalogFile->empty()) ||
     (xmlCatalogEle && *xmlCatalogEle)) {
    config->setParameter(XMLUni::fgXercesScannerName, XMLUni::fgSGXMLScanner);
    config->setParameter(XMLUni::fgDOMValidate, true);
    config->setParameter(XMLUni::fgXercesSchema, true);
    config->setParameter(XMLUni::fgXercesUseCachedGrammarInParse, true);
    config->setParameter(XMLUni::fgXercesDOMHasPSVIInfo, true);
    entityResolver.setParser(this);
    config->setParameter(XMLUni::fgXercesEntityResolver, &entityResolver);
    typeDerHandler.setParser(this);
    abstractParser->setPSVIHandler(&typeDerHandler);
    shared_ptr<DOMParser> nonValParser=create();
    shared_ptr<DOMDocument> doc;
    DOMElement *root=nullptr;
    if(xmlCatalogFile) {
      doc=nonValParser->parse(*xmlCatalogFile);
      root=doc->getDocumentElement();
    }
    else
      root=*xmlCatalogEle;
    static const NamespaceURI XMLNAMESPACE("http://www.w3.org/XML/1998/namespace");
    if(E(root)->hasAttribute(XMLNAMESPACE%"base"))
      throw runtime_error("This parser does not supports xml:base attributes in XML catalogs.");
    for(DOMElement *c=root->getFirstElementChild(); c!=nullptr; c=c->getNextElementSibling()) {
      if(E(c)->getTagName()!=XMLCATALOG%"uri")
        throw runtime_error("This parser only supports <uri> elements in XML catalogs.");
      if(E(c)->hasAttribute(XMLNAMESPACE%"base"))
        throw runtime_error("This parser does not supports xml:base attributes in XML catalogs.");
      path schemaPath=E(c)->getAttribute("uri");
      if(!xmlCatalogFile && schemaPath.is_relative())
        throw runtime_error("Relative path in XML catalogs are only supported when reading the catalog from file ("+
                            schemaPath.string()+").");
      if(xmlCatalogFile)
        schemaPath=absolute(schemaPath, xmlCatalogFile->parent_path());
      registeredGrammar.emplace(E(c)->getAttribute("name"), schemaPath);
    }
    for(auto &[ns, schemaFilename]: registeredGrammar) {
      std::ignore = ns;
      // load grammar
      errorHandler.resetError();
      parser->loadGrammar(X()%schemaFilename.string(), Grammar::SchemaGrammarType, true);
      if(errorHandler.hasError())
        throw errorHandler.getError();
    }
  }
}

void DOMParser::handleXInclude(DOMElement *&e, vector<path> *dependencies) {
  // handle xinclude
  if(E(e)->getTagName()==XINCLUDE%"include") {
    path incFile=E(e)->convertPath(E(e)->getAttribute("href"));
    if(dependencies)
      dependencies->push_back(incFile);
    shared_ptr<DOMDocument> incDoc=D(e->getOwnerDocument())->getParser()->parse(incFile, dependencies);
    E(incDoc->getDocumentElement())->workaroundDefaultAttributesOnImportNode();// workaround
    DOMNode *incNode=e->getOwnerDocument()->importNode(incDoc->getDocumentElement(), true);
    e->getParentNode()->replaceChild(incNode, e)->release();
    e=static_cast<DOMElement*>(incNode);
    return;
  }
  // walk tree
  for(DOMElement *c=e->getFirstElementChild(); c!=nullptr; c=c->getNextElementSibling()) {
    handleXInclude(c, dependencies);
    if(c==nullptr) break;
  }
}

shared_ptr<DOMDocument> DOMParser::parse(const path &inputSource, vector<path> *dependencies, bool doXInclude) {
  if(!exists(inputSource))
    throw runtime_error("XML document "+inputSource.string()+" not found");
  // reset error handler and parser document and throw on errors
  errorHandler.resetError();
  shared_ptr<DOMDocument> doc;
  // check if file is writeable
  bool writeable=true;
  {
    std::ofstream dummy(inputSource.string(), ios_base::app);
    writeable=dummy.is_open();
  }
  // if the file is writable use a lock file, if not writable no locking is needed
  if(writeable) {
    // at least using wine we cannot use inputSource as lock file itself, its crashing
    path inputSourceLock(inputSource.parent_path()/("."+inputSource.filename().string()+".lock"));
    { std::ofstream dummy(inputSourceLock.string()); } // create the file
#ifdef _WIN32
    { // make lock file hidden on windows
      auto attrs = GetFileAttributesA(inputSourceLock.string().c_str());
      if(attrs != INVALID_FILE_ATTRIBUTES)
        SetFileAttributesA(inputSourceLock.string().c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
    }
#endif
    boost::interprocess::file_lock inputSourceFileLock(inputSourceLock.string().c_str()); // lock the file
    boost::interprocess::sharable_lock lock(inputSourceFileLock);
    doc.reset(parser->parseURI(X()%inputSource.string()), [](auto && PH1) { if(PH1) PH1->release(); });
  }
  else
    doc.reset(parser->parseURI(X()%inputSource.string()), [](auto && PH1) { if(PH1) PH1->release(); });
  doc->setDocumentURI(X()%("mbxmlutilsfile://"+inputSource.string()));
  if(errorHandler.hasError()) {
    // fix the filename
    DOMEvalException ex(errorHandler.getError());
    if(!ex.locationStack.empty()) {
      auto &l=ex.locationStack.front();
      EmbedDOMLocator exNew(inputSource.string(), l.getLineNumber(),
                            l.getEmbedCount(), l.getRootXPathExpression());
      ex.locationStack.front()=exNew;
    }
    throw ex;
  }
  // add a new shared_ptr<DOMParser> to document user data to extend the lifetime to the lifetime of all documents
  doc->setUserData(X()%domParserKey, new shared_ptr<DOMParser>(shared_from_this()), &userDataHandler);
  // set file name
  DOMElement *root=doc->getDocumentElement();
  if(!E(root)->getFirstProcessingInstructionChildNamed("OriginalFilename")) {
    DOMProcessingInstruction *filenamePI=doc->createProcessingInstruction(X()%"OriginalFilename",
      X()%inputSource.string());
    root->insertBefore(filenamePI, root->getFirstChild());
  }
  // handle CDATA nodes
  if(doXInclude)
    handleXInclude(root, dependencies);
  // return DOM document
  return doc;
}

namespace {
  shared_ptr<DOMLSSerializer> serializeHelper(DOMNode *n, bool prettyPrint);
}

void DOMParser::serialize(DOMNode *n, const path &outputSource, bool prettyPrint) {
  shared_ptr<DOMLSSerializer> ser=serializeHelper(n, prettyPrint);
  // at least using wine we cannot use outputSource as lock file itself, its crashing
  path outputSourceLock(outputSource.parent_path()/("."+outputSource.filename().string()+".lock"));
  { std::ofstream dummy(outputSourceLock.string()); } // create the file
#ifdef _WIN32
  { // make lock file hidden on windows
    auto attrs = GetFileAttributesA(outputSourceLock.string().c_str());
    if(attrs != INVALID_FILE_ATTRIBUTES)
      SetFileAttributesA(outputSourceLock.string().c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);
  }
#endif
  boost::interprocess::file_lock outputSourceFileLock(outputSourceLock.string().c_str()); // lock the file
  boost::interprocess::scoped_lock lock(outputSourceFileLock);
  if(!ser->writeToURI(n, X()%outputSource.string()))
    throw runtime_error("Serializing the document failed.");
}

void DOMParser::serializeToString(DOMNode *n, string &outputData, bool prettyPrint) {
  shared_ptr<DOMLSSerializer> ser=serializeHelper(n, prettyPrint);
  // disable the XML declaration which will be UTF-16 but we convert ti later to UTF-8
  ser->getDomConfig()->setParameter(XMLUni::fgDOMXMLDeclaration, false);
  shared_ptr<XMLCh> data(ser->writeToString(n), &X::releaseXMLCh); // serialize to data being UTF-16
  if(!data.get())
    throw runtime_error("Serializing the document to memory failed.");
  outputData=X()%data.get();
}

namespace {
  shared_ptr<DOMLSSerializer> serializeHelper(DOMNode *n, bool prettyPrint) {
    if(n->getNodeType()==DOMNode::DOCUMENT_NODE)
      D(static_cast<DOMDocument*>(n))->normalizeDocument();
    else
      D(n->getOwnerDocument())->normalizeDocument();

    DOMImplementation *impl=DOMImplementationRegistry::getDOMImplementation(X()%"");
    shared_ptr<DOMLSSerializer> ser(impl->createLSSerializer(), [](auto && PH1) { if(PH1) PH1->release(); });
    ser->getDomConfig()->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, prettyPrint);
    return ser;
  }
}

void DOMParser::resetCachedGrammarPool() {
  parser->resetCachedGrammarPool();
  typeMap.clear();
}

shared_ptr<DOMDocument> DOMParser::createDocument() {
  shared_ptr<DOMDocument> doc(domImpl->createDocument(), [](auto && PH1) { if(PH1) PH1->release(); });
  doc->setUserData(X()%domParserKey, new shared_ptr<DOMParser>(shared_from_this()), &userDataHandler);
  return doc;
}

}
