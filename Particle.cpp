/********************************************************
  Particle.cpp

  Source File for Particle Class

  Gina Guerrero - Fall 2013
********************************************************/

#include "Particle.h"

using namespace std;

//////////////////// PUBLIC FUNCTIONS /////////////////////
void Particle::Reset() {
	InUse = false;
	nhistory = 0;
}

Particle::Particle(){
    InUse = false;
	nhistory = 0;
	maxhistory = 0;

    history = NULL;
}

Particle::~Particle() {
    if (history != NULL) {
        delete[] history;
        history = NULL;
    }
}

Particle::Particle(const Particle& o) {
    history = new Vector3d;

    InUse = o.InUse;
    Birth = o.Birth;
    *history = *o.history;
    nhistory = o.nhistory;
    maxhistory = o.maxhistory;
    Blend = o.Blend;
    A = o.A;
}

Particle& Particle::operator=(const Particle& o) {
    Particle temp(o);

    swap(InUse, temp.InUse);
    swap(Birth, temp.Birth);
    swap(history, temp.history);
    swap(nhistory, temp.nhistory);
    swap(maxhistory, temp.maxhistory);
    swap(Blend, temp.Blend);
    swap(A, temp.A);

    return *this;
}


void Particle::SetMaxHistory(int bs) {
    maxhistory = bs;
    history = new Vector3d[maxhistory];
}


void Particle::Draw() {
    int i;
    glDisable(GL_LIGHTING);
    glEnable(GL_SMOOTH);
    glEnable(GL_BLEND);

    if(!Blend) {
    glEnable(GL_POINT_SMOOTH);
        glBegin(GL_POINTS);
            glColor4f(A.GetColor().x, A.GetColor().y, A.GetColor().z, 1);
            glVertex3f(A.GetCenter().x, A.GetCenter().y, A.GetCenter().z);
        glEnd();
    } else {
        if(nhistory <= 1) {
            glEnable(GL_POINT_SMOOTH);
                glBegin(GL_POINTS);
                glColor4f(A.GetColor().x, A.GetColor().y, A.GetColor().z, 0);
                glVertex3f(history[0].x, history[0].y, history[0].z);
        } else {
            glEnable(GL_LINE_SMOOTH);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_LINE_STRIP);
            glLineWidth(1);

            for (i = nhistory - 1; i >= 0; i--) {
                glColor4f(A.GetColor().x, A.GetColor().y, A.GetColor().z, (i/(nhistory-1)));
                glVertex3f(history[i].x, history[i].y, history[i].z);
            }
        }
        glEnd();
    }
	glDisable(GL_BLEND);
	glDisable(GL_SMOOTH);
	glEnable(GL_LIGHTING);
}

void Particle::AddHistory(Vector3d c) {
    int i;
    //cout << "nhistory before --- " << nhistory << endl;
	if(nhistory == maxhistory){
		for (i = 0; i < nhistory - 1; i++) {
            history[i] = history[i+1];
		}
		history[nhistory - 1] = c;
	} else {
        history[nhistory] = c;
        nhistory++;
        //cout << "nhistory after: "<< nhistory << endl;
	}
}


//////////// SETTERS //////////////
void Particle::SetBirth(double timestep) { Birth = timestep; }
void Particle::SetInUse(int type) { InUse = type; }
void Particle::SetBlend(int blend) { Blend = blend; }

//////////// GETTERS ///////////////
double Particle::GetBirth() { return Birth; }
double Particle::GetAge(double currentTimestep) { return currentTimestep - Birth; }
int Particle::IsInUse() { return InUse; }

int Particle::Getnhistory() { return nhistory; }


/////////// DEBUGGING ////////////////////
void Particle::PrintInfo() {
    cout << "In Use? " << InUse << endl;
    cout << "Birth Time: " << Birth << endl;
    cout << "maxhistory: " << maxhistory << endl;
    cout << "nhistory: " << nhistory << endl;

    int i;
    for (i=0; i<nhistory; i++) {
        cout << "nhistory #" << i << " vector: ";
        history[i].print();
        cout << endl;
    }

    cout << "Blend Mode? " << Blend << endl;

    A.PrintAttr();
}
