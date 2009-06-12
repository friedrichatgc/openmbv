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

#include <openmbvcppinterface/spineextrusion.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace OpenMBV;

SpineExtrusion::SpineExtrusion() : Body(),
  numberOfSpinePoints(0),
  contour(0),
  data(0), 
  staticColor(-1),
  minimalColorValue(0.),
  maximalColorValue(1.),
  scaleFactor(1) {
  }

  SpineExtrusion::~SpineExtrusion() {
    if(!hdf5LinkBody && data) delete data;
  }

void SpineExtrusion::writeXMLFile(std::ofstream& xmlFile, const std::string& indent) {
  xmlFile<<indent<<"<SpineExtrusion name=\""<<name<<"\">"<<endl;
  Body::writeXMLFile(xmlFile, indent+"  ");
  if(contour) PolygonPoint::serializePolygonPointContour(xmlFile, indent+"  ", contour);
  xmlFile<<indent<<"  <minimalColorValue>"<<minimalColorValue<<"</minimalColorValue>"<<endl;
  xmlFile<<indent<<"  <maximalColorValue>"<<maximalColorValue<<"</maximalColorValue>"<<endl;
  xmlFile<<indent<<"  <scaleFactor>"<<scaleFactor<<"</scaleFactor>"<<endl;
  xmlFile<<indent<<"  <color>"<<staticColor<<"</color>"<<endl;
  xmlFile<<indent<<"</SpineExtrusion>"<<endl;
}

void SpineExtrusion::createHDF5File() {
  Body::createHDF5File();
  if(!hdf5LinkBody) {
    data=new H5::VectorSerie<double>;
    vector<string> columns;
    columns.push_back("Time");
    for(int i=0;i<numberOfSpinePoints;i++) {
      columns.push_back("x"+numtostr(i));
      columns.push_back("y"+numtostr(i));
      columns.push_back("z"+numtostr(i));
      columns.push_back("twist"+numtostr(i));
    }
    data->create(*hdf5Group,"data",columns);
  }
}


void SpineExtrusion::initializeUsingXML(TiXmlElement *element) {
  Body::initializeUsingXML(element);
  TiXmlElement *e;
  e=element->FirstChildElement(OPENMBVNS"minimalColorValue");
  setMinimalColorValue(toVector(e->GetText())[0]);
  e=element->FirstChildElement(OPENMBVNS"maximalColorValue");
  setMaximalColorValue(toVector(e->GetText())[0]);
  e=element->FirstChildElement(OPENMBVNS"scaleFactor");
  setScaleFactor(toVector(e->GetText())[0]);
  e=element->FirstChildElement(OPENMBVNS"color");
  setStaticColor(toVector(e->GetText())[0]);
}