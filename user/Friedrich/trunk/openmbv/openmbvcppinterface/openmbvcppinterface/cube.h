#ifndef _OPENMBV_CUBE_H_
#define _OPENMBV_CUBE_H_

#include <openmbvcppinterface/rigidbody.h>

namespace OpenMBV {

  /** A cube */
  class Cube : public RigidBody {
    protected:
      double length;
      void writeXMLFile(std::ofstream& xmlFile, const std::string& indent="");
    public:
      /** Default constructor */
      Cube();

      /** Set the length of the cube (x, y and z)*/
      void setLength(double length_) {
        length=length_;
      } 
  };

}

#endif
