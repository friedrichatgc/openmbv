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
#include "dynamiccoloredbody.h"
#include "openmbvcppinterface/dynamiccoloredbody.h"
#include "utils.h"

using namespace std;

DynamicColoredBody::DynamicColoredBody(OpenMBV::Object *obj, H5::Group *h5Parent, QTreeWidgetItem *parentItem, SoGroup *soParent) : Body(obj, h5Parent, parentItem, soParent), color(0), oldColor(nan("")) {
  OpenMBV::DynamicColoredBody *dcb=(OpenMBV::DynamicColoredBody*)obj;
  // read XML
  minimalColorValue=dcb->getMinimalColorValue();
  maximalColorValue=dcb->getMaximalColorValue();
  staticColor=dcb->getStaticColor();
}

void DynamicColoredBody::setColor(SoMaterial *mat, double col) {
  if(oldColor!=col) {
    color=col;
    oldColor=col;
    double m=1/(maximalColorValue-minimalColorValue);
    col=m*col-m*minimalColorValue;
    if(col<0) col=0;
    if(col>1) col=1;
    mat->diffuseColor.setHSVValue((1-col)*2/3,1,1);
    mat->specularColor.setHSVValue((1-col)*2/3,0.7,1);
  }
}

QString DynamicColoredBody::getInfo() {
  return Body::getInfo()+
         QString("-----<br/>")+
         QString("<b>Color:</b> %1<br/>").arg(getColor());
}

