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
#include "openmbvcppinterface/compoundrigidbody.h"

using namespace std;

CompoundRigidBody::CompoundRigidBody(OpenMBV::Object *obj, QTreeWidgetItem *parentItem, SoGroup *soParent, int ind) : RigidBody(obj, parentItem, soParent, ind) {
  OpenMBV::CompoundRigidBody* crb=(OpenMBV::CompoundRigidBody*)obj;
  iconFile=":/compoundrigidbody.svg";
  setIcon(0, Utils::QIconCached(iconFile.c_str()));

  // read XML
  vector<OpenMBV::RigidBody*> rb=crb->getRigidBodies();
  for(size_t i=0; i<rb.size(); i++)
    rigidBody.push_back((RigidBody*)ObjectFactory(rb[i], this, soSepRigidBody, ind));

  // create so

  // outline
}

CompoundRigidBody::~CompoundRigidBody() {
  for(size_t i=0; i<rigidBody.size(); i++)
    delete rigidBody[i];
}