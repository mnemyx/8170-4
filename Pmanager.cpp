/********************************************************
  Pmanager.cpp

  Source File for the particle manager class

  Gina Guerrero - Fall 2013
********************************************************/

#include "Pmanager.h"

using namespace std;

///////////////////////////// PUBLIC FUNCTIONS /////////////////////////
Pmanager::Pmanager() {
    nused = 0;
	Started = true;
	Stopped = true;
	Step = false;
	Particles = NULL;
}

Pmanager::~Pmanager() {
    if (Particles != NULL) {
        delete[] Particles;
        Particles = NULL;
    }
}

Pmanager::Pmanager(const Pmanager& o) {
    Particles = new Particle;

    nused = o.nused;
    Stopped = o.Stopped;
    Started = o.Started;
    Step = o.Step;
    nmaxparticles = o.nmaxparticles;
    *Particles = *o.Particles;
    S = o.S;
}

Pmanager& Pmanager::operator=(const Pmanager& o) {
    Pmanager temp(o);

    swap(nused, temp.nused);
    swap(Stopped, temp.Stopped);
    swap(Started, temp.Started);
    swap(Step, temp.Step);
    swap(nmaxparticles, temp.nmaxparticles);
    swap(Particles, temp.Particles);
    swap(S, temp.S);

    return *this;
}

void Pmanager::SetMaxPart(int numofp, int bs) {
    nmaxparticles = numofp;
    Particles = new Particle[nmaxparticles];

    int i;
    for (i = 0; i < nmaxparticles; i++ ) {
        Particles[i].SetMaxHistory(bs);
    }

    S.SetSize(numofp);
}

void Pmanager::SetStopped(int type) { Stopped = type; }
void Pmanager::SetStarted(int type) { Started = type;}
void Pmanager::SetStep(int type) { Step = type; }

int Pmanager::IsStopped() { return Stopped; }
int Pmanager::IsStarted() { return Started; }
int Pmanager::IsStep() { return Step; }
int Pmanager::GetNused() { return nused; }
int Pmanager::GetMaxParticles() { return nmaxparticles; }

int Pmanager::HasFreeParticles() {
//cout << "nmaxparticles: "<< nmaxparticles << " --- "<< "nused: " << nused << endl;
    if(nused >= nmaxparticles) return false;
    else return true;
}

int Pmanager::FreePLeft() {
    return (nmaxparticles-nused);
}

void Pmanager::UseParticle(Vector3d c0, Vector3d v0, double ts, Vector4d color, double m, double coefff, double coeffr, int blend) {
//cout << "UseParticle() nused's before " << nused << endl;

	Particles[nused].SetInUse(true);
	Particles[nused].A.SetC0(c0);
	Particles[nused].A.SetCenter(c0);
	Particles[nused].A.SetV0(v0);
    Particles[nused].A.SetVelocity(v0);
    Particles[nused].SetBirth(ts);
	Particles[nused].A.SetColor(color);
	Particles[nused].A.SetMass(m);
	Particles[nused].A.SetCoefff(coefff);
	Particles[nused].A.SetCoeffr(coeffr);
	Particles[nused].SetBlend(blend);
    Particles[nused].AddHistory(c0);

	//Particles[nused].PrintInfo();
    S.AddState(nused, c0, v0);

	nused++;
	//cout << "UseParticle() nused's after " << nused << endl;
}

void Pmanager::EnableBlend(int bs) {
    int i;
    for(i=0; i<nused; i++) {
        Particles[i].SetBlend(bs);
    }
}

void Pmanager::FreeParticle(int indx) {
    if (indx < nused - 1) {
        //cout << "!! freeing indx: " << indx << " from nused: " << nused << endl;
        Particles[indx] = Particles[nused-1];
        //cout << "NHISTORY_NEW: " << Particles[indx].Getnhistory() << endl;
        Particles[nused-1].Reset();
        //cout << "NHISTORY_SHOULDBE0: " << Particles[nused-1].Getnhistory() << endl;
        nused--;

        S.MoveState(indx, nused);

    } else if (indx == nused - 1){
        //cout << "-- freeing indx: " << indx << " from nused: " << nused << endl;
        Particles[indx].Reset();
        //cout << "NHISTORY_SHOULDALSOBE0: " << Particles[indx].Getnhistory() << endl;
        nused--;

        S.RemoveState(indx);
    }
//cout << "new nused: " << nused << endl;
}

void Pmanager::KillAll() {
    int i;
        for (i = 0; i < nused; i++) {
            Particles[i].Reset();
            S.RemoveState(i);
        }
    nused = 0;
}

int Pmanager::KillParticles(double ts) {
    int i;
    int cnt = 0;
    Vector3d center;
    int onused = nused;

    for (i = 0; i < onused; i++) {
        if (Particles[i].IsInUse()) {
            center = Particles[i].A.GetCenter();

            if(Particles[i].GetAge(ts) > 10
                || center.x > 80 || center.x < -80
                || center.y > 80 || center.y < -80
                || center.z > 80 || center.z < -80)
            {
                FreeParticle(i);
                cnt++;
            }
        }
    }
    //cout << "cnt of those killed: "<< cnt << endl;
    return cnt;
}

void Pmanager::DrawSystem(int odd) {
	int i;
	int zdiff = 0;

	glEnable(GL_LIGHTING);
    glEnable(GL_SMOOTH);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    //glEnable(GL_POINT_SMOOTH);
    //glBegin(GL_POINTS);

    glShadeModel(GL_SMOOTH);

    Vector3d ux, uy, uz, vel, up;
    GLfloat r[16] = {0};
    up.set(0,1,0);

    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

	for ( i = 0; i < nused; i++ ) {
        //cout << "x: " << S[i].x << " y: " << S[i].y << " z: " <<  S[i].z << endl;
        //glBegin(GL_TRIANGLES)
        //if(i == 0) glColor4f(255,255,255,1);
        //else glColor4f(255,0,0,1);
        //glVertex3f(S[i].x, S[i].y, S[i].z);

        if(odd && (i%2)) zdiff = 3;
        else if (odd && !(i%2)) zdiff = 0;
        else -3;

        vel = S[i + nmaxparticles];

        ux = vel.normalize();
        uz = (vel % up).normalize();
        uy = uz % ux;

        r[0] = ux.x; r[1] = ux.y; r[2] = ux.z; r[3] = 0;
        r[4] = uy.x; r[5] = uy.y; r[6] = uy.z; r[7] = 0;
        r[8] = uz.x; r[9] = uz.y; r[10] = uz.z; r[11] = 0;
        r[12] = 0; r[13] = 0; r[14] = 0; r[15] = 1;

        glPushMatrix();
        glScalef(.4,.4,.4);
        glMultMatrixf(r);

            glBegin(GL_TRIANGLES);
                glVertex3f(S[i].x, S[i].y, S[i].z);
                glVertex3f(S[i].x+4, S[i].y+zdiff, S[i].z+5);
                glVertex3f(S[i].x+3, S[i].y+zdiff, S[i].z+1);

                glVertex3f(S[i].x, S[i].y, S[i].z);
                glVertex3f(S[i].x-4, S[i].y+zdiff, S[i].z+5);
                glVertex3f(S[i].x-3, S[i].y+zdiff, S[i].z+1);

                glVertex3f(S[i].x, S[i].y, S[i].z);
                glVertex3f(S[i].x+2, S[i].y+zdiff, S[i].z-8);
                glVertex3f(S[i].x+4, S[i].y+zdiff, S[i].z-1);

                glVertex3f(S[i].x, S[i].y, S[i].z);
                glVertex3f(S[i].x-2, S[i].y+zdiff, S[i].z-8);
                glVertex3f(S[i].x-4, S[i].y+zdiff, S[i].z-1);
            glEnd();

        glPopMatrix();
    }

    //glEnd();

    //glDisable(GL_BLEND);
	//glDisable(GL_SMOOTH);
	//glEnable(GL_LIGHTING);
}




