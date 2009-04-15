#include "config.h"
#include "frustum.h"
#include <vector>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoIndexedTriangleStripSet.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoNormal.h>

using namespace std;

Frustum::Frustum(TiXmlElement *element, H5::Group *h5Parent) : RigidBody(element, h5Parent) {
  iconFile=":/frustum.svg";
  setIcon(0, QIcon(iconFile.c_str()));

  // read XML
  TiXmlElement *e=element->FirstChildElement(MBVISNS"baseRadius");
  double baseRadius=toVector(e->GetText())[0];
  e=e->NextSiblingElement();
  double topRadius=toVector(e->GetText())[0];
  e=e->NextSiblingElement();
  double height=toVector(e->GetText())[0];
  e=e->NextSiblingElement();
  double innerBaseRadius=toVector(e->GetText())[0];
  e=e->NextSiblingElement();
  double innerTopRadius=toVector(e->GetText())[0];

  const int N=25;

  // create so
  // coordinates
  SoCoordinate3 *coord=new SoCoordinate3;
  soSep->addChild(coord);
  for(int i=0; i<N; i++) {
    double phi=2*M_PI/N*i;
    coord->point.set1Value(i+0, baseRadius*cos(phi), baseRadius*sin(phi), 0);
    coord->point.set1Value(i+N, topRadius*cos(phi), topRadius*sin(phi), height);
    if(innerBaseRadius>0 || innerTopRadius>0) {
      coord->point.set1Value(i+2*N, innerBaseRadius*cos(phi), innerBaseRadius*sin(phi), 0);
      coord->point.set1Value(i+3*N, innerTopRadius*cos(phi), innerTopRadius*sin(phi), height);
    }
  }
  // normals
  SoNormal *normal=new SoNormal;
  soSep->addChild(normal);
  normal->vector.set1Value(0, 0, 0, -1);
  normal->vector.set1Value(1, 0, 0, 1);
  for(int i=0; i<N; i++) {
    double phi=2*M_PI/N*i;
    normal->vector.set1Value(i+2, cos(phi), sin(phi), (baseRadius-topRadius)/height);
    if(innerBaseRadius>0 || innerTopRadius>0)
      normal->vector.set1Value(i+2+N, -cos(phi), -sin(phi), -(innerBaseRadius-innerTopRadius)/height);
  }
  // faces (base/top)
  if(innerBaseRadius>0 || innerTopRadius>0) {
    int nr=-1;
    SoIndexedTriangleStripSet *baseFace=new SoIndexedTriangleStripSet;
    soSep->addChild(baseFace);
    SoIndexedTriangleStripSet *topFace=new SoIndexedTriangleStripSet;
    soSep->addChild(topFace);
    baseFace->coordIndex.set1Value(++nr, N-1);
    baseFace->normalIndex.set1Value(nr, 0);
    topFace->coordIndex.set1Value(nr, 2*N-1);
    topFace->normalIndex.set1Value(nr, 1);
    baseFace->coordIndex.set1Value(++nr, 3*N-1);
    baseFace->normalIndex.set1Value(nr, 0);
    topFace->coordIndex.set1Value(nr, 4*N-1);
    topFace->normalIndex.set1Value(nr, 1);
    for(int i=0; i<N; i++) {
      baseFace->coordIndex.set1Value(++nr, i);
      baseFace->normalIndex.set1Value(nr, 0);
      topFace->coordIndex.set1Value(nr, i+N);
      topFace->normalIndex.set1Value(nr, 1);
      baseFace->coordIndex.set1Value(++nr, i+2*N);
      baseFace->normalIndex.set1Value(nr, 0);
      topFace->coordIndex.set1Value(nr, i+3*N);
      topFace->normalIndex.set1Value(nr, 1);
    }
  }
  else {
    int nr=-1;
    SoIndexedFaceSet *baseFace=new SoIndexedFaceSet;
    soSep->addChild(baseFace);
    SoIndexedFaceSet *topFace=new SoIndexedFaceSet;
    soSep->addChild(topFace);
    for(int i=0; i<N; i++) {
      baseFace->coordIndex.set1Value(++nr, i);
      baseFace->normalIndex.set1Value(nr, 0);
      topFace->coordIndex.set1Value(nr, i+N);
      topFace->normalIndex.set1Value(nr, 1);
    }
  }
  // faces outer
  int nr=-1;
  SoIndexedTriangleStripSet *outerFace=new SoIndexedTriangleStripSet;
  soSep->addChild(outerFace);
  outerFace->coordIndex.set1Value(++nr, N-1);
  outerFace->normalIndex.set1Value(nr, N-1+2);
  outerFace->coordIndex.set1Value(++nr, 2*N-1);
  outerFace->normalIndex.set1Value(nr, N-1+2);
  for(int i=0; i<N; i++) {
    outerFace->coordIndex.set1Value(++nr, i);
    outerFace->normalIndex.set1Value(nr, i+2);
    outerFace->coordIndex.set1Value(++nr, i+N);
    outerFace->normalIndex.set1Value(nr, i+2);
  }
  // faces outer
  if(innerBaseRadius>0 || innerTopRadius>0) {
    int nr=-1;
    SoIndexedTriangleStripSet *innerFace=new SoIndexedTriangleStripSet;
    soSep->addChild(innerFace);
    innerFace->coordIndex.set1Value(++nr, 3*N-1);
    innerFace->normalIndex.set1Value(nr, 2*N-1+2);
    innerFace->coordIndex.set1Value(++nr, 4*N-1);
    innerFace->normalIndex.set1Value(nr, 2*N-1+2);
    for(int i=0; i<N; i++) {
      innerFace->coordIndex.set1Value(++nr, 2*N+i);
      innerFace->normalIndex.set1Value(nr, i+2+N);
      innerFace->coordIndex.set1Value(++nr, i+3*N);
      innerFace->normalIndex.set1Value(nr, i+2+N);
    }
  }
  
  // outline
  SoIndexedLineSet *outLine=new SoIndexedLineSet;
  nr=-1;
  outLine->coordIndex.set1Value(++nr, N-1);
  for(int i=0; i<N; i++)
    outLine->coordIndex.set1Value(++nr, i);
  outLine->coordIndex.set1Value(++nr, -1);
  outLine->coordIndex.set1Value(++nr, 2*N-1);
  for(int i=0; i<N; i++)
    outLine->coordIndex.set1Value(++nr, i+N);
  if(innerBaseRadius>0 || innerTopRadius>0) {
    outLine->coordIndex.set1Value(++nr, -1);
    outLine->coordIndex.set1Value(++nr, 3*N-1);
    for(int i=0; i<N; i++)
      outLine->coordIndex.set1Value(++nr, i+2*N);
    outLine->coordIndex.set1Value(++nr, -1);
    outLine->coordIndex.set1Value(++nr, 4*N-1);
    for(int i=0; i<N; i++)
      outLine->coordIndex.set1Value(++nr, i+3*N);
  }
  soSep->addChild(soOutLineSwitch);
  soOutLineSep->addChild(outLine);
}
