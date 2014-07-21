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

#ifndef _OPENMBV_OBJECT_H_
#define _OPENMBV_OBJECT_H_

#include <fmatvec/atom.h>
#include <string>
#include <sstream>
#include <openmbvcppinterface/objectfactory.h>
#include <hdf5serie/group.h>
#include <mbxmlutilshelper/dom.h>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <vector>

namespace XERCES_CPP_NAMESPACE {
  class DOMNode;
  class DOMElement;
}

namespace OpenMBV {

#ifndef SWIG // SWIG can not parse this (swig bug?). However it is not needed for the swig interface -> removed for swig
  const MBXMLUtils::NamespaceURI OPENMBV("http://openmbv.berlios.de/OpenMBV");
  const MBXMLUtils::NamespaceURI MBXMLUTILSPARAM("http://openmbv.berlios.de/MBXMLUtils/parameter");
#endif

  class Group;

  /** Abstract base class */
  class Object : public fmatvec::Atom {
    friend class Group;
    protected:
      std::string name;
      std::string enableStr, boundingBoxStr;
      std::string ID; // Note: the ID is metadata and stored as a processing instruction in XML
      bool selected; // Note: the selected flag is metadata and not stored in XML but used by OpenMBVGUI
      Group* parent;

      virtual void createHDF5File()=0;
      virtual void openHDF5File()=0;
      H5::GroupBase *hdf5Group;
      virtual void terminate()=0;

      /** Virtual destructor */
      virtual ~Object();
    public:
      /** Default constructor */
      Object();

      /** It must be possible to delete the top level Group: use this function for therefore.
       * If this object is was added into a parent object this object is first removed from this parent and then deleted. */
      virtual void destroy() const;

      /** Retrun the class name */
      virtual std::string getClassName()=0;

      /** Enable this object in the viewer if true (the default) */
      void setEnable(bool enable) { enableStr=(enable==true)?"true":"false"; }

      bool getEnable() { return enableStr=="true"?true:false; }

      /** Draw bounding box of this object in the viewer if true (the default) */
      void setBoundingBox(bool bbox) { boundingBoxStr=(bbox==true)?"true":"false"; }

      bool getBoundingBox() { return boundingBoxStr=="true"?true:false; }

      /** Set the name of this object */
      void setName(const std::string& name_) { name=name_; }

      std::string getName() { return name; }

      /** Returns the full name (path) of the object */
      virtual std::string getFullName(bool includingFileName=false, bool stopAtSeparateFile=false);

      /** Initializes the time invariant part of the object using a XML node */
      virtual void initializeUsingXML(xercesc::DOMElement *element);

      virtual xercesc::DOMElement *writeXMLFile(xercesc::DOMNode *parent);

      /** return the first Group in the tree which is an separateFile */
      Group* getSeparateGroup();

      /** return the top level Group */
      Group* getTopLevelGroup();

      Group* getParent() { return parent; }

      H5::GroupBase *getHDF5Group() { return hdf5Group; };

      /** get the ID sting of the Object (Note: the ID is metadata and stored as a processing instruction in XML) */
      std::string getID() const { return ID; }
      /** set the ID sting of the Object (Note: the ID is metadata and stored as a processing instruction in XML) */
      void setID(std::string ID_) { ID=ID_; }

      /** get the selected flag (Note: the selected flag is metadata and not stored in XML but used by OpenMBVGUI) */
      bool getSelected() const { return selected; }
      /** set the selected flag (Note: the selected flag is metadata and not stored in XML but used by OpenMBVGUI) */
      void setSelected(bool selected_) { selected=selected_; }

      // FROM NOW ONLY CONVENIENCE FUNCTIONS FOLLOW !!!
      static double getDouble(xercesc::DOMElement *e);
      static std::vector<double> getVec(xercesc::DOMElement *e, unsigned int rows=0);
      static std::vector<std::vector<double> > getMat(xercesc::DOMElement *e, unsigned int rows=0, unsigned int cols=0);

      static std::string numtostr(int i) { std::ostringstream oss; oss << i; return oss.str(); }
      static std::string numtostr(double d) { std::ostringstream oss; oss << d; return oss.str(); } 


      template<class T>
      static void addElementText(xercesc::DOMElement *parent, const MBXMLUtils::FQN &name, const T &value) {
        std::ostringstream oss;
        oss<<value;
        xercesc::DOMElement *ele = MBXMLUtils::D(parent->getOwnerDocument())->createElement(name);
        ele->insertBefore(parent->getOwnerDocument()->createTextNode(MBXMLUtils::X()%oss.str()), NULL);
        parent->insertBefore(ele, NULL);
      }
      static void addElementText(xercesc::DOMElement *parent, const MBXMLUtils::FQN &name, double value, double def);
      static void addElementText(xercesc::DOMElement *parent, const MBXMLUtils::FQN &name, const std::vector<double> &value);
      static void addElementText(xercesc::DOMElement *parent, const MBXMLUtils::FQN &name, const std::vector<std::vector<double> > &value);

      template <class T>
      static void addAttribute(xercesc::DOMNode *node, std::string name, T value) {
        xercesc::DOMElement *ele = dynamic_cast<xercesc::DOMElement*>(node);
        if(ele) {
          std::ostringstream oss;
          oss<<value;
          MBXMLUtils::E(ele)->setAttribute(name, oss.str());
        }
      }

      template <class T>
      static void addAttribute(xercesc::DOMNode *node, std::string name, T value, std::string def) {
        if(value!=def) addAttribute(node, name, value);
      }

    protected:
      static std::vector<double> toVector(std::string str);
      static std::vector<std::vector<double> > toMatrix(std::string str);
  };

}

#endif
