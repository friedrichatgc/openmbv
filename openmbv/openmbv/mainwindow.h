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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "config.h"
#include <QtGui/QMainWindow>
#include <QtGui/QTreeWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QActionGroup>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>
#include <QWebView>
#include <QTimer>
#include <QTime>
#include <string>
#include "body.h"
#include "SoSpecial.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/engines/SoTransformVec3f.h>
#include "SoTransposeEngine.h"
#include <Inventor/fields/SoSFRotation.h>
#include <FXViz/nodes/SoShadowGroup.h>
#include <Inventor/SoOffscreenRenderer.h>
#include "SoQtMyViewer.h"
#ifdef HAVE_QWT_WHEEL_H
#  include <qwt_wheel.h>
#else
#  include <QtGui/QSlider>
#endif

class MainWindow : public QMainWindow {
  Q_OBJECT
  friend class Body;
  private:
    static MainWindow *instance;
    enum ViewSide { top, bottom, front, back, right, left, isometric, dimetric,
                    rotateXpWorld, rotateXmWorld, rotateYpWorld, rotateYmWorld, rotateZpWorld, rotateZmWorld,
                    rotateXpScreen, rotateXmScreen, rotateYpScreen, rotateYmScreen, rotateZpScreen, rotateZmScreen };
    enum Mode { no, rotate, translate, zoom };
    enum Animation { stop, play, lastFrame };
    struct WindowState { bool hasMenuBar, hasStatusBar, hasFrameSlider; };
    Mode mode;
    SoText2 *timeString;
    double fpsMax;
    QWebView *helpViewerGUI, *helpViewerXML;
    bool enableFullScreen;
    SoTransformVec3f *cameraPosition;
    SoTransposeEngine *cameraOrientation;
    SoSwitch *worldFrameSwitch;
    SoSwitch *engDrawing;
    SoDrawStyle *olseDrawStyle;
    SoBaseColorHeavyOverride *olseColor;
  protected:
    SoSepNoPickNoBBox *sceneRootBBox;
    QTreeWidget *objectList;
    QTextEdit *objectInfo;
    QSpinBox *frameSB;
    bool openFile(std::string fileName);
    SoQtMyViewer *glViewer;
    void viewChange(ViewSide side);
    SoShadowGroup *sceneRoot;
    SoComplexity *complexity;
    QTimer *animTimer;
    QTime *time;
    QDoubleSpinBox *speedSB;
    int animStartFrame;
    QActionGroup *animGroup;
    QSlider *timeSlider;
    double deltaTime;
    SoSFUInt32 *frame;
    QLabel *fps;
    QTime *fpsTime;
#ifdef HAVE_QWT_WHEEL_H
    QwtWheel *speedWheel;
#else
    QSlider *speedWheel;
#endif
    double oldSpeed;
    QAction *stopAct, *lastFrameAct, *playAct, *toggleMenuBar, *toggleStatusBar, *toggleFrameSlider, *toggleFullScreen, *toggleDecoration;
    QAction *engDrawingView, *topBGColorAct, *bottomBGColorAct;
    SoMFColor *bgColor, *fgColorTop, *fgColorBottom;
    void help(std::string type, QDialog *helpDialog);
    QLineEdit *filter;
    int mySearch(const QRegExp& filterRegExp, Object *item);
  protected slots:
    void objectListClicked();
    void openFileDialog();
    void guiHelp();
    void xmlHelp();
    void aboutOpenMBV();
    void updateFrame(int frame_) { frame->setValue(frame_); }
    void viewAllSlot() { glViewer->viewAll(); }
    void toggleCameraTypeSlot() { glViewer->toggleCameraType(); }
    void releaseCameraFromBodySlot();
    void showWorldFrameSlot();
    void shadowRenderingSlot();

    void viewTopSlot() { viewChange(top); }
    void viewBottomSlot() { viewChange(bottom); }
    void viewFrontSlot() { viewChange(front); }
    void viewBackSlot() { viewChange(back); }
    void viewRightSlot() { viewChange(right); }
    void viewLeftSlot() { viewChange(left); }
    void viewIsometricSlot() { viewChange(isometric); }
    void viewDimetricSlot() { viewChange(dimetric); }
    void viewRotateXpWorld() { viewChange(rotateXpWorld); }
    void viewRotateXmWorld() { viewChange(rotateXmWorld); }
    void viewRotateYpWorld() { viewChange(rotateYpWorld); }
    void viewRotateYmWorld() { viewChange(rotateYmWorld); }
    void viewRotateZpWorld() { viewChange(rotateZpWorld); }
    void viewRotateZmWorld() { viewChange(rotateZmWorld); }
    void viewRotateXpScreen() { viewChange(rotateXpScreen); }
    void viewRotateXmScreen() { viewChange(rotateXmScreen); }
    void viewRotateYpScreen() { viewChange(rotateYpScreen); }
    void viewRotateYmScreen() { viewChange(rotateYmScreen); }
    void viewRotateZpScreen() { viewChange(rotateZpScreen); }
    void viewRotateZmScreen() { viewChange(rotateZmScreen); }

    void setObjectInfo(QTreeWidgetItem* current) { if(current) objectInfo->setHtml(((Object*)current)->getInfo()); }
    void frameSBSetRange(int min, int max) { frameSB->setRange(min, max); } // because QAbstractSlider::setRange is not a slot
    void heavyWorkSlot();
    void speedChanged(double value);
    void speedWheelChangedD(double value) { speedWheelChanged((int)value); }
    void speedWheelChanged(int value);
    void speedWheelPressed();
    void speedWheelReleased();
    void exportAsPNG(SoOffscreenRenderer &myrendere, std::string fileName, bool transparent);
    void exportCurrentAsPNG();
    void exportSequenceAsPNG();
    void exportCurrentAsIV();
    void loadUrlGUI(const QUrl &url); // a workaround for Qt bug N261352
    void loadUrlXML(const QUrl &url); // a workaround for Qt bug N261352
    void helpHomeXML();
    void helpHomeGUI();
    void stopSCSlot();
    void lastFrameSCSlot();
    void playSCSlot();
    void speedUpSlot();
    void speedDownSlot();
    void topBGColor();
    void bottomBGColor();
    void olseColorSlot();
    void olseLineWidthSlot();
    void loadWindowState(std::string filename="");
    void saveWindowState();
    void loadCamera(std::string filename="");
    void saveCamera();
    void toggleMenuBarSlot();
    void toggleStatusBarSlot();
    void toggleFrameSliderSlot();
    void toggleFullScreenSlot();
    void toggleDecorationSlot();
    void filterObjectList();
    void searchObjectList(Object *item, const QRegExp&);
    void collapseItem(QTreeWidgetItem* item);
    void expandItem(QTreeWidgetItem* item);
    void expandToDepth1() { 
      for(int i=0; i<objectList->invisibleRootItem()->childCount(); i++)
        objectList->invisibleRootItem()->child(i)->setExpanded(false); 
    }
    void expandToDepth2() { objectList->expandToDepth(0); }
    void expandToDepth3() { objectList->expandToDepth(1); }
    void expandToDepth4() { objectList->expandToDepth(2); }
    void expandToDepth5() { objectList->expandToDepth(3); }
    void expandToDepth6() { objectList->expandToDepth(4); }
    void expandToDepth7() { objectList->expandToDepth(5); }
    void expandToDepth8() { objectList->expandToDepth(6); }
    void expandToDepth9() { objectList->expandToDepth(7); }
    void toggleEngDrawingViewSlot();
    void setOutLineAndShilouetteEdgeRecursive(QTreeWidgetItem *obj, bool enableOutLine, bool enableShilouetteEdge);
    void complexityType();
    void complexityValue();
  public:
    MainWindow(std::list<std::string>& arg);
    void updateScene() { glViewer->getSceneManager()->render(); }
    static MainWindow*const getInstance() { return instance; }
    bool soQtEventCB(const SoEvent *const event);
    static void frameSensorCB(void *data, SoSensor*);
    void fpsCB();
    SoSepNoPickNoBBox *getSceneRootBBox() { return sceneRootBBox; }
    QSlider *getTimeSlider() { return timeSlider; }
    double &getDeltaTime() { return deltaTime; }
    double getSpeed() { return speedSB->value(); }
    SoSFUInt32 *getFrame() { return frame; }
    void setTime(double t) { timeString->string.setValue(QString("Time: %2").arg(t,0,'f',5).toStdString().c_str()); }
    SoText2 *getTimeString() { return timeString; }
    SoMFColor *getBgColor() { return bgColor; }
    SoMFColor *getFgColorTop() { return fgColorTop; }
    SoMFColor *getFgColorBottom() { return fgColorBottom; }
    bool getEnableFullScreen() { return enableFullScreen; }
    void moveCameraWith(SoSFVec3f *pos, SoSFRotation *rot);
    SoDrawStyle* getOlseDrawStyle() { return olseDrawStyle; }
    SoBaseColorHeavyOverride* getOlseColor() { return olseColor; }
};

#endif
