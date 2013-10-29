/********************************************************
  Pgenerator.cpp

  Source File for the particle generator class

  Gina Guerrero - Fall 2013
********************************************************/

#include "Pgenerator.h"

using namespace std;

Pgenerator::Pgenerator() {
    srand48(time(0));
	gauss(1,1,time(0));

	Vector ta(0.0, 0.0, 0.0);
    Vector tc(0.0, 0.0, 0.0, 1.0);

	Type = POINT;
	Orientation = HORIZON;
	Center = ta;
	Velocity = ta;

	Radius = 0;
	P0 = ta;
	P1 = ta;
	P2 = ta;
	P3 = ta;
    internalcnt = 0;

	Mean = StdDev = BaseMass = MStdDev = CStdDev = BaseCoefff = BaseCoeffr = GeneratedMass = 0.0;

	BaseColor = tc;
	GeneratedV0 = ta;
	GeneratedC0 = ta;
	GeneratedColor = tc;

	PNum = 0;
}

void Pgenerator::SetBaseAttr(int type, double bs, double sd, double bm, double msd, Vector4d bc, double csd, double pnum, double coefff, double coeffr) {
	Type = type;
	Mean = bs;
	StdDev = sd;
	BaseMass = bm;
	MStdDev = msd;
	BaseColor = bc;
	CStdDev = csd;
	PNum = pnum;
	BaseCoefff = coefff;
	BaseCoeffr = coeffr;
}

void Pgenerator::SetCenterRadius(Vector3d center, double radius) {
	Center = center;
	Radius = radius;
}

void Pgenerator::SetPlanePts(Vector3d p0, Vector3d p1, Vector3d p2, Vector3d p3) {
    P0 = p0; P1 = p1; P2 = p2; P3 = p3;
}

void Pgenerator::SetVelocity(Vector3d v) {
    Velocity = v;
}

void Pgenerator::SetModel(int orientation) {
    Orientation = orientation;
	switch (Type) {
		case CIRCLE: Shape.BuildCircle(Radius, Orientation, Center.x, Center.y, Center.z); break;
		case SPHERE: Shape.BuildSphere(Radius, Center.x, Center.y, Center.z); break;
		case PLANE:  Shape.BuildPlane(P0, P1, P2, P3); break;
		default: break;
	}
}

void Pgenerator::SetBaseColor(Vector4d newbc) { BaseColor = newbc; }


void Pgenerator::GenerateAttr(int spdir) {
	double smallr = drand48() * drand48();
	Vector3d smallv;
	smallv.set(drand48() * drand48(), drand48() * drand48(), drand48() * drand48());

	// get random theta and phi
	double theta = drand48() * 360;
	double phi = drand48() * 360;
	Vector3d unit;

	// for triangles- need random index, need temp vectors, u & v
	int triIndx = (int) (drand48() * (Shape.GetNtriangles() - 1));
	Vector3d vertices, p0, p1, p2;
	double u, v;

	switch(Type) {
		case POINT:
			unit.set(sin(theta) * cos(phi), sin(theta) * cos(phi), cos(phi));

			GeneratedC0 = Center + smallr * unit;
			GeneratedV0 = gauss(Mean, StdDev, 0) * unit + smallv;

			break;
        case SPHERE:
            if(spdir) {
                if(internalcnt == 0) internalcnt = Shape.GetNtriangles() - 1;

                if(internalcnt > Shape.GetNtriangles() / 2) {
                    internalcnt--;
                } else {
                    internalcnt = Shape.GetNtriangles() - 1;
                }
            } else {
                if(internalcnt == 0)  internalcnt = Shape.GetNtriangles() / 2;

                if (internalcnt > 0) {
                    internalcnt--;
                } else {
                    internalcnt = Shape.GetNtriangles() / 2;
                }
            }

            vertices = Shape.GetTriangle(internalcnt);
			p2 = Shape.GetVertex(vertices.z);

            GeneratedC0 = p2;
			//GeneratedC0 = smallr * p1;
			GeneratedV0 = gauss(Mean, StdDev, 0) * Shape.GetNormal(internalcnt) + smallv;

			break;
		// most of my other stuff have triangles...
		default:
			vertices = Shape.GetTriangle(triIndx);
			p0 = Shape.GetVertex(vertices.x);
			p1 = Shape.GetVertex(vertices.y);
			p2 = Shape.GetVertex(vertices.z);

			u = drand48();
			v = drand48() * (1 - u);

			while(u + v >= 1) {
				u = drand48();
				v = drand48() * (1 - u);
			}

			// calculate new point by turning it back to cartesian coordinates
            //cout << "triIndx: " << triIndx;
			//cout << " Shape.GetNormal(triIndx): ";
			//( Shape.GetNormal(triIndx)).print();
			//cout << endl ;
			GeneratedC0 = p2 + u * (p0 - p2) + v * (p1 - p2) + (smallr * Shape.GetNormal(triIndx));
			//GeneratedC0 = smallr * p1;
			GeneratedV0 = gauss(Mean, StdDev, 0) * Shape.GetNormal(triIndx) + smallv;

			break;
	}

	// figure out a random mass and color
	GeneratedMass = gauss(BaseMass, MStdDev, 0);
	//GeneratedColor.set(1,1,1,1);
	GeneratedColor.set(gauss(BaseColor.x, CStdDev, 0), gauss(BaseColor.y, CStdDev, 0), gauss(BaseColor.z, CStdDev, 0), 1);

	//PrintGen();
}

Vector4d Pgenerator::GenerateColor(Vector4d c) {
    Vector4d newc;
    newc.set(gauss(c.x, CStdDev, 0), gauss(c.y, CStdDev, 0), gauss(c.z, CStdDev, 0), 1);
    return newc;
}

void Pgenerator::MoveGenerator(double ts) {
    Vector3d tempc;

    tempc = Center + ts * Velocity;

    if(tempc.x+Radius > 60)
        Velocity.set(Velocity.x * -1, Velocity.y, Velocity.z);
    if(tempc.x-Radius < -60)
        Velocity.set(Velocity.x * -1, Velocity.y, Velocity.z);
    if(tempc.y+Radius > 60)
        Velocity.set(Velocity.x, Velocity.y * -1, Velocity.z);
    if(tempc.y-Radius < -60)
        Velocity.set(Velocity.x, Velocity.y * -1, Velocity.z);
    if(tempc.z+Radius > 60)
        Velocity.set(Velocity.x, Velocity.y, Velocity.z * -1);
    if(tempc.z-Radius < -60)
        Velocity.set(Velocity.x, Velocity.y, Velocity.z * -1);

    Center = Center + ts * Velocity;

    switch(Type) {
        case CIRCLE: Shape.BuildCircle(Radius, Orientation, Center.x, Center.y, Center.z); break;
		case SPHERE: Shape.BuildSphere(Radius, Center.x, Center.y, Center.z); break;
		case PLANE:  Shape.BuildPlane(P0, P1, P2, P3); break;
		default: break;
    }

}

// generate random velocity, center, color, mass
Vector3d Pgenerator::GenV0() { return GeneratedV0; }
Vector3d Pgenerator::GenC0() { return GeneratedC0; }
Vector4d Pgenerator::GenCol() { return GeneratedColor; }
double Pgenerator::GenMass() { return GeneratedMass; }
int Pgenerator::GetPNum() { return PNum; }
double Pgenerator::GetCoefff() { return BaseCoefff; }
double Pgenerator::GetCoeffr() { return BaseCoeffr; }

// printing
void Pgenerator::PrintGen() {
    cout << "GENERATED VELOCITY: ";
    GeneratedV0.print();
    cout << endl << "GENERATED CENTER: ";
    GeneratedC0.print();
    cout << endl << "GENERATED COLOR: ";
    GeneratedColor.print();
    cout << endl << "GENERATED MASS: " << GeneratedMass << endl;
}

void Pgenerator::DrawGenerator() {
    Shape.Draw(1);
}
