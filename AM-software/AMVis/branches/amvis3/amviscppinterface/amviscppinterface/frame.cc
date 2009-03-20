#include <amviscppinterface/frame.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace AMVis;

Frame::Frame() : RigidBody(),
  size(1), offset(0) {
}

void Frame::writeXMLFile(std::ofstream& xmlFile, const std::string& indent) {
  xmlFile<<indent<<"<Frame name=\""<<name<<"\">"<<endl;
    RigidBody::writeXMLFile(xmlFile, indent+"  ");
    xmlFile<<indent<<"  <size>"<<size<<"</size>"<<endl;
    xmlFile<<indent<<"  <offset>"<<size<<"</offset>"<<endl;
  xmlFile<<indent<<"</Frame>"<<endl;
}
