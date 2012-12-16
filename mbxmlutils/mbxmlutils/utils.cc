#include <mbxmlutils/utils.h>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "mbxmlutilstinyxml/tinynamespace.h"
#include <octave/parse.h>

using namespace std;

namespace MBXMLUtils {

int getMaschineNumDigits() {
  static int machinePrec=0;
  if(machinePrec>0)
    return machinePrec;
  
  // calcaulate machine precision
  double machineEps;
  for(machineEps=1.0; (1.0+machineEps)>1.0; machineEps*=0.5);
  machineEps*=2.0;
  machinePrec=(int)(-log(machineEps)/log(10))+1;
  return machinePrec;
}

static int orgstderr;
static streambuf *orgcerr;
static int disableCount=0;
// disable output off stderr (including stack)
static void disable_stderr() {
  if(disableCount==0) {
    orgcerr=std::cerr.rdbuf(0);
    orgstderr=dup(fileno(stderr));
#ifdef _WIN32
    if(freopen("nul", "w", stderr)==0) throw(1);
#else
    if(freopen("/dev/null", "w", stderr)==0) throw(1);
#endif
  }
  disableCount++;
}
// enable output off stderr (including stack)
static void enable_stderr() {
  disableCount--;
  if(disableCount==0) {
    std::cerr.rdbuf(orgcerr);
    dup2(orgstderr, fileno(stderr));
    close(orgstderr);
  }
}



OctaveEvaluator::OctaveEvaluator() {
  symbol_table::clear_variables();
}

// add a parameter to the parameter list (used by octavePushParam and octavePopParams)
void OctaveEvaluator::octaveAddParam(const string &paramName, const octave_value& value, bool useCache) {
  // add paramter to parameter list if a parameter of the same name dose not exist in the list
  currentParam[paramName]=value;
  if(useCache) {
    if(paramStack.size()>=currentParamHash.size()) {
      currentParamHash.resize(paramStack.size()+1);
      currentParamHash[paramStack.size()]=0;
    }
    currentParamHash[paramStack.size()]++;
  }
}

// push all parameters from list to a parameter stack
void OctaveEvaluator::octavePushParams() {
  paramStack.push(currentParam);
}

// pop all parameters from list from the parameter stack
void OctaveEvaluator::octavePopParams() {
  // restore previous parameter list
  currentParam=paramStack.top();
  paramStack.pop();
}

// evaluate a single statement or a statement list and save the result in the variable 'ret'
#define PATHLENGTH 10240
void OctaveEvaluator::octaveEvalRet(string str, TiXmlElement *e, bool useCache) {
  // delete leading new lines in str
  for(unsigned int j=0; j<str.length() && (str[j]==' ' || str[j]=='\n' || str[j]=='\t'); j++)
    str[j]=' ';
  // delete trailing new lines in str
  for(unsigned int j=str.length()-1; j>=0 && (str[j]==' ' || str[j]=='\n' || str[j]=='\t'); j--)
    str[j]=' ';

  pair<unordered_map<string, octave_value>::iterator, bool> ins;
  if(useCache) {
    // a cache: this cache is only unique per input file on the command line.
    // Hence this cache must be cleared after/before each input file. Hence this cache is moved to a global variable.
    if(paramStack.size()>=currentParamHash.size()) {
      currentParamHash.resize(paramStack.size()+1);
      currentParamHash[paramStack.size()]=0;
    }
    stringstream s; s<<paramStack.size()<<";"<<currentParamHash[paramStack.size()]<<";"; string id=s.str();
    ins=cache.insert(pair<string, octave_value>(id+str, octave_value()));
    if(!ins.second) {
      symbol_table::varref("ret")=ins.first->second;
      return;
    }
  }

  // clear octave
  symbol_table::clear_variables();
  // restore current parameters
  for(map<string, octave_value>::iterator i=currentParam.begin(); i!=currentParam.end(); i++)
    symbol_table::varref(i->first)=i->second;

  char savedPath[PATHLENGTH];
  if(e) { // set working dir to path of current file, so that octave works with correct relative paths
    if(getcwd(savedPath, PATHLENGTH)==0) throw(1);
    if(chdir(fixPath(TiXml_GetElementWithXmlBase(e,0)->Attribute("xml:base"),".").c_str())!=0) throw(1);
  }

  int dummy;
  MBXMLUtils::disable_stderr();
  try{
    eval_string("ret="+str,true,dummy); // eval as single statement, and save in 'ret'
  }
  catch(...) {
    error_state=1;
  }
  MBXMLUtils::enable_stderr();
  if(error_state!=0) { // if error, maybe it is a statement list
    error_state=0;
    try {
      eval_string(str,true,dummy,0); // eval as statement list
    }
    catch(...) {
      error_state=1;
    }
    if(error_state!=0) { // if error => wrong code => throw error
      error_state=0;
      if(e) if(chdir(savedPath)!=0) throw(1);
      throw string("Error in octave expression: "+str);
    }
    if(!symbol_table::is_variable("ret")) {
      cout<<"ERRRO\n";
      error_state=0;
      if(e) if(chdir(savedPath)!=0) throw(1);
      throw string("'ret' not defined in octave statement list: "+str);
    }
  }
  if(e) if(chdir(savedPath)!=0) throw(1);

  if(useCache)
    ins.first->second=symbol_table::varval("ret"); // add to cache
}

void OctaveEvaluator::checkType(const octave_value& val, ValueType expectedType) {
  ValueType type;
  // get type of val
  if(val.is_scalar_type() && val.is_real_type() && (val.is_string()!=1))
    type=ScalarType;
  else if(val.is_matrix_type() && val.is_real_type() && (val.is_string()!=1)) {
    Matrix m=val.matrix_value();
    type=m.cols()==1?VectorType:MatrixType;
  }
  else if(val.is_string())
    type=StringType;
  else // throw on unknown type
    throw(string("Unknown type: none of scalar, vector, matrix or string"));
  // check for correct type
  if(expectedType!=ArbitraryType) {
    if(type==ScalarType && expectedType==StringType)
      throw string("Got scalar value, while a string is expected");
    if(type==VectorType && (expectedType==StringType || expectedType==ScalarType)) 
      throw string("Got column-vector value, while a ")+(expectedType==StringType?"string":"scalar")+" is expected";
    if(type==MatrixType && (expectedType==StringType || expectedType==ScalarType || expectedType==VectorType))
      throw string("Got matrix value, while a ")+(expectedType==StringType?"string":(expectedType==ScalarType?"scalar":"column-vector"))+" is expected";
    if(type==StringType && (expectedType==MatrixType || expectedType==ScalarType || expectedType==VectorType))
      throw string("Got string value, while a ")+(expectedType==MatrixType?"matrix":(expectedType==ScalarType?"scalar":"column-vector"))+" is expected";
  }
}

// return the value of 'ret'
string OctaveEvaluator::octaveGetRet(ValueType expectedType) {
  octave_value o=symbol_table::varval("ret"); // get 'ret'

  ostringstream ret;
  ret.precision(MBXMLUtils::getMaschineNumDigits());
  if(o.is_scalar_type() && o.is_real_type() && (o.is_string()!=1)) {
    ret<<o.double_value();
  }
  else if(o.is_matrix_type() && o.is_real_type() && (o.is_string()!=1)) {
    Matrix m=o.matrix_value();
    ret<<"[";
    for(int i=0; i<m.rows(); i++) {
      for(int j=0; j<m.cols(); j++)
        ret<<m(j*m.rows()+i)<<(j<m.cols()-1?",":"");
      ret<<(i<m.rows()-1?" ; ":"]");
    }
  }
  else if(o.is_string()) {
    ret<<"\""<<o.string_value()<<"\"";
  }
  else { // if not scalar, matrix or string => error
    throw(string("Unknown type: none of scalar, vector, matrix or string"));
  }
  checkType(o, expectedType);

  return ret.str();
}

// fill octave with parameters
int OctaveEvaluator::fillParam(TiXmlElement *e, bool useCache) {
  // generate a vector of parameters
  vector<Param> param;
  for(TiXmlElement *ee=e->FirstChildElement(); ee!=0; ee=ee->NextSiblingElement())
    param.push_back(Param(ee->Attribute("name"), ee->GetText(), ee));
  return fillParam(param, useCache);
}

int OctaveEvaluator::fillParam(vector<Param> param, bool useCache) {
  // outer loop to resolve recursive parameters
  size_t length=param.size();
  for(size_t outerLoop=0; outerLoop<length; outerLoop++)
    // evaluate parameter
    for(vector<Param>::iterator i=param.begin(); i!=param.end(); i++) {
      MBXMLUtils::disable_stderr();
      int err=0;
      octave_value ret;
      try { 
        octaveEvalRet(i->equ, i->ele, useCache);
        ret=symbol_table::varval("ret");
        if(i->ele)
          checkType(ret, i->ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}scalarParameter"?ScalarType:
                         i->ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}vectorParameter"?VectorType:
                         i->ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}matrixParameter"?MatrixType:
                         i->ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}stringParameter"?StringType:ArbitraryType);
      }
      catch(...) { err=1; }
      MBXMLUtils::enable_stderr();
      if(err==0) { // if no error
        octaveAddParam(i->name, ret, useCache); // add param to list
        vector<Param>::iterator isave=i-1; // delete param from vector
        param.erase(i);
        i=isave;
      }
    }
  if(param.size()>0) { // if parameters are left => error
    cout<<"Error in one of the following parameters or infinit loop in this parameters:\n";
    for(size_t i=0; i<param.size(); i++) {
      try {
        octaveEvalRet(param[i].equ, param[i].ele, useCache); // output octave error
        octave_value ret=symbol_table::varval("ret");
        if(param[i].ele)
          checkType(ret, param[i].ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}scalarParameter"?ScalarType:
                         param[i].ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}vectorParameter"?VectorType:
                         param[i].ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}matrixParameter"?MatrixType:
                         param[i].ele->ValueStr()=="{http://openmbv.berlios.de/MBXMLUtils/parameter}stringParameter"?StringType:ArbitraryType);
      }
      catch(string str) { cout<<str<<endl; }
      if(param[i].ele) TiXml_location(param[i].ele, "", ": "+param[i].name+": "+param[i].equ); // output location of element
    }
    return 1;
  }

  return 0;
}

void OctaveEvaluator::saveAndClearCurrentParam() {
  savedCurrentParam=currentParam; // save parameters
  currentParam.clear(); // clear parameters
}

void OctaveEvaluator::restoreCurrentParam() {
  currentParam=savedCurrentParam; // restore parameter
}



} // end namespace MBXMLUtils