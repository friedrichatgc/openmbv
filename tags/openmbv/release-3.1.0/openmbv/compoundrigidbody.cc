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
#include "compoundrigidbody.h"
#include "objectfactory.h"
#include "utils.h"

using namespace std;

CompoundRigidBody::CompoundRigidBody(TiXmlElement *element, H5::Group *h5Parent, QTreeWidgetItem *parentItem, SoGroup *soParent) : RigidBody(element, h5Parent, parentItem, soParent) {
  iconFile=":/compoundrigidbody.svg";
  setIcon(0, Utils::QIconCached(iconFile.c_str()));

  // read XML
  TiXmlElement *e=element->FirstChildElement(OPENMBVNS"scaleFactor");
  e=e->NextSiblingElement(); // first rigidbody
  while(e!=0) {
    ObjectFactory(e, 0, this, soSepRigidBody);
    e=e->NextSiblingElement(); // next rigidbody
  }

  // create so

  // outline
}