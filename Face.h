/*
 *  Face.h
 *  
 *
 *  Created by Donald House on 2/18/11.
 *  Copyright 2011 Clemson University. All rights reserved.
 *
 */

#ifndef __FACE__
#define __FACE__

#include "Vector.h"
#include "Group.h"
#include "Material.h"

struct Face{
  int nverts, maxverts;
  int (*verts)[3];
  Vector3d normal;
  int group;
  int material;

  Face(int g = -1, int m = -1);
  ~Face(){;}   // note, verts not destroyed
  
  void setGroup(int g);
  void setMaterial(int m);
  
  void addVert(int v, int n = -1, int u = -1);
  
  // added getters for face information
  int getFaceVertCnt() { return nverts; }
  int getVertNdx(int ndx) { return verts[ndx][0]; }
  int getNormNdx(int ndx) { return verts[ndx][1]; }
  int getUVNdx(int ndx) { return verts[ndx][2]; }
  int getMatNdx() { return material; }
  
  friend ostream& operator<< (ostream& os, const Face& f);
};

#endif
