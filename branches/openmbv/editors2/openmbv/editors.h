#ifndef _EDITORS_H_
#define _EDITORS_H_

#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoTranslation.h>
#include <openmbvcppinterface/simpleparameter.h>
#include <openmbvcppinterface/polygonpoint.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>
#include <utils.h>
#include <QDialog>
#include <QCheckBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QAction>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <Inventor/draggers/SoCenterballDragger.h>
#include <Inventor/draggers/SoDragger.h>
#include <Inventor/nodes/SoSwitch.h>

class PropertyDialog;





class PropertyDialog : public QDialog {
  public:
    PropertyDialog();
    void setParentObject(QObject *obj);
    void addSmallRow(const QIcon& icon, const std::string& name, QWidget *widget);
    void addLargeRow(QIcon& icon, const std::string& name, QWidget *widget);
    QObject* getParentObject() { return parentObject; }
  protected:
    QObject* parentObject;
    QGridLayout *layout;
};






class Editor : public QWidget {
  public:
    Editor(PropertyDialog *parent_, const QIcon &icon, const std::string &name);
  protected:
    PropertyDialog *dialog;
    void replaceObject();
};
///*! Base class for all Editors */
//class Editor : public QObject {
//  Q_OBJECT
//
//  public:
//    /*! Constructor.
//     * parent_ should be a of type Object if possible.
//     * icon is displayed by the action.
//     * name_ is displayed by the action (a '...' is added for a WidgetEditor) */
//    Editor(QObject *parent_, const QIcon &icon, const std::string &name_);
//
//    /*! Get the action to activate this Editor */
//    QAction *getAction() { return action; }
//
//  protected:
//    std::string name;
//    QObject *parent;
//    QAction *action;
//    std::string sortName();
//    void replaceObject();
//};





/*! A Editor of type boolean */
class BoolEditor : public Editor {
  Q_OBJECT

  public:
    /*! Constructor. */
    BoolEditor(PropertyDialog *parent_, const QIcon &icon, const std::string &name);

    /*! OpenMBVCppInterface syncronization.
     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface */
    template<class OMBVClass>
    void setOpenMBVParameter(OMBVClass *ombv_, bool (OMBVClass::*getter)(), void (OMBVClass::*setter)(bool));

  protected slots:
    void valueChangedSlot(int);

  protected:
    QCheckBox *checkbox;
    boost::function<bool ()> ombvGetter;
    boost::function<void (bool)> ombvSetter;
};





///*! Base class for all WidgetEditors: a Editor which required a more complex layout than a simple action (like bools) */
//class WidgetEditor : public Editor {
//  Q_OBJECT
//  friend class WidgetEditorCollector;
//
//  public:
//    WidgetEditor(QObject *parent_, const QIcon &icon, const std::string &name);
//    ~WidgetEditor();
//
//  protected slots:
//    void actionClickedSlot(bool newValue);
//
//  protected:
//    QWidget *widget;
//    QGridLayout *layout;
//};
//
//
//
//
//
///*! A QDockWidget which collects and displays all WidgetEditors being currently activated */
//class WidgetEditorCollector : public QDockWidget {
//  friend class WidgetEditor;
//  protected:
//    WidgetEditorCollector();
//    static WidgetEditorCollector *instance;
//    std::multimap<std::string, WidgetEditor*> handledEditors;
//    void updateLayout();
//    QVBoxLayout *layout;
//    QScrollArea *scrollArea;
//
//  public:
//    /* Get a singleton instance of WidgetEditorCollector */
//    static WidgetEditorCollector *getInstance();
//
//    /* Add a WidgetEditor */
//    void addEditor(WidgetEditor *editor);
//
//    /* Remove a WidgetEditor */
//    void removeEditor(WidgetEditor *editor);
//};





/*! A Editor of type double */
class FloatEditor : public Editor {
  Q_OBJECT

  public:
    /*! Constructor. */
    FloatEditor(PropertyDialog *parent_, const QIcon &icon, const std::string &name);

    /*! Set the valid range of the double value */
    void setRange(double min, double max) { spinBox->blockSignals(true); spinBox->setRange(min, max); spinBox->blockSignals(false); }

    /*! Set step size of the double value */
    void setStep(double step) { spinBox->blockSignals(true); spinBox->setSingleStep(step); spinBox->blockSignals(false); }

    /*! Set the special value at min */
    void setNaNText(const std::string &value) { spinBox->blockSignals(true); spinBox->setSpecialValueText(value.c_str()); spinBox->blockSignals(false); }

    /*! Set the suffix to display */
    void setSuffix(const QString &value) { spinBox->blockSignals(true); spinBox->setSuffix(value); spinBox->blockSignals(false); }

    /*! Set the conversion factor between the display value and the real value */
    void setFactor(const double value) { factor=value; }

    /*! OpenMBVCppInterface syncronization.
     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface */
    template<class OMBVClass>
    void setOpenMBVParameter(OMBVClass *ombv_, double (OMBVClass::*getter)(), void (OMBVClass::*setter)(OpenMBV::ScalarParameter));

  protected slots:
    void valueChangedSlot(double);

  protected:
    double factor;
    QDoubleSpinBox *spinBox;
    boost::function<double ()> ombvGetter;
    boost::function<void (double)> ombvSetter;
};





///*! A Editor of type double for a matrix or vector */
//class FloatMatrixEditor : public WidgetEditor {
//  Q_OBJECT
//
//  public:
//    /*! Constructor.
//     * Use 0 for rows/cols to defined a arbitary size in this dimension.*/
//    FloatMatrixEditor(QObject *parent_, const QIcon &icon, const std::string &name, unsigned int rows_, unsigned int cols_);
//
//    /*! OpenMBVCppInterface syncronization.
//     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface.
//     * Vector version. */
//    template<class OMBVClass>
//    void setOpenMBVParameter(OMBVClass *ombv_, std::vector<double> (OMBVClass::*getter)(),
//                                               void (OMBVClass::*setter)(const std::vector<double>&));
//
//    /*! OpenMBVCppInterface syncronization.
//     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface.
//     * Matrix version. */
//    template<class OMBVClass>
//    void setOpenMBVParameter(OMBVClass *ombv_, std::vector<std::vector<double> > (OMBVClass::*getter)(),
//                                               void (OMBVClass::*setter)(const std::vector<std::vector<double> >&));
//
//    /*! OpenMBVCppInterface syncronization.
//     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface.
//     * std::vector<OpenMBV::PolygonPoint*>* version. */
//    template<class OMBVClass>
//    void setOpenMBVParameter(OMBVClass *ombv_, std::vector<OpenMBV::PolygonPoint*>* (OMBVClass::*getter)(),
//                                               void (OMBVClass::*setter)(std::vector<OpenMBV::PolygonPoint*>*));
//
//  protected slots:
//    void addRowSlot(); // calls valueChanged (also see addRow)
//    void removeRowSlot(); // calls valueChanged
//    void addColumnSlot(); // calls valueChanged (also see addColumn)
//    void removeColumnSlot(); // calls valueChanged
//    void valueChangedSlot();
//
//  protected:
//    void addRow(); // does not call valueChanged (also see addRowSlot)
//    void addColumn(); // does not call valueChanged (also see addColumnSlot)
//    unsigned int rows, cols;
//    QTableWidget *table;
//    boost::function<std::vector<double> ()> ombvGetterVector;
//    boost::function<void (const std::vector<double>&)> ombvSetterVector;
//    boost::function<std::vector<std::vector<double> > ()> ombvGetterMatrix;
//    boost::function<void (const std::vector<std::vector<double> >&)> ombvSetterMatrix;
//    boost::function<std::vector<OpenMBV::PolygonPoint*>* ()> ombvGetterPolygonPoint;
//    boost::function<void (std::vector<OpenMBV::PolygonPoint*>*)> ombvSetterPolygonPoint;
//};
//
//
//
//
//
///*! A Editor of type int */
//class IntEditor : public WidgetEditor {
//  Q_OBJECT
//
//  public:
//    /*! Constructor. */
//    IntEditor(QObject *parent_, const QIcon &icon, const std::string &name);
//
//    /*! Set the valid range of the double value */
//    void setRange(int min, int max) { spinBox->blockSignals(true); spinBox->setRange(min, max); spinBox->blockSignals(false); }
//
//    /*! OpenMBVCppInterface syncronization.
//     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface */
//    template<class OMBVClass>
//    void setOpenMBVParameter(OMBVClass *ombv_, int (OMBVClass::*getter)(), void (OMBVClass::*setter)(int));
// 
//    /*! unsigned int version of setOpenMBVParameter */
//    template<class OMBVClass>
//    void setOpenMBVParameter(OMBVClass *ombv_, unsigned int (OMBVClass::*getter)(), void (OMBVClass::*setter)(unsigned int));
//
//  protected slots:
//    void valueChangedSlot(int);
//
//  protected:
//    QSpinBox *spinBox;
//    boost::function<int ()> ombvGetter;
//    boost::function<void (int)> ombvSetter;
//};
//
//
//
//
//
///*! A Editor of type string */
//class StringEditor : public WidgetEditor {
//  Q_OBJECT
//
//  public:
//    /*! Constructor. */
//    StringEditor(QObject *parent_, const QIcon &icon, const std::string &name);
//
//    /*! OpenMBVCppInterface syncronization.
//     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface */
//    template<class OMBVClass>
//    void setOpenMBVParameter(OMBVClass *ombv_, std::string (OMBVClass::*getter)(), void (OMBVClass::*setter)(std::string));
//
//  protected slots:
//    void valueChangedSlot();
//
//  protected:
//    QLineEdit *lineEdit;
//    boost::function<std::string ()> ombvGetter;
//    boost::function<void (std::string)> ombvSetter;
//};
//
//
//
//
//
///*! A Editor of type enum */
//class ComboBoxEditor : public WidgetEditor {
//  Q_OBJECT
//
//  public:
//    /*! Constructor. */
//    ComboBoxEditor(QObject *parent_, const QIcon &icon, const std::string &name,
//      const std::vector<boost::tuple<int, std::string, QIcon> > &list);
//
//    /*! OpenMBVCppInterface syncronization.
//     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface */
//    template<class OMBVClass, class OMBVEnum>
//    void setOpenMBVParameter(OMBVClass *ombv_, OMBVEnum (OMBVClass::*getter)(), void (OMBVClass::*setter)(OMBVEnum));
//
//  protected slots:
//    void valueChangedSlot(int);
//
//  protected:
//    QComboBox *comboBox;
//    boost::function<int ()> ombvGetter;
//    boost::function<void (int)> ombvSetter;
//};





/*! A Editor of type 3xdouble */
class Vec3fEditor : public Editor {
  Q_OBJECT

  public:
    /*! Constructor. */
    Vec3fEditor(PropertyDialog *parent_, const QIcon &icon, const std::string &name);

    /*! Set the valid range of the double values */
    void setRange(double min, double max) {
      for(int i=0; i<3; i++) {
        spinBox[i]->blockSignals(true);
        spinBox[i]->setRange(min, max);
        spinBox[i]->blockSignals(false);
      }
    }

    /*! Set step size of the double values */
    void setStep(double step) {
      for(int i=0; i<3; i++) {
        spinBox[i]->blockSignals(true);
        spinBox[i]->setSingleStep(step);
        spinBox[i]->blockSignals(false);
      }
    }

    /*! OpenMBVCppInterface syncronization.
     * Use getter and setter of ombv_ to sync this Editor with OpenMBVCppInterface */
    template<class OMBVClass>
    void setOpenMBVParameter(OMBVClass *ombv_, std::vector<double> (OMBVClass::*getter)(),
                                               void (OMBVClass::*setter)(double x, double y, double z));

  protected slots:
    void valueChangedSlot();

  protected:
    QDoubleSpinBox *spinBox[3];
    boost::function<std::vector<double> ()> ombvGetter;
    boost::function<void (double, double, double)> ombvSetter;
};





///*! A special Editor to edit a (mechanical) relative translation and rotation between two frames incling a interactive Dragger.
// * Note: This Editor does NOT delete and renew the Object at each value change. */
//class TransRotEditor : public WidgetEditor {
//  Q_OBJECT
//
//  public:
//    /*! Constructor.
//     * soTranslation_ and soRotation_ is syncronized with this Editor */
//    TransRotEditor(QObject *parent_, const QIcon &icon, const std::string &name, SoGroup *grp);
//
//    /*! Set step size of the translation values (rotation is 10deg) */
//    void setStep(double step) {
//      for(int i=0; i<3; i++) {
//        spinBox[i]->blockSignals(true);
//        spinBox[i]->setSingleStep(step);
//        spinBox[i]->blockSignals(false);
//      }
//    }
//
//    /*! OpenMBVCppInterface syncronization.
//     * Use *Getter and *Setter of ombv_ to sync this Editor with OpenMBVCppInterface */
//    template<class OMBVClass>
//    void setOpenMBVParameter(OMBVClass *ombv_, std::vector<double> (OMBVClass::*transGetter)(), 
//                                               void (OMBVClass::*transSetter)(double x, double y, double z),
//                                               std::vector<double> (OMBVClass::*rotGetter)(),
//                                               void (OMBVClass::*rotSetter)(double x, double y, double z));
//
//    /*! return a action to active the Dragger */
//    QAction *getDraggerAction() { return draggerAction; }
//
//  protected slots:
//    void valueChangedSlot();
//    void draggerSlot(bool newValue);
//
//  protected:
//    QDoubleSpinBox *spinBox[6];
//    SoTranslation *soTranslation;
//    SoRotation *soRotation;
//    boost::function<std::vector<double> ()> ombvTransGetter, ombvRotGetter;
//    boost::function<void (double, double, double)> ombvTransSetter, ombvRotSetter;
//    SoCenterballDragger *soDragger;
//    SoSwitch *soDraggerSwitch;
//    QAction *draggerAction;
//    static void draggerMoveCB(void *, SoDragger *dragger_);
//    static void draggerFinishedCB(void *, SoDragger *dragger_);
//};






/******************************************************************/





template<class OMBVClass>
void BoolEditor::setOpenMBVParameter(OMBVClass *ombv_, bool (OMBVClass::*getter)(), void (OMBVClass::*setter)(bool)) {
  ombvGetter=boost::bind(getter, ombv_);
  ombvSetter=boost::bind(setter, ombv_, _1);
  checkbox->blockSignals(true);
  checkbox->setCheckState(ombvGetter()?Qt::Checked:Qt::Unchecked);
  checkbox->blockSignals(false);
}





template<class OMBVClass>
void FloatEditor::setOpenMBVParameter(OMBVClass *ombv_, double (OMBVClass::*getter)(), void (OMBVClass::*setter)(OpenMBV::ScalarParameter)) {
  ombvGetter=boost::bind(getter, ombv_);
  ombvSetter=boost::bind(setter, ombv_, _1);
  spinBox->blockSignals(true);
  if(spinBox->specialValueText()=="" || !isnan(ombvGetter()))
    spinBox->setValue(ombvGetter()/factor);
  else
    spinBox->setValue(spinBox->minimum());
  spinBox->blockSignals(false);
}





//template<class OMBVClass>
//void FloatMatrixEditor::setOpenMBVParameter(OMBVClass *ombv_, std::vector<double> (OMBVClass::*getter)(),
//                                                              void (OMBVClass::*setter)(const std::vector<double>&)) {
//  // set functions
//  ombvGetterVector=boost::bind(getter, ombv_);
//  ombvSetterVector=boost::bind(setter, ombv_, _1);
//  // asserts
//  assert((cols==1 && (rows==0 || rows==ombvGetterVector().size())) ||
//         (rows==1 && (cols==0 || cols==ombvGetterVector().size())));
//  // create cells
//  if(cols==1) {
//    table->setColumnCount(1);
//    for(unsigned int r=0; r<ombvGetterVector().size(); r++)
//      addRow();
//  }
//  else { // rows==1
//    table->setRowCount(1);
//    for(unsigned int c=0; c<ombvGetterVector().size(); c++)
//      addColumn();
//  }
//  // set cell values
//  for(unsigned int i=0; i<ombvGetterVector().size(); i++) {
//    QDoubleSpinBox *spinBox;
//    if(cols==1)
//      spinBox=static_cast<QDoubleSpinBox*>(table->cellWidget(i, 0));
//    else
//      spinBox=static_cast<QDoubleSpinBox*>(table->cellWidget(0, i));
//    spinBox->blockSignals(true);
//    spinBox->setValue(ombvGetterVector()[i]);
//    spinBox->blockSignals(false);
//  }
//}
//
//template<class OMBVClass>
//void FloatMatrixEditor::setOpenMBVParameter(OMBVClass *ombv_, std::vector<std::vector<double> > (OMBVClass::*getter)(),
//                                                              void (OMBVClass::*setter)(const std::vector<std::vector<double> >&)) {
//  // set functions
//  ombvGetterMatrix=boost::bind(getter, ombv_);
//  ombvSetterMatrix=boost::bind(setter, ombv_, _1);
//  // asserts
//  assert(rows==0 || rows==ombvGetterMatrix().size());
//  assert(cols==0 || cols==ombvGetterMatrix()[0].size());
//  // create cells
//  table->setRowCount(ombvGetterMatrix().size());
//  for(unsigned int c=0; c<ombvGetterMatrix()[0].size(); c++)
//    addColumn();
//  // set cell values
//  for(unsigned int r=0; r<ombvGetterMatrix().size(); r++)
//    for(unsigned int c=0; c<ombvGetterMatrix()[r].size(); c++) {
//      QDoubleSpinBox *spinBox;
//      spinBox=static_cast<QDoubleSpinBox*>(table->cellWidget(r, c));
//      spinBox->blockSignals(true);
//      spinBox->setValue(ombvGetterMatrix()[r][c]);
//      spinBox->blockSignals(false);
//    }
//}
//
//
//template<class OMBVClass>
//void FloatMatrixEditor::setOpenMBVParameter(OMBVClass *ombv_, std::vector<OpenMBV::PolygonPoint*>* (OMBVClass::*getter)(),
//                                                              void (OMBVClass::*setter)(std::vector<OpenMBV::PolygonPoint*>*)) {
//  // set functions
//  ombvGetterPolygonPoint=boost::bind(getter, ombv_);
//  ombvSetterPolygonPoint=boost::bind(setter, ombv_, _1);
//  // asserts
//  assert(cols==3);
//  assert(rows==0 || rows==ombvGetterMatrix().size());
//  // create cells
//  table->setColumnCount(3);
//  for(unsigned int r=0; r<ombvGetterPolygonPoint()->size(); r++)
//    addRow();
//  // set cell values
//  for(unsigned int r=0; r<ombvGetterPolygonPoint()->size(); r++) {
//    QDoubleSpinBox *spinBox;
//
//    spinBox=static_cast<QDoubleSpinBox*>(table->cellWidget(r, 0));
//    spinBox->blockSignals(true);
//    spinBox->setValue((*ombvGetterPolygonPoint())[r]->getXComponent());
//    spinBox->blockSignals(false);
//
//    spinBox=static_cast<QDoubleSpinBox*>(table->cellWidget(r, 1));
//    spinBox->blockSignals(true);
//    spinBox->setValue((*ombvGetterPolygonPoint())[r]->getYComponent());
//    spinBox->blockSignals(false);
//
//    spinBox=static_cast<QDoubleSpinBox*>(table->cellWidget(r, 2));
//    spinBox->blockSignals(true);
//    spinBox->setValue((*ombvGetterPolygonPoint())[r]->getBorderValue());
//    spinBox->setSingleStep(1);
//    spinBox->setRange(0, 1);
//    spinBox->setDecimals(0);
//    spinBox->blockSignals(false);
//  }
//}
//
//
//
//
//template<class OMBVClass>
//void IntEditor::setOpenMBVParameter(OMBVClass *ombv_, int (OMBVClass::*getter)(), void (OMBVClass::*setter)(int)) {
//  ombvGetter=boost::bind(getter, ombv_);
//  ombvSetter=boost::bind(setter, ombv_, _1);
//  spinBox->blockSignals(true);
//  spinBox->setValue(ombvGetter());
//  spinBox->blockSignals(false);
//}
//
//template<class OMBVClass>
//void IntEditor::setOpenMBVParameter(OMBVClass *ombv_, unsigned int (OMBVClass::*getter)(), void (OMBVClass::*setter)(unsigned int)) {
//  setOpenMBVParameter(ombv_, reinterpret_cast<int (OMBVClass::*)()>(getter), reinterpret_cast<void (OMBVClass::*)(int)>(setter));
//}
//
//
//
//
//
//template<class OMBVClass>
//void StringEditor::setOpenMBVParameter(OMBVClass *ombv_, std::string (OMBVClass::*getter)(), void (OMBVClass::*setter)(std::string)) {
//  ombvGetter=boost::bind(getter, ombv_);
//  ombvSetter=boost::bind(setter, ombv_, _1);
//  lineEdit->blockSignals(true);
//  lineEdit->setText(ombvGetter().c_str());
//  lineEdit->blockSignals(false);
//}
//
//
//
//
//
//template<class OMBVClass, class OMBVEnum>
//void ComboBoxEditor::setOpenMBVParameter(OMBVClass *ombv_, OMBVEnum (OMBVClass::*getter)(), void (OMBVClass::*setter)(OMBVEnum)) {
//  ombvGetter=boost::bind(getter, ombv_);
//  ombvSetter=boost::bind(reinterpret_cast<void (OMBVClass::*)(int)>(setter), ombv_, _1); // for the setter we have to cast the first argument from OMBVEnum to int
//  comboBox->blockSignals(true);
//  comboBox->setCurrentIndex(comboBox->findData(ombvGetter()));
//  comboBox->blockSignals(false);
//}





template<class OMBVClass>
void Vec3fEditor::setOpenMBVParameter(OMBVClass *ombv_, std::vector<double> (OMBVClass::*getter)(), void (OMBVClass::*setter)(double x, double y, double z)) {
  ombvGetter=boost::bind(getter, ombv_);
  ombvSetter=boost::bind(setter, ombv_, _1, _2, _3);
  std::vector<double> vec=ombvGetter();
  for(int i=0; i<3; i++) {
    spinBox[i]->blockSignals(true);
    spinBox[i]->setValue(vec[i]);
    spinBox[i]->blockSignals(false);
  }
}





//template<class OMBVClass>
//void TransRotEditor::setOpenMBVParameter(OMBVClass *ombv_, std::vector<double> (OMBVClass::*transGetter)(),
//                                                           void (OMBVClass::*transSetter)(double x, double y, double z),
//                                                           std::vector<double> (OMBVClass::*rotGetter)(),
//                                                           void (OMBVClass::*rotSetter)(double x, double y, double z)) {
//  ombvTransGetter=boost::bind(transGetter, ombv_);
//  ombvTransSetter=boost::bind(transSetter, ombv_, _1, _2, _3);
//  ombvRotGetter=boost::bind(rotGetter, ombv_);
//  ombvRotSetter=boost::bind(rotSetter, ombv_, _1, _2, _3);
//  std::vector<double> trans=ombvTransGetter();
//  std::vector<double> rot=ombvRotGetter();
//  soTranslation->translation.setValue(trans[0], trans[1], trans[2]);
//  soRotation->rotation=Utils::cardan2Rotation(SbVec3f(rot[0],rot[1],rot[2])).invert();
//  for(int i=0; i<3; i++) {
//    spinBox[i  ]->setValue(trans[i]);
//    spinBox[i+3]->setValue(rot[i]*180/M_PI);
//  }
//}

#endif
