/*****************************************************
  Attributes.cpp

  Source file for Attributes Class

  CPSC8170 - Proj 1   GBG   9/2013
******************************************************/

#include "Attributes.h"

using namespace std;

//
// Constructor
//
Attributes::Attributes() {
    Vector ta(0.0, 0.0, 0.0);
    Vector tc(0.0, 0.0, 0.0, 1.0);

    V0 = ta;
    Velocity = ta;
    C0 = ta;
    Center = ta;
    Acceleration = ta;
    tempv = ta;
    tempc = ta;

    Color = tc;

    Mass = Coefff = Coeffr = EPS = 0.0;
}

void Attributes::InitAttr(Vector3d v0, Vector3d v, Vector3d c0, Vector3d c, Vector4d color, double m, double coefff, double coeffr, float eps) {
    V0 = v0;
    C0 = c0;
    Velocity = v;
    Center = c;
    Color = color;
    Mass = m;
    Coefff = coefff;
    Coeffr = coeffr;
    EPS = eps;
}

// setters
void Attributes::SetV0(Vector3d v) { V0 = v; }
void Attributes::SetVelocity(Vector3d v) { Velocity = v; }
void Attributes::SetC0(Vector3d c) { C0 = c; }
void Attributes::SetCenter(Vector3d c) { Center = c; }
void Attributes::SetAcceleration(Vector3d a) { Acceleration = a; }
void Attributes::SetColor(Vector4d c) { Color = c; }
void Attributes::SetMass(double m) { Mass = m; }
void Attributes::SetCoefff(double f) { Coefff = f; }
void Attributes::SetCoeffr(double r) { Coeffr = r; }
void Attributes::SetTempv(Vector3d v) { tempv = v; }
void Attributes::SetTempc(Vector3d c) { tempc = c; }

// getters
Vector3d Attributes::GetV0() { return V0; }
Vector3d Attributes::GetVelocity() { return Velocity; }
Vector3d Attributes::GetC0() { return C0; }
Vector3d Attributes::GetCenter() { return Center; }
Vector3d Attributes::GetAcceleration() { return Acceleration; }
double Attributes::GetMass() { return Mass; }
double Attributes::GetCoefff() { return Coefff; }
double Attributes::GetCoeffr() { return Coeffr; }
Vector3d Attributes::GetTempc() { return tempc; }
Vector3d Attributes::GetTempv() { return tempv; }
Vector4d Attributes::GetColor() { return Color; }

// calculations
void Attributes::CalcAccel(Vector3d g, Vector3d w, double v) {
    //Acceleration = Acceleration + Viscosity * (Wind - Velocity) / Mass;
    //acceleration = acceleration - Viscosity * Velocity / mass;
    if(w.x == 0 && w.y == 0 && w.z == 0)
        Acceleration = g - v * Velocity / Mass;
    else
        Acceleration = g + v * (w - Velocity) / Mass;
}

void Attributes::CalcTempCV(double ts) {
    //Velocity + timestep * Acceleration;
    //Center + timestep * Velocity;
    tempv = Velocity + ts * Acceleration;
    tempc = Center + ts * Acceleration;
}

void Attributes::CalcTempCV(double ts, double f) {
    //Velocity + f * timestep * Acceleration;
    //Center + f * timestep * Velocity;
    tempv = Velocity + f * ts * Acceleration;
    tempc = Center + f * ts * Velocity;
}

// Scale the velocity w/ coefficients of friction & restition - DO STORE IT.
// use for particles
void Attributes::ScaleVelocity(Vector3d pnormal) {
	Vector3d vn, vt;
	Vector3d unorm;

	unorm.set(pnormal.normalize());

	if (Velocity * unorm == 0) vn.set(0,0,0);
	else vn = (Velocity * unorm) * unorm;

	if (Velocity * unorm == 0) vt = Velocity;
	else vt = Velocity - (Velocity * unorm) * unorm;

	vn = -Coeffr * vn;
	vt = (1 - Coefff) * vt;

	Velocity = vn + vt;
}

// Adjust the acceleration, velocity, and position of the particle
// use for particles
void Attributes::AdjustAccVelPos(Vector3d pnormal, Vector3d pvertex, double t) {
	// reverse the direction of the vector i want to subtract...so...bVelocity - vNorm?
	// then set the new center so that...radius = t....& solve for bCenter?
	// bCenter	= vertex - t * (vN * bvel) / vn
	// then subtract from the acceleration...

	Vector3d vn, an;
	Vector3d intersect;
	float p;

	vn = (Velocity * pnormal) * pnormal;
	an = (Acceleration * pnormal) * pnormal;

	// find where we intersect and find  if we're on the plane...
	//once we have the point of intersection, decide where the ball is & adjust the intersection to account for the radius
	p = (Velocity.normalize() * pvertex) - (Velocity.normalize() * pnormal);
	if(p < 0) { // we're behind
		intersect = Center + Velocity * t ;
	} else {
		intersect = Center - Velocity * t ;
	}

    Velocity = Velocity - vn;
	Acceleration = Acceleration - an;
	Center = intersect;
}

// reflect velocity off of plane
void Attributes::Reflect(Vector3d pnormal, Vector3d pvertex) {
    Vector3d vn, vt;
    Vector3d smallr;
    smallr.set(.0001, .0001, .0001);
    double d;

    if (tempv * pnormal == 0) vn.set(0,0,0);
    else vn = (tempv * pnormal) * pnormal;

    if (Velocity * pnormal == 0) vt = Velocity;
    else vt = Velocity - (Velocity * pnormal) * pnormal;

    //Center = (fhit * Velocity) + smallr;  // need the center before we flip the velocity. - ((1 - Coefff) * vt)
    Velocity = tempv - ((1 + Coeffr) * (vn)) - ((1 - Coefff) * vt);

    d = (tempc - pvertex) * pnormal + .00001;
    Center = tempc - d * pnormal + smallr;
}

void Attributes::CalcPtAttract(Vector3d p0, Vector3d g) {
    Vector3d u, d;

    u = (Center - p0).normalize();
    d = (Center - p0).norm();

    Acceleration = Acceleration + ((- g * ( 1 / (d * d)) * u));
}

//////////// DEBUGGING ///////////////
void Attributes::PrintAttr() {
    cout << "Intial Velocity: "; V0.print();
    cout << endl << "Initial Center: "; C0.print();
    cout << endl << "Current Velocity: "; Velocity.print();
    cout << endl << "Current Center: "; Center.print();
    cout << endl << "Acceleration: "; Acceleration.print();
    cout << endl << "Color: "; Color.print();
    cout << endl << "Mass: " << Mass << endl;
    cout << "Coefff: " << Coefff << endl;
    cout << "Coeffr: " << Coeffr << endl;
}
