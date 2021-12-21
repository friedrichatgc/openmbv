/*
    OpenMBV - Open Multi Body Viewer.
    Copyright (C) 2009 Markus Friedrich

  This library is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version. 
   
  This library is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details. 
   
  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

#ifndef _OPENMBVGUI_PATH_H_
#define _OPENMBVGUI_PATH_H_

#include "body.h"
#include <Inventor/C/errors/debugerror.h> // workaround a include order bug in Coin-3.1.3
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoLineSet.h>
#include <hdf5serie/vectorserie.h>

namespace OpenMBV {
  class Path;
}

namespace OpenMBVGUI {

class Path : public Body {
  Q_OBJECT
  protected:
    double update() override;
    SoCoordinate3 *coord;
    SoLineSet *line;
    int maxFrameRead;
    std::shared_ptr<OpenMBV::Path> path;
    void createProperties() override;
  public:
    Path(const std::shared_ptr<OpenMBV::Object> &obj, QTreeWidgetItem *parentItem, SoGroup *soParent, int ind);
    QString getInfo() override;
};

}

#endif
