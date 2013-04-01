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
#include "body.h"
#include "utils.h"
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoTriangleStripSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoLightModel.h>
#include "SoSpecial.h"
#include <QtGui/QMenu>
#include "mainwindow.h"
#include "compoundrigidbody.h"
#include <GL/gl.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/SoPrimitiveVertex.h>
#include "utils.h"

using namespace std;

bool Body::existFiles=false;
Body *Body::timeUpdater=0;

Body::Body(TiXmlElement *element, H5::Group *h5Parent, QTreeWidgetItem *parentItem, SoGroup *soParent) : Object(element, h5Parent, parentItem, soParent) {
  if(h5Parent) {
    // read XML
    TiXmlElement *e=element->FirstChildElement(OPENMBVNS"hdf5Link");
    if(e); // hdf5Link

    // register callback function on frame change
    SoFieldSensor *sensor=new SoFieldSensor(frameSensorCB, this);
    sensor->attach(MainWindow::getInstance()->getFrame());
    sensor->setPriority(0); // is needed for png export
  }

  // switch for outline
  soOutLineSwitch=new SoSwitch;
  soOutLineSwitch->ref(); // add to scene must be done by derived class
  if(dynamic_cast<CompoundRigidBody*>(parentItem)==0)
    soOutLineSwitch->whichChild.setValue(SO_SWITCH_ALL);
  else
    soOutLineSwitch->whichChild.connectFrom(&((Body*)parentItem)->soOutLineSwitch->whichChild);
  soOutLineSep=new SoSeparator;
  soOutLineSwitch->addChild(soOutLineSep);
  SoLightModel *lm=new SoLightModel;
  soOutLineSep->addChild(lm);
  lm->model.setValue(SoLightModel::BASE_COLOR);
  SoBaseColor *color=new SoBaseColor;
  soOutLineSep->addChild(color);
  color->rgb.setValue(0,0,0);
  SoDrawStyle *style=new SoDrawStyle;
  style->style.setValue(SoDrawStyle::LINES);
  soOutLineSep->addChild(style);

  if(dynamic_cast<CompoundRigidBody*>(parentItem)==0) {
    // draw method
    drawStyle=new SoDrawStyle;
    soSep->addChild(drawStyle);
  
    // GUI
    // draw outline action
    outLine=new QAction(Utils::QIconCached(":/outline.svg"),"Draw Out-Line", this);
    outLine->setCheckable(true);
    outLine->setChecked(true);
    outLine->setObjectName("Body::outLine");
    connect(outLine,SIGNAL(changed()),this,SLOT(outLineSlot()));
    // draw method action
    drawMethod=new QActionGroup(this);
    drawMethodPolygon=new QAction(Utils::QIconCached(":/filled.svg"),"Draw Style: Filled", drawMethod);
    drawMethodLine=new QAction(Utils::QIconCached(":/lines.svg"),"Draw Style: Lines", drawMethod);
    drawMethodPoint=new QAction(Utils::QIconCached(":/points.svg"),"Draw Style: Points", drawMethod);
    drawMethodPolygon->setCheckable(true);
    drawMethodPolygon->setData(QVariant(filled));
    drawMethodPolygon->setObjectName("Body::drawMethodPolygon");
    drawMethodLine->setCheckable(true);
    drawMethodLine->setData(QVariant(lines));
    drawMethodLine->setObjectName("Body::drawMethodLine");
    drawMethodPoint->setCheckable(true);
    drawMethodPoint->setData(QVariant(points));
    drawMethodPoint->setObjectName("Body::drawMethodPoint");
    drawMethodPolygon->setChecked(true);
    connect(drawMethod,SIGNAL(triggered(QAction*)),this,SLOT(drawMethodSlot(QAction*)));
  }
}

void Body::frameSensorCB(void *data, SoSensor*) {
  Body* me=(Body*)data;
  double time=0;
  if(me->drawThisPath) 
    time=me->update();
  if(timeUpdater==me)
    MainWindow::getInstance()->setTime(time);
}

QMenu* Body::createMenu() {
  QMenu* menu=Object::createMenu();
  menu->addSeparator()->setText("Properties from: Body");
  menu->addAction(outLine);
  menu->addSeparator();
  menu->addAction(drawMethodPolygon);
  menu->addAction(drawMethodLine);
  menu->addAction(drawMethodPoint);
  return menu;
}

void Body::outLineSlot() {
  if(outLine->isChecked())
    soOutLineSwitch->whichChild.setValue(SO_SWITCH_ALL);
  else
    soOutLineSwitch->whichChild.setValue(SO_SWITCH_NONE);
}

void Body::drawMethodSlot(QAction* action) {
  DrawStyle ds=(DrawStyle)action->data().toInt();
  if(ds==filled)
    drawStyle->style.setValue(SoDrawStyle::FILLED);
  else if(ds==lines)
    drawStyle->style.setValue(SoDrawStyle::LINES);
  else
    drawStyle->style.setValue(SoDrawStyle::POINTS);
}

// number of rows / dt
void Body::resetAnimRange(int numOfRows, double dt) {
  if(numOfRows-1<MainWindow::getInstance()->getTimeSlider()->maximum() || !existFiles) {
    MainWindow::getInstance()->getTimeSlider()->setMaximum(numOfRows-1);
    if(existFiles) {
      QString str("WARNING! Resetting maximal frame number!");
      MainWindow::getInstance()->statusBar()->showMessage(str, 10000);
      cout<<str.toStdString()<<endl;
    }
  }
  if(MainWindow::getInstance()->getDeltaTime()!=dt || !existFiles) {
    MainWindow::getInstance()->getDeltaTime()=dt;
    if(existFiles) {
      QString str("WARNING! dt in HDF5 datas are not the same!");
      MainWindow::getInstance()->statusBar()->showMessage(str, 10000);
      cout<<str.toStdString()<<endl;
    }
  }
  timeUpdater=this;
  existFiles=true;
}