/* Copyright (C) 2004-2009 OpenMBV Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: friedrich.at.gc@googlemail.com
 */

#include "config.h"
#include <cstdlib>
#include "utils.h"
#include "dom.h"
#include <boost/functional/hash.hpp> //  boost::hash can hash a std::pair but std::hash cannot
#include <regex>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBXMLUtils {

  set<size_t> Deprecated::printedMessages;

  void Deprecated::message(ostream &str, const string &msg, const DOMElement *e) {
    // create the full deprecated message (including a trace)
    string msg2;
    if(e)
      msg2=DOMEvalException("", e).what();
    else
      // MISSING get a stacktrace here. e.g. using boost::backtrace if its available
      msg2="(no stack trace available)";
    // create a hash of the message and ...
    boost::hash<pair<ostream*, string> > messageHash;
    if(printedMessages.insert(messageHash(make_pair(&str, msg+"\n"+msg2))).second)
      // ... print the message if it is not already printed
      str<<"Deprecated feature called:"<<endl<<msg<<endl<<msg2<<endl;
  }

  void setupMessageStreams(std::list<std::string> &args, bool forcePlainOutput) {
#ifdef _WIN32
    bool stdoutIsTTY=GetFileType(GetStdHandle(STD_OUTPUT_HANDLE))==FILE_TYPE_CHAR;
#else
    bool stdoutIsTTY=isatty(1)==1;
#endif
    // defaults for --stdout and --stderr
    if(find(args.begin(), args.end(), "--stdout")==args.end() &&
       find(args.begin(), args.end(), "--stderr")==args.end()) {
      if(stdoutIsTTY && !forcePlainOutput) {
        args.emplace_back("--stdout"); args.emplace_back(  "info~\x1b[KInfo: ~");
        args.emplace_back("--stderr"); args.emplace_back(  "warn~\x1b[KWarn: ~");
        args.emplace_back("--stderr"); args.emplace_back( "error~\x1b[K~");
        args.emplace_back("--stderr"); args.emplace_back(  "depr~\x1b[KDepr: ~");
        args.emplace_back("--stdout"); args.emplace_back("status~\x1b[K~\r");
      }
      else {
        args.emplace_back("--stdout"); args.emplace_back(  "info~Info: ~");
        args.emplace_back("--stderr"); args.emplace_back(  "warn~Warn: ~");
        args.emplace_back("--stderr"); args.emplace_back( "error~~");
        args.emplace_back("--stderr"); args.emplace_back(  "depr~Depr: ~");
        args.emplace_back("--stdout"); args.emplace_back("status~~\r");
      }
    }
  
    // disable all streams
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Info      , std::make_shared<bool>(false));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Warn      , std::make_shared<bool>(false));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Debug     , std::make_shared<bool>(false));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Error     , std::make_shared<bool>(false));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Deprecated, std::make_shared<bool>(false));
    fmatvec::Atom::setCurrentMessageStream(fmatvec::Atom::Status    , std::make_shared<bool>(false));
  
    // handle --stdout and --stderr args
    list<string>::iterator it;
    while((it=find_if(args.begin(), args.end(), [](const string &x){ return x=="--stdout" || x=="--stderr"; }))!=args.end()) {
      ostream &ostr=*it=="--stdout"?cout:cerr;
      auto itn=next(it);
      if(itn==args.end())
        throw runtime_error("Invalid argument: "+*it+" "+*itn);
      fmatvec::Atom::MsgType msgType;
      if     (itn->substr(0, 5)=="info~"  ) msgType=fmatvec::Atom::Info;
      else if(itn->substr(0, 5)=="warn~"  ) msgType=fmatvec::Atom::Warn;
      else if(itn->substr(0, 6)=="debug~" ) msgType=fmatvec::Atom::Debug;
      else if(itn->substr(0, 6)=="error~" ) msgType=fmatvec::Atom::Error;
      else if(itn->substr(0, 5)=="depr~"  ) msgType=fmatvec::Atom::Deprecated;
      else if(itn->substr(0, 7)=="status~") msgType=fmatvec::Atom::Status;
      else throw runtime_error("Unknown message stream.");
      static std::regex re(".*~(.*)~(.*)", std::regex::extended);
      std::smatch m;
      if(!std::regex_match(*itn, m, re))
        throw runtime_error("Invalid argument: "+*it+" "+*itn);
      fmatvec::Atom::setCurrentMessageStream(msgType, std::make_shared<bool>(true),
        std::make_shared<fmatvec::PrePostfixedStream>(m.str(1), m.str(2), ostr));
  
      args.erase(itn);
      args.erase(it);
    }
  }

}
