/***************************************************
  Attributes.h

  Header file for Attributes Class

  CPSC8170 - Proj 1   GBG   9/2013
****************************************************/

#ifndef _ATTR_H_
#define _ATTR_H_

#include "Vector.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

class Attributes{
  private:
    Vector3d V0;					// initial velocity
    Vector3d Velocity;				// (current) velocity
    Vector3d C0;					// initial position
    Vector3d Center;				// (current) position

    Vector3d Acceleration;			// acceleration...

    Vector3d tempv;                 // temporary velocity
    Vector3d tempc;                 // temporary center

    Vector4d Color;					// color
    double Mass;					// particle's mass

    double Coefff;					// coefficient of friction for particle
    double Coeffr;					// coefficient of restitution for particle

    float EPS;					    // the "fudge factor"

  public:
    // Constructor
    Attributes();

    // Setters
    void InitAttr(Vector3d v0, Vector3d v, Vector3d c0, Vector3d c, Vector4d color, double m, double coefff, double coeffr, float eps);
    void SetV0(Vector3d v);
    void SetVelocity(Vector3d v);
    void SetC0(Vector3d c);
    void SetCenter(Vector3d c);
    void SetAcceleration(Vector3d a);
    void SetCoefff(double f);
    void SetCoeffr(double r);
    void SetTempv(Vector3d v);
    void SetTempc(Vector3d c);
    void SetColor(Vector4d color);
    void SetMass(double m);


    // Getters
    Vector3d GetV0();
    Vector3d GetVelocity();
    Vector3d GetC0();
    Vector3d GetCenter();
    Vector3d GetAcceleration();
    Vector4d GetColor();
    double GetMass();
    double GetCoefff();
    double GetCoeffr();
    Vector3d GetTempc();
    Vector3d GetTempv();


    // Necessary calculations
    void CalcAccel(Vector3d g, Vector3d w, double v);       // calculate acceleration
    void CalcTempCV(double ts);                             // calculate temp velocity & center;
    void CalcTempCV(double ts, double f);                   // calculate temp velocity & center;
    void ScaleVelocity(Vector3d pnormal);                   // FROM FIRST PROJ: Scales velocity at collision
    void AdjustAccVelPos(Vector3d pnormal, Vector3d pvertex, double t);  // FROM FIRST PROJ: When putting sphere at rest. t is needed - distance from collision.
    void Reflect(Vector3d pnormal, Vector3d pvertex);          // FROM SECOND PROJECT: don't do expensive calculations! just reflect the particle.
    void CalcPtAttract(Vector3d p0, Vector3d g);            // FROM SECOND PROJECT: calculates attraction acceleration

    // debugging
    void PrintAttr();
};

#endif
