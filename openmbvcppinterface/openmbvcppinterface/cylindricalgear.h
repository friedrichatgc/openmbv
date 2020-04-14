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

#ifndef _OPENMBV_CYLINDRICALGEAR_H_
#define _OPENMBV_CYLINDRICALGEAR_H_

#include <openmbvcppinterface/rigidbody.h>
#include <vector>

namespace OpenMBV {

  /** A cylindrical gear with an involute tooth profile */
  class CylindricalGear : public RigidBody {
    friend class ObjectFactory;
    protected:
      int N{15};
      double w{5e-2};
      double be{0};
      double m{16e-3};
      double al{0.349065850398866};
      double b{0};
      bool ext{true};
      double R{0};
      CylindricalGear() = default;
      ~CylindricalGear() override = default;
    public:
      /** Set the number of teeth */
      void setNumberOfTeeth(int N_) { N = N_; }

      int getNumberOfTeeth() { return N; }

      /** Set the width. */
      void setWidth(double w_) { w = w_; }
      
      double getWidth() { return w; }

      /** Set the helix angle. */
      void setHelixAngle(double be_) { be = be_; }

      double getHelixAngle() { return be; }

      /** Set the module. */
      void setModule(double m_) { m = m_; }
      
      double getModule() { return m; }

      /** Set the pressure angle. */
      void setPressureAngle(double al_) { al = al_; }
      
      double getPressureAngle() { return al; }

      /** Set the backlash. */
      void setBacklash(double b_) { b = b_; }

      double getBacklash() { return b; }

      /** Define the gearwheel to be external or internal toothed. */
      void setExternalToothed(bool ext_) { ext = ext_; }

      bool getExternalToothed() { return ext; }

      /** Set the outside radius. */
      void setOutsideRadius(double R_) { R = R_; }

      double getOutsideRadius() { return R; }

      /** Initializes the time invariant part of the object using a XML node */
      void initializeUsingXML(xercesc::DOMElement *element) override;

      xercesc::DOMElement *writeXMLFile(xercesc::DOMNode *parent) override;

  };

}

#endif /* _OPENMBV_CYLINDRICALGEAR_H_ */
