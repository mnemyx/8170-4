/*
 *  Struts.cpp
 *  GBG - Proj #4 - Springy Mesh
 *
 */

#include "Strut.h"

using namespace std;

Strut::Strut() {
    K = D = 0.0;
    L0 = 0.0;
    P0 = P1 = 0;
}

Strut::Strut(double k, double d, float l, int p0, int p1) {
    K = k;
    D = d;
    L0 = l;
    P0 = p0;
    P1 = p1;
}


void Strut::SetStrut(double k, double d, float l, int p0, int p1) {
    K = k;
    D = d;
    L0 = l;
    P0 = p0;
    P1 = p1;
}

void Strut::PrintStrut() {
    cout << "K: " << K << endl;
    cout << "D: " << D << endl;
    cout << "L0: " << L0 << endl;
    cout << "P0, P1: " << P0 << ", " << P1 << endl;
}
