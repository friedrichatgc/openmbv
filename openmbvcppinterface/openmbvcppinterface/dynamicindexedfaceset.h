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

#ifndef _OPENMBV_DYNAMICINDEXEDFACESET_H
#define _OPENMBV_DYNAMICINDEXEDFACESET_H

#include <openmbvcppinterface/dynamiccoloredbody.h>
#include <vector>
#include <hdf5serie/vectorserie.h>

namespace OpenMBV {

  /** A nurbs surface */
  class DynamicIndexedFaceSet : public DynamicColoredBody {
    friend class ObjectFactory;
    protected:
      std::vector<Index> indices;
      int numvp;
      H5::VectorSerie<double>* data;
      DynamicIndexedFaceSet();
      ~DynamicIndexedFaceSet() {}
      xercesc::DOMElement *writeXMLFile(xercesc::DOMNode *parent);
      /** Write H5 file for time-dependent data. */
      void createHDF5File();
      void openHDF5File();
    public:
      /** Get control points
       */
      int getNumberOfVertexPositions() const { return numvp; }
      const std::vector<Index>& getIndices() { return indices; }

      /** Set control points
       */
      void setNumberOfVertexPositions(int num) { numvp = num; }
      void setIndices(const std::vector<Index> &indices_) { indices = indices_; }

      /** Append a data vector to the h5 datsset */
      template<typename T>
      void append(const T& row) { 
        if(data==0) throw std::runtime_error("can not append data to an environment object"); 
        data->append(row);
      }

      int getRows() { return data?data->getRows():-1; }
      std::vector<double> getRow(int i) { return data?data->getRow(i):std::vector<double>(1+3*numvp); }

      /** Initializes the time invariant part of the object using a XML node */
      virtual void initializeUsingXML(xercesc::DOMElement *element);
  };

}

#endif
