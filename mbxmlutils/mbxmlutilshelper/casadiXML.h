#ifndef _MBXMLUTILS_CASADIXML_H_
#define _MBXMLUTILS_CASADIXML_H_

#include <casadi/symbolic/sx/sx.hpp>
#include <casadi/symbolic/fx/sx_function.hpp>
#include <casadi/symbolic/fx/sx_function.hpp>
#include <mbxmlutilshelper/dom.h>
#include <map>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMImplementation.hpp>


namespace CasADi {

const MBXMLUtils::NamespaceURI CASADI("http://openmbv.berlios.de/MBXMLUtils/CasADi");

inline xercesc::DOMElement *convertCasADiToXML(const SX &s, std::map<SXNode*, int> &nodes, xercesc::DOMDocument *doc) {
  // add the node of s to the list of all nodes (creates a integer id for newly added nodes)
  std::string idStr;
  std::pair<std::map<SXNode*, int>::iterator, bool> ret=nodes.insert(std::make_pair(s.get(), nodes.size()));
  // if the node of s already exists in the list of all nodes write a reference to this node to XML
  if(ret.second==false) {
    xercesc::DOMElement *e=MBXMLUtils::D(doc)->createElement(CASADI%"reference");
    std::stringstream str;
    str<<ret.first->second;
    MBXMLUtils::E(e)->setAttribute("refid", str.str());
    return e;
  }
  // if the node of s does not exist in the list of all nodes set the id of this node
  else {
    std::stringstream str;
    str<<ret.first->second;
    idStr=str.str();
  }

  // add s to XML dependent on the type of s
  xercesc::DOMElement *e;
  if(s.isSymbolic()) {
    e=MBXMLUtils::D(doc)->createElement(CASADI%"SymbolicSX");
    e->insertBefore(doc->createTextNode(MBXMLUtils::X()%s.getName()), NULL);
  }
  else if(s.isZero())
    e=MBXMLUtils::D(doc)->createElement(CASADI%"ZeroSX");
  else if(s.isOne())
    e=MBXMLUtils::D(doc)->createElement(CASADI%"OneSX");
  else if(s.isMinusOne())
    e=MBXMLUtils::D(doc)->createElement(CASADI%"MinusOneSX");
  else if(s.isInf())
    e=MBXMLUtils::D(doc)->createElement(CASADI%"InfSX");
  else if(s.isMinusInf())
    e=MBXMLUtils::D(doc)->createElement(CASADI%"MinusInfSX");
  else if(s.isNan())
    e=MBXMLUtils::D(doc)->createElement(CASADI%"NanSX");
  else if(s.isInteger()) {
    e=MBXMLUtils::D(doc)->createElement(CASADI%"IntegerSX");
    std::stringstream str;
    str<<s.getIntValue();
    e->insertBefore(doc->createTextNode(MBXMLUtils::X()%str.str()), NULL);
  }
  else if(s.isConstant()) {
    e=MBXMLUtils::D(doc)->createElement(CASADI%"RealtypeSX");
    std::stringstream str;
    str.precision(18);
    str<<s.getValue();
    e->insertBefore(doc->createTextNode(MBXMLUtils::X()%str.str()), NULL);
  }
  else if(s.hasDep() && s.getNdeps()==2) {
    e=MBXMLUtils::D(doc)->createElement(CASADI%"BinarySX");
    std::stringstream str;
    str<<s.getOp();
    MBXMLUtils::E(e)->setAttribute("op", str.str());
    e->insertBefore(convertCasADiToXML(s.getDep(0), nodes, doc), NULL);
    e->insertBefore(convertCasADiToXML(s.getDep(1), nodes, doc), NULL);
  }
  else if(s.hasDep() && s.getNdeps()==1) {
    e=MBXMLUtils::D(doc)->createElement(CASADI%"UnarySX");
    std::stringstream str;
    str<<s.getOp();
    MBXMLUtils::E(e)->setAttribute("op", str.str());
    e->insertBefore(convertCasADiToXML(s.getDep(0), nodes, doc), NULL);
  }
  else
    throw std::runtime_error("Unknown CasADi::SX type in convertCasADiToXML");

  // write also the id of a newly node to XML
  MBXMLUtils::E(e)->setAttribute("id", idStr);
  return e;
}

inline xercesc::DOMElement* convertCasADiToXML(const SXMatrix &m, std::map<SXNode*, int> &nodes, xercesc::DOMDocument *doc) {
  // if it is a scalar print it as a scalar
  if(m.size1()==1 && m.size2()==1)
    return convertCasADiToXML(m.elem(0, 0), nodes, doc);
  // write each matrixRow of m to XML enclosed by a <matrixRow> element and each element in such rows 
  // to this element
  xercesc::DOMElement *e=MBXMLUtils::D(doc)->createElement(CASADI%"SXMatrix");
  if(m.size1()==1) MBXMLUtils::E(e)->setAttribute("rowVector", "true");
  if(m.size2()==1) MBXMLUtils::E(e)->setAttribute("columnVector", "true");
  for(int r=0; r<m.size1(); r++) {
    xercesc::DOMElement *matrixRow;
    if(m.size1()==1 || m.size2()==1)
      matrixRow=e;
    else {
      matrixRow=MBXMLUtils::D(doc)->createElement(CASADI%"matrixRow");
      e->insertBefore(matrixRow, NULL);
    }
    for(int c=0; c<m.size2(); c++)
      matrixRow->insertBefore(convertCasADiToXML(m.elem(r, c), nodes, doc), NULL);
  }
  return e;
}

inline xercesc::DOMElement* convertCasADiToXML(const SXFunction &f, xercesc::DOMDocument *doc) {
  // write each input of f to XML enclosed by a <inputs> element
  std::map<SXNode*, int> nodes;
  xercesc::DOMElement *e=MBXMLUtils::D(doc)->createElement(CASADI%"SXFunction");
  const std::vector<SXMatrix> &in=f.inputExpr();
  xercesc::DOMElement *input=MBXMLUtils::D(doc)->createElement(CASADI%"inputs");
  e->insertBefore(input, NULL);
  for(size_t i=0; i<in.size(); i++)
    input->insertBefore(convertCasADiToXML(in[i], nodes, doc), NULL);
  // write each output of f to XML enclosed by a <outputs> element
  const std::vector<SXMatrix> &out=f.outputExpr();
  xercesc::DOMElement *output=MBXMLUtils::D(doc)->createElement(CASADI%"outputs");
  e->insertBefore(output, NULL);
  for(size_t i=0; i<out.size(); i++)
    output->insertBefore(convertCasADiToXML(out[i], nodes, doc), NULL);

  return e;
}

inline CasADi::SX createCasADiSXFromXML(xercesc::DOMElement *e, std::map<int, CasADi::SXNode*> &nodes) {
  // creata an SX dependent on the type
  CasADi::SX sx;
  if(MBXMLUtils::E(e)->getTagName()==CASADI%"BinarySX") {
    int op = atoi(MBXMLUtils::E(e)->getAttribute("op").c_str());
    xercesc::DOMElement *ee=e->getFirstElementChild();
    CasADi::SX dep0=createCasADiSXFromXML(ee, nodes);
    ee=ee->getNextElementSibling();
    CasADi::SX dep1=createCasADiSXFromXML(ee, nodes);
    sx=CasADi::SX::binary(op, dep0, dep1);
  }
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"UnarySX") {
    int op = atoi(MBXMLUtils::E(e)->getAttribute("op").c_str());
    xercesc::DOMElement *ee=e->getFirstElementChild();
    CasADi::SX dep=createCasADiSXFromXML(ee, nodes);
    sx=CasADi::SX::unary(op, dep);
  }
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"SymbolicSX") {
    sx=CasADi::SX(MBXMLUtils::X()%MBXMLUtils::E(e)->getFirstTextChild()->getData());
  }
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"RealtypeSX") {
    std::stringstream str(MBXMLUtils::X()%MBXMLUtils::E(e)->getFirstTextChild()->getData());
    double value;
    str>>value;
    sx=value;
  }
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"IntegerSX") {
    std::stringstream str(MBXMLUtils::X()%MBXMLUtils::E(e)->getFirstTextChild()->getData());
    int value;
    str>>value;
    sx=value;
  }
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"ZeroSX")
    sx=CasADi::casadi_limits<CasADi::SX>::zero;
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"OneSX")
    sx=CasADi::casadi_limits<CasADi::SX>::one;
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"MinusOneSX")
    sx=CasADi::casadi_limits<CasADi::SX>::minus_one;
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"InfSX")
    sx=CasADi::casadi_limits<CasADi::SX>::inf;
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"MinusInfSX")
    sx=CasADi::casadi_limits<CasADi::SX>::minus_inf;
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"NanSX")
    sx=CasADi::casadi_limits<CasADi::SX>::nan;
  // reference elements must be handled specially: return the referenced node instead of creating a new one
  else if(MBXMLUtils::E(e)->getTagName()==CASADI%"reference") {
    int refid = atoi(MBXMLUtils::E(e)->getAttribute("refid").c_str());
    sx=CasADi::SX(nodes[refid], false);
    return sx;
  }
  else
    throw std::runtime_error("Unknown XML element named "+MBXMLUtils::X()%e->getTagName()+" in createCasADiSXFromXML");

  // insert a newly created SX (node) to the list of all nodes
  int id = atoi(MBXMLUtils::E(e)->getAttribute("id").c_str());
  nodes.insert(std::make_pair(id, sx.get()));
  return sx;
}

inline CasADi::SXMatrix createCasADiSXMatrixFromXML(xercesc::DOMElement *e, std::map<int, CasADi::SXNode*> &nodes) {
  // create a SXMatrix
  if(MBXMLUtils::E(e)->getTagName()==CASADI%"SXMatrix") {
    // loop over all rows
    std::vector<std::vector<CasADi::SX> > ret;
    xercesc::DOMElement *matrixRow=e->getFirstElementChild();
    while(matrixRow) {
      // loop over all elements in a matrixRow
      std::vector<CasADi::SX> stdrow;
      xercesc::DOMElement *matrixEle;
      if((MBXMLUtils::E(e)->hasAttribute("rowVector") && MBXMLUtils::E(e)->getAttribute("rowVector")=="true") ||
         (MBXMLUtils::E(e)->hasAttribute("columnVector") && MBXMLUtils::E(e)->getAttribute("columnVector")=="true"))
        matrixEle=matrixRow;
      else
        matrixEle=matrixRow->getFirstElementChild();
      while(matrixEle) {
        stdrow.push_back(createCasADiSXFromXML(matrixEle, nodes));
        matrixEle=matrixEle->getNextElementSibling();
      }
      if((MBXMLUtils::E(e)->hasAttribute("rowVector") && MBXMLUtils::E(e)->getAttribute("rowVector")=="true") ||
         (MBXMLUtils::E(e)->hasAttribute("columnVector") && MBXMLUtils::E(e)->getAttribute("columnVector")=="true"))
        matrixRow=matrixEle;
      else
        matrixRow=matrixRow->getNextElementSibling();
      ret.push_back(stdrow);
    }
    if(MBXMLUtils::E(e)->hasAttribute("columnVector") && MBXMLUtils::E(e)->getAttribute("columnVector")=="true")
      return CasADi::SXMatrix(ret).trans();
    else
      return CasADi::SXMatrix(ret);
  }
  // if it is a scalar create a SX
  else
    return createCasADiSXFromXML(e, nodes);
}

inline CasADi::SXFunction createCasADiSXFunctionFromXML(xercesc::DOMElement *e) {
  // create a SXFunction
  std::map<int, CasADi::SXNode*> nodes;
  if(MBXMLUtils::E(e)->getTagName()==CASADI%"SXFunction") {
    // get all inputs
    std::vector<CasADi::SXMatrix> in;
    xercesc::DOMElement *input=e->getFirstElementChild();
    xercesc::DOMElement *inputEle=input->getFirstElementChild();
    while(inputEle) {
      in.push_back(createCasADiSXMatrixFromXML(inputEle, nodes));
      inputEle=inputEle->getNextElementSibling();
    }
    // get all outputs
    std::vector<CasADi::SXMatrix> out;
    xercesc::DOMElement *output=input->getNextElementSibling();
    xercesc::DOMElement *outputEle=output->getFirstElementChild();
    while(outputEle) {
      out.push_back(createCasADiSXMatrixFromXML(outputEle, nodes));
      outputEle=outputEle->getNextElementSibling();
    }

    return CasADi::SXFunction(in, out);
  }
  else
    throw std::runtime_error("Unknown XML element named "+MBXMLUtils::X()%e->getTagName()+" in createCasADiSXFunctionFromXML.");
}

}

#endif