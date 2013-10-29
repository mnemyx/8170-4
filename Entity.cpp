/***************************************************
  Entity.cpp

  Source file for Entity Class

  CPSC8170 - Proj 1   GBG   8/2013
****************************************************/

#include "Entity.h"


using namespace std;

//
// Constructor
//
Entity::Entity(){
	Start = true;
	Stop = true;
	Step = false;
	Rest = false;
}

//
// Setters
//
void Entity::SetRest(int rest) { Rest = rest; }
void Entity::SetStart(int start) { Start = start; }
void Entity::SetStop(int stop) { Stop = stop; }
void Entity::SetStep(int step) { Step = step; }
void Entity::SetRadius(double r) { Radius = r; }

//
// Getters
//
int Entity::IsRest() { return Rest; }
int Entity::IsStart() { return Start; }
int Entity::IsStop() { return Stop; }
int Entity::IsStep() { return Step; }
double Entity::GetRadius() { return Radius; }


//
// Functions
//
// fast triangle intersection test
// should be called by collidable objects
// uh...returns the index of the triangle that's closeset
// pass in as &fhit
int Entity::CheckCollision(Vector3d pcen, Vector3d pvel, Vector3d pnewcen) {
  int i, rtni;
  float f, fhit;
  double u, v, a;
  Vector3d p0, p1, p2;

  rtni = -1;
  fhit = 100;

  for (i = 0; i < ntriangles/2; i++) {
    p0.set(vertices[triangles[i][0]]);
    p1.set(vertices[triangles[i][1]]);
    p2.set(vertices[triangles[i][2]]);

    a = ((p2 - p1) % (p1 - p0)).norm();

    u = ((p2 - p1) % (pcen - p1)) * normals[i] / a;
    v = ((p0 - p2) % (pcen - p2)) * normals[i] / a;

    if(u >= 0.001 && v >= 0.001 && u + v <= 1 && u + v > -0.001) { // we hit...so now to calculate...
      f = ((pcen - p1) * normals[i] / ((pcen - pnewcen) * normals[i]));

      if(f >= 0 && f < 1 && f < fhit) {
        rtni = i;
      }
    }
  }

  return rtni;
}
