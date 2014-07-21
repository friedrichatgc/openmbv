/*
    OpenMBV - Open Multi Body Viewer.
    Copyright (C) 2009 Markus Friedrich

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"
#include <openmbvcppinterface/path.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace MBXMLUtils;

namespace OpenMBV {

OPENMBV_OBJECTFACTORY_REGISTERXMLNAME(Path, OPENMBV%"Path")

Path::Path() : Body(), data(NULL), color(vector<double>(3,1)) {
}

Path::~Path() {
}

xercesc::DOMElement* Path::writeXMLFile(xercesc::DOMNode *parent) {
  xercesc::DOMElement *e=Body::writeXMLFile(parent);
  addElementText(e, OPENMBV%"color", color);
  return 0;
}

void Path::createHDF5File() {
  Body::createHDF5File();
  if(!hdf5LinkBody) {
    data=hdf5Group->createChildObject<H5::VectorSerie<double> >("data")(4);
    vector<string> columns;
    columns.push_back("Time");
    columns.push_back("x");
    columns.push_back("y");
    columns.push_back("z");
    data->setColumnLabel(columns);
  }
}

void Path::openHDF5File() {
  Body::openHDF5File();
  if(!hdf5Group) return;
  if(!hdf5LinkBody) {
    try {
      data=hdf5Group->openChildObject<H5::VectorSerie<double> >("data");
    }
    catch(...) {
      data=NULL;
      msg(Warn)<<"Unable to open the HDF5 Dataset 'data'"<<endl;
    }
  }
}

void Path::initializeUsingXML(xercesc::DOMElement *element) {
  Body::initializeUsingXML(element);
  xercesc::DOMElement *e;
  e=E(element)->getFirstElementChildNamed(OPENMBV%"color");
  setColor(getVec(e,3));
}

}
