/********************************************************
  particles.cpp

  CPSC8170 - Proj 4   GBG   10/2013
*********************************************************/

#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <fstream>

#include "time.h"
#include "OBJFile.h"
#include "PolySurf.h"
#include "State.h"
#include "Strut.h"
#include "Hinge.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;

#define WINDOW_WIDTH	800		/* window dimensions */
#define WINDOW_HEIGHT	600

#define MAXSTEPS	10000

#define MenuContinuous	1	// menu; switch from continues to step
#define MenuQuit	2		// quits program

#define NEAR		10		// distance of near clipping plane
#define FAR		1000		// distance of far clipping plane
#define DEPTH		-500	// initial z coord. of center of cube

#define ORTHO		0		// projection system codes
#define PERSPECTIVE	1
#define NONE		-1		// used to indicate no mouse button pressed

#define ROTFACTOR	0.2     // degrees rotation per pixel of mouse movement
#define XLATEFACTOR	0.5     // units of translation per pixel of mouse movement

#define DRAWWIDTH	200		// view volume sizes (note: width and
#define DRAWHEIGHT	150		//   height should be in same ratio as window)

#define AMBIENT_FRACTION 0.1	// lighting
#define DIFFUSE_FRACTION 0.2
#define SPECULAR_FRACTION 0.2


/******************* SHADING & COLORS *****************************/
const float BRIGHT_PALEBLUE[] = {0.5, 0.5, 1, 0.25};
const float CRIMSON[] = {.86, .078, .235, 0.25};
const float WHITE[] = {1, 1, 1, 1};
const float VIOLET[] = {.78, 0.08, 0.521, 1};
const float YELLOW[] = {1, 1, 0, 1};
const float BG[] = {.5,.5,.5};

float hues[][4] = { {1, 1, 1},    // white
		    {0.5, 0.5, 1},    // dim paleblue
		    {.86, .078, .235, 0},  // violet
		    {1, 1, 0},    // yellow
		  };


/*** Global variables updated and shared by callback routines ***/
// Viewing parameters
static int Projection;
// Camera position and orientation
static double Pan;
static double Tilt;
static double Approach;

// model orientation
static double ThetaX;
static double ThetaY;

// global variables to track mouse and shift key
static int MouseX;
static int MouseY;
static int Button = NONE;

static int MenuAttached;

static double WinWidth = WINDOW_WIDTH;
static double WinHeight = WINDOW_HEIGHT;
static int MiddleButton = false;

/********************** FOR THE ACTUAL SIMULATION ********************/
int FrameNumber = 0;

void TimerCallback(int value);

static char *ParamFilename = NULL;
static char *ObjFilename = NULL;

static double TimeStep;
static double DispTime;
static int TimerDelay;

static int Stopped = true;
static int Started = true;
static int Stepped = false;

static double Time = 0;
static int NTimeSteps = -1;

struct Env {
    Vector3d G;
    Vector3d Wind;
    double Viscosity;
    float Mass;
 } env;

static int Resting = false;
static int Wireframe = true;

static State B_State;
static Strut *B_Strut;
static Vector3d *Forces;
static Hinge *B_Hinge;

static double K;

/******************** OBJ/MTL RELATED VARIABLES *******************/
static OBJFile Objfile;
static PolySurf *Butterfly;	      // polygonal surface data structure
static Vector3d B_Centroid, B_Bboxmin, B_Bboxmax;
static Vector3d RightV[6], LeftV[6];
static int RightVIndx[6], LeftVIndx[6];
static int keyframe = 0;

/** Texture map to be used by program **/
static GLuint* TextureID;	    		// texture ID from OpenGL


/************** DRAWING & SHADING FUNCTIONS ***********************/
//
// Draw butterfly
//
void DrawModel() {
    int op = (Wireframe? GL_LINE_LOOP: GL_POLYGON);
	Vector3d tempVert, normal, x0, x1, x2;
	Vector2d textCoords;
	float ambient_color[4];
	float diffuse_color[4];
	float specular_color[4];
	int shininess;
	Material m;

	glEnable(GL_LIGHTING);
    glEnable(GL_SMOOTH);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

	// for each of the defined faces...
	for(int i = 0; i < Butterfly->getFaceCnt(); i++) {
		if(!Wireframe) {
			// do the material attached to the face...
			m = Butterfly->getMat(Butterfly->getFaces(i).getMatNdx());

			for(int k = 0; k < 4; k++) {
				ambient_color[k] = m.a[k];
				diffuse_color[k] = m.d[k];
				specular_color[k] = m.a[k];
			}

			shininess = m.n;

			switch(m.illum_model) {
				case 0:
					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_color);
					break;
				case 1:
					glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_color);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_color);
					break;
				case 2:
					glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_color);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_color);
					glMaterialfv(GL_FRONT, GL_SPECULAR, specular_color);
					glMaterialf(GL_FRONT, GL_SHININESS, shininess);
					break;
			}

			if(m.dmap || m.amap || m.smap){
				glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

				glBindTexture(GL_TEXTURE_2D, TextureID[Butterfly->getFaces(i).getMatNdx()]);	    // set the active texture
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
		}

		glBegin(op);
		if(!Wireframe) {
			x0 = B_State[Butterfly->getFaces(i).getVertNdx(0)];
            x1 = B_State[Butterfly->getFaces(i).getVertNdx(1)];
            x2 = B_State[Butterfly->getFaces(i).getVertNdx(2)];

            normal = (((x1-x0) % (x2-x0)).normalize());

			glNormal3f(normal.x, normal.y, normal.z);
		}

		// do the regular vertices for each face...
		for(int j = 0; j < Butterfly->getFaces(i).getFaceVertCnt(); j++) {

			if(Butterfly->getFaces(i).getUVNdx(j) != -1) {
				textCoords = Butterfly->getUV(Butterfly->getFaces(i).getUVNdx(j));
				glTexCoord2f(textCoords[0], textCoords[1]);
			}

			tempVert = B_State[Butterfly->getFaces(i).getVertNdx(j)];
			glVertex3f(tempVert.x, tempVert.y, tempVert.z);
		}
		glEnd();
	}
}

//
//  Draw the ball, its traces and the floor if needed
//
void DrawScene(){

  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  double scaleBy = (DRAWHEIGHT/2) / (B_Bboxmax.y - B_Bboxmin.y);

  glPushMatrix();
  glScalef(scaleBy, scaleBy, scaleBy);
  glTranslatef(-B_Centroid.x, -B_Centroid.y, -B_Centroid.z);
  DrawModel();
  glPopMatrix();

  glutSwapBuffers();
}

/********************* CALLED BY SIMULATE() ***********************/
Vector3d Displace(int a, int b) {
    Vector3d temp;

    temp.set(fmod((a + drand48()), (b - a)), fmod((a + drand48()), (b - a)), fmod((a + drand48()), (b - a)));

    return temp;
}

void HingeForces(State s, double t, double m) {        // this doesn't bode well for me...
    // remember: f0 + f1 + f2 + f3 = 0 AND t3 + t2 + t1 = 0
    int i, j;
    double l01, l3, l2, sina, cosa, theta;
    Vector3d x0, x1, x2, x3, h;
    Vector3d u03, u02;
    Vector3d nl, nr;
    Vector3d rl, rr;
    Vector3d f0, f1, f2, f3;
    Vector3d T;


    for(i = 0; i < Butterfly->getFMCnt(); i++) {
        x0 = s[B_Hinge[i].GetX0()];
        x1 = s[B_Hinge[i].GetX1()];
        x2 = s[B_Hinge[i].GetX2()];
        x3 = s[B_Hinge[i].GetX3()];

        // hinge:
        l01 = (x1 - x0).norm();
        h = (x1 - x0) / l01;

        u03 = (x3 - x0).normalize(); // / ((x3 - x0).norm());
        u02 = (x2 - x0).normalize(); // / ((x2 - x0).norm());

        nl = (h % u03).normalize(); // / (h % u03).norm();
        nr = (u02 % h).normalize(); // / (u02 % h).norm();

        rl = (x3 - x0) - ((x3 - x0) * h) * h;
        rr = (x2 - x0) - ((x2 - x0) * h) * h;

        sina = (nl % nr) * h;
        cosa = nl * nr;

        theta = atan2(sina, cosa);

        T = B_Hinge[i].GetK() * (theta - B_Hinge[i].GetA0()) * h;

        f3 = T / (rl.norm()) * nl;
        f2 = T / (rr.norm()) * nr;

        l3 = (x3 - x0) * h;
        l2 = (x2 - x0) * h;

        f1 = - (l2 * f2 + l3 * f3) / l01;
        f0 = - f1 - f2 - f3;

        Forces[B_Hinge[i].GetX0()] = Forces[B_Hinge[i].GetX0()] + f0;
        Forces[B_Hinge[i].GetX1()] = Forces[B_Hinge[i].GetX1()] + f1;
        Forces[B_Hinge[i].GetX2()] = Forces[B_Hinge[i].GetX2()] + f2;
        Forces[B_Hinge[i].GetX3()] = Forces[B_Hinge[i].GetX3()] + f3;

        //cout << "f0: " << f0 << endl;
        //cout << "f1: " << f1 << endl;
        //cout << "f2: " << f2 << endl;
        //cout << "f3: " << f3 << endl;

    }

}


void StrutForces(State s, double t, double m, int statesize) {            // needs state, strut and forces
    Vector3d xij, uij;
    float lij;
    int i, xi, xj;
    Vector3d tempf;

    // adding and calculating force - starting with spring and dampener
    for (i = 0; i < Butterfly->getEdgeCnt(); i++) {

            //if(B_Strut[i].IsStrut()) {
                xi = B_Strut[i].GetP1();
                xj = B_Strut[i].GetP0();

                xij = s[xj] - s[xi];
                lij = xij.norm();
                uij = xij / lij;
                //cout << endl;
                //cout << "i : " << i << endl;
                //cout << "xij: " << xij << "; lij: " << lij << "; B_Strut[i].GetL0(): " << B_Strut[i].GetL0()<< "; uij: " << uij << endl;

                tempf =  ((B_Strut[i].GetK() * (lij - B_Strut[i].GetL0())) * uij);

                //cout << "B_Strut->GetK(): " << B_Strut[i].GetK() << endl;
                //cout << "(lij - B_Strut->GetL0()) * uij: " << (lij - B_Strut[i].GetL0()) * uij << endl;
                //cout << "fs: " << tempf << endl;

                Forces[xi] = Forces[xi] + tempf;
                Forces[xj] = Forces[xj] - tempf;

                //cout << "before damping | [xi]: " <<  Forces[xi] << endl;
                //cout << "before damping | [xj]: " <<  Forces[xj] << endl;

                tempf =  ((B_Strut[i].GetD())  * ((s[xj + statesize] - s[xi + statesize]) * uij) * uij);

                //cout << "fd: " << tempf << endl;

                Forces[xi] = Forces[xi] + tempf;
                Forces[xj] = Forces[xj] - tempf;
            //}
            //cout << "after damping, Forces[xi]: " << Forces[xi] << endl;
            //cout << "after damping, Forces[xj]: " << Forces[xj] << endl;


    }
}

void CalcForces(State s, double  t, double m) {
    int i, j;
    int statesize = s.GetSize();
    Vector3d tempf;

    // intialize all forces to 0
    for(i = 0; i < statesize ; i++)
        Forces[i].set(0.0,0.0,0.0);

    StrutForces(s, t, m, statesize);
    HingeForces(s, t, m);

    //cout << "statesize: " << statesize << endl;
    for(i = 0; i < statesize ; i++) {
        //cout << "i - s[i + statesize]: " << i << " - " << s[i + statesize] << endl;
        if (env.Wind.x == 0 && env.Wind.y == 0 && env.Wind.z == 0)
            tempf = (env.G - env.Viscosity * s[i + statesize]) * m;
        else
            tempf = (env.G + env.Viscosity * (env.Wind - s[i + statesize])) * m;

        //cout << "tempf: " << tempf << endl;
        Forces[i] = Forces[i] + tempf;
    }

    //if(Forces[i].x < 0 || Forces[i].y  || Forces[i].z < 0) Forces[i].set(0,0,0);
}

State F(State s, double m, double t) {
    int i;
    int nmaxp = s.GetSize();
    State x;

    x.SetSize(nmaxp);

    CalcForces(s, t, m);

    for (i = 0; i < nmaxp; i++) {
        x[i] = s[nmaxp + i];
        x[nmaxp + i] = (1 / m) * Forces[i];
    }

    return x;
}

State RK4(State s, double m, double t, double ts) {
    State k1, k2, k3, k4;

    k1 = F(s, m, t) * ts;
    //cout << "k1" << endl;
    //k1.PrintState();
    k2 = F(s + (k1 * .5), m, t + ts * .5) * ts;
    //cout << "k2" << endl;
    //k2.PrintState();
    k3 = F(s + (k2 * .5), m, t + ts * .5) * ts;
    //cout << "k3" << endl;
    //k3.PrintState();
    k4 = F(s + k3, m, t + ts) * ts;
    //cout << "k4" << endl;
    //k4.PrintState();

    return (s + ((k1 + (k2*2) + (k3*2) + k4) * (.1666)));
}


/*********************** SIMULATE FUNCTION ************************/
///
//  Run a single time step in the simulation
//
void Simulate(){
    int i, j;

    // don't do anything if our simulation is stopped
    if(Stopped) {
        return;
    }

    //filebuf buf2;
    //buf2.open(("statelog"), ios::out);
    //streambuf* oldbuf2 = cout.rdbuf( &buf2 ) ;

    //cout << "Before & After " << endl;
    DrawScene();
    B_State = RK4(B_State, env.Mass, Time, TimeStep);

    if (keyframe == 0) {
        for(j = 0; j < 6; j++) {
            B_State[LeftVIndx[j]] = LeftV[j] + Vector3d(.2,.4,-1);
            B_State[RightVIndx[j]] = RightV[j] + Vector3d(-.2,.4,-1);
            keyframe = 1;
        }
    } else {
        for(j = 0; j < 6; j++) {
            B_State[LeftVIndx[j]] = LeftV[j] + Vector3d(.2,.4,.01);
            B_State[RightVIndx[j]] = RightV[j] + Vector3d(-.2,.4,.01);
            keyframe = 0;
        }
    }


    //B_State.PrintState();
    //cout.rdbuf(oldbuf2);


    // advance the real timestep
    Time += TimeStep;
    NTimeSteps++;


    // set up time for next timestep if in continuous mode
    glutIdleFunc(NULL);
    if(Stepped)
        Stopped = true;
    else{
        Stopped = false;
        glutTimerFunc(TimerDelay, TimerCallback, 0);
    }

}

//
//  Run a single time step in the simulation
//
void TimerCallback(int){
  Simulate();
}

/******************** LOAD PARAMETERS / RESET ****************************/
//
//  Load parameter file and reinitialize global parameters
//
void PopulateState(int vertcnt, Vector3d v0l, Vector3d v0r, Vector3d v0) {
    int i, j;
    B_State.SetSize(vertcnt);
    int alreadyAdded = false;

    for(i = 0; i < vertcnt; i++) {
        for(j = 0; j < 6; j++) {
            if (Butterfly->getVert(i) == LeftV[j]) {
                LeftVIndx[j] = i;
                B_State.AddState(i, Butterfly->getVert(i), v0l);
                alreadyAdded = true;
            }

            if (Butterfly->getVert(i) == RightV[j]) {
                RightVIndx[j] = i;
                B_State.AddState(i, Butterfly->getVert(i), v0r);
                alreadyAdded = true;
            }
        }

        if(!alreadyAdded) B_State.AddState(i, Butterfly->getVert(i), v0);

        alreadyAdded = false;
    }
}

void PopulateStrut(int edgecnt, int vertcnt, double kw, double dw, double kb, double db) {
    int i, j, tempegid;
    float l;
    Vector2d tempedge;
    char *tempgrpname;
    int tmpindx = edgecnt;

    B_Strut = new Strut[edgecnt];

    for(i = 0; i < edgecnt; i++) {
        tempedge = Butterfly->getEdge(i);
        tempegid =  Butterfly->getEdgeGrp(i);
        tempgrpname = Butterfly->getGroup(tempegid).getName();

        l = ((Butterfly->getVert(tempedge.y) - Butterfly->getVert(tempedge.x)).norm());

        if(strcmp(tempgrpname, "body") != 0) {
            //for(j = 0; j < 6; j++) {
                //if (tempedge.x == RightVIndx[j] || tempedge.x == LeftVIndx[j] || tempedge.y == RightVIndx[j] || tempedge.y == LeftVIndx[j])
                    //B_Strut[i].SetStrut(kw, dw, l, tempedge.x, tempedge.y, 1);
                //else
            B_Strut[i].SetStrut(kw, dw, l, tempedge.x, tempedge.y, 1);
            //}
        } else
            B_Strut[i].SetStrut(kb, db, l, tempedge.x, tempedge.y, 0);
    }
}

void PopulateHinge(int hingecnt, double ktheta) {
    int i, j;
    Vector3d x0, x1, x2, x3, h;
    Vector3d u03, u02;
    double l01, sina, cosa;
    Vector3d nl, nr;
    Vector3d rl, rr;

    B_Hinge = new Hinge[hingecnt];

    for(i = 0; i < hingecnt; i++) {
        B_Hinge[i].SetK(ktheta);
        B_Hinge[i].SetX0(Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x);
        B_Hinge[i].SetX1(Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y);

        x0 = Butterfly->getVert(Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x);
        x1 = Butterfly->getVert(Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y);

        //cout << "Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x" << Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x << endl;
        //cout << "Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y" << Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y << endl;

        for(j = 0; j < 3; j++) {
            if(Butterfly->getFaces(Butterfly->getFaceMatch(i).x).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x &&
                Butterfly->getFaces(Butterfly->getFaceMatch(i).x).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x &&
                Butterfly->getFaces(Butterfly->getFaceMatch(i).x).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y &&
                Butterfly->getFaces(Butterfly->getFaceMatch(i).x).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y)
                {
                    B_Hinge[i].SetX2(Butterfly->getFaces(Butterfly->getFaceMatch(i).x).getVertNdx(j));
                    x2 = Butterfly->getVert(Butterfly->getFaces(Butterfly->getFaceMatch(i).x).getVertNdx(j));
                }

             if(Butterfly->getFaces(Butterfly->getFaceMatch(i).y).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x &&
                Butterfly->getFaces(Butterfly->getFaceMatch(i).y).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).x &&
                Butterfly->getFaces(Butterfly->getFaceMatch(i).y).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y &&
                Butterfly->getFaces(Butterfly->getFaceMatch(i).y).getVertNdx(j) != Butterfly->getEdge(Butterfly->getFaceMatch(i).z).y)
                {
                    B_Hinge[i].SetX3(Butterfly->getFaces(Butterfly->getFaceMatch(i).y).getVertNdx(j));
                    x3 = Butterfly->getVert(Butterfly->getFaces(Butterfly->getFaceMatch(i).y).getVertNdx(j));
                }
        }

        //cout << x0 << endl << x1 << endl << x2 << endl << x3 << endl << endl;

        // hinge:
        l01 = (x1 - x0).norm();
        h = (x1 - x0) / l01;

        u03 = (x3 - x0).normalize(); //  / ((x3 - x0).norm() / 100 );
        u02 = (x2 - x0).normalize(); // / ((x2 - x0).norm() / 100 );

        nl = (h % u03).normalize(); // / (h % u03).norm();
        nr = (u02 % h).normalize(); // / (u02 % h).norm();

        rl = (x3 - x0) - ((x3 - x0) * h) * h;
        rr = (x2 - x0) - ((x2 - x0) * h) * h;

        sina = (nl % nr) * h;
        cosa = nl * nr;

        B_Hinge[i].SetA0(atan2(sina, cosa));
    }

}


void LoadParameters(char *filename, char *objfile){

    FILE *paramfile;
    double kw, dw, m, kb, db, ktheta;
    int i, vertcnt, edgecnt, hingecnt, suffix;
    Vector3d v0l, v0r, v0;

    if((paramfile = fopen(filename, "r")) == NULL){
        fprintf(stderr, "error opening parameter file %s\n", filename);
        exit(1);
    }

    // init the objfile
    suffix = strlen(objfile) - 4;

    if(strcmp(&(objfile[suffix]), ".obj") != 0) {
        fprintf(stderr, "obj file must end in .obj: %s\n", objfile);
        exit(1);
    }

    Objfile.setfilename(objfile);

    ParamFilename = filename;
    ObjFilename = objfile;

    // init the param file....
    if(fscanf(paramfile, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
              &TimeStep, &ktheta, &kw, &dw, &kb, &db, &m,
              &(v0r.x), &(v0r.y), &(v0r.z),
              &(v0l.x), &(v0l.y), &(v0l.z),
              &(v0.x), &(v0.y), &(v0.z),
              &(env.G.x), &(env.G.y), &(env.G.z),
              &(env.Wind.x), &(env.Wind.y), &(env.Wind.z), &(env.Viscosity),
              &(RightV[0].x), &(RightV[0].y), &(RightV[0].z),
              &(RightV[1].x), &(RightV[1].y), &(RightV[1].z),
              &(RightV[2].x), &(RightV[2].y), &(RightV[2].z),
              &(RightV[3].x), &(RightV[3].y), &(RightV[3].z),
              &(RightV[4].x), &(RightV[4].y), &(RightV[4].z),
              &(RightV[5].x), &(RightV[5].y), &(RightV[5].z),
              &(LeftV[0].x), &(LeftV[0].y), &(LeftV[0].z),
              &(LeftV[1].x), &(LeftV[1].y), &(LeftV[1].z),
              &(LeftV[2].x), &(LeftV[2].y), &(LeftV[2].z),
              &(LeftV[3].x), &(LeftV[3].y), &(LeftV[3].z),
              &(LeftV[4].x), &(LeftV[4].y), &(LeftV[4].z),
              &(LeftV[5].x), &(LeftV[5].y), &(LeftV[5].z))  != 59){
        fprintf(stderr, "error reading parameter file %s\n", filename);
        fclose(paramfile);
        exit(1);
    }

    // init the obj file
    int err = Objfile.read();
    Butterfly = Objfile.getscene();

    if(err || Butterfly == NULL){
        cerr << "OBJ file " << objfile << " has errors" << endl;
        exit(2);
    }

    B_Centroid = Butterfly->Centroid();
    B_Bboxmin = Butterfly->MinBBox();
    B_Bboxmax = Butterfly->MaxBBox();

    vertcnt = Butterfly->getVertCnt();
    edgecnt = Butterfly->getEdgeCnt();
    hingecnt = Butterfly->getFMCnt();

    // Set State from PolySurf - Butterfly
    // Need to figure out what to do with velocity...
    PopulateState(vertcnt, v0l, v0r, v0);
    PopulateStrut(edgecnt, vertcnt, kw, dw, kb, db);
    PopulateHinge(hingecnt, ktheta);

    // init forces...
    Forces = new Vector3d[vertcnt];
    //cout << TopRV << endl << TopLV << endl << "RV -> LV" << endl;
    // final initializations:
    env.Mass = (float) m / vertcnt;
    TimerDelay = int(0.5 * TimeStep * 1000);

    filebuf buf;
    buf.open(("test_butterfy_log"), ios::out);
    streambuf* oldbuf = cout.rdbuf( &buf ) ;
    cout << "Butterfly Data: " << *Butterfly << endl;
    cout << endl << "Special Vertices: " << endl;
    for(i = 0; i < 6; i++) { cout << i << " : V(" << RightVIndx[i] << "), " << RightV[i] << endl; }
    for(i = 0; i < 6; i++) { cout << i << " : V(" << LeftVIndx[i] << "), " << LeftV[i] << endl; }
    cout << endl << "Strut Data: " << endl;
    for(i = 0; i < edgecnt; i++) { cout << i << " : ";  B_Strut[i].PrintStrut(); }
    cout << endl << "Hinge Data: " << endl;
    for(i = 0; i < hingecnt; i++) { cout << i << " : ";  B_Hinge[i].PrintHinge(); }
    cout << endl << "Initialized State Vector: " << endl;
    B_State.PrintState();
    cout.rdbuf(oldbuf);
}

//
// Routine to restart simulation
//
void RestartSim(){

  LoadParameters(ParamFilename, ObjFilename); // reload parameters in case changed

  glutIdleFunc(NULL);
  Time = 0;
  NTimeSteps = -1;
  DrawScene();
}

/************************* INITIALIZATIONS ****************************/
//
//  Initialize the Simulation
//
void InitSimulation(int argc, char* argv[]){

  if(argc != 3){
    fprintf(stderr, "usage: particles paramfile butterfly.obj\n");
    exit(1);
  }

  LoadParameters(argv[1], argv[2]);

  Time = 0;
  NTimeSteps = -1;

  srand48(time(0));
}

//
// Initialize Camera
//
void InitCamera() {
  Projection = PERSPECTIVE;

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  Pan = 0;
  Tilt = 0;
  Approach = DEPTH;

  ThetaX = 0;
  ThetaY = 0;
}

/**************** ACTUAL (RE)DRAW FUNCTIONS ***********************/
//
//  On Redraw request, erase the window and redraw everything
//
void drawDisplay(){
  const float light_position1[] = {-1, 1, -1, 1};
  const float light_position2[] = {1, 1, 1, 1};

  // clear the window to the background color
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);  // solid - clear depth buffer
  // establish shading model, flat or smooth
  glShadeModel(GL_SMOOTH);

  // light is positioned in camera space so it does not move with object
  glLoadIdentity();
  glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
  glLightfv(GL_LIGHT0, GL_AMBIENT, WHITE);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, WHITE);
  glLightfv(GL_LIGHT0, GL_SPECULAR, WHITE);

  glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
  glLightfv(GL_LIGHT1, GL_AMBIENT, WHITE);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, WHITE);
  glLightfv(GL_LIGHT1, GL_SPECULAR, WHITE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

  // establish camera coordinates
  glRotatef(Tilt, 1, 0, 0);	    // tilt - rotate camera about x axis
  glRotatef(Pan, 0, 1, 0);	    // pan - rotate camera about y axis
  glTranslatef(0, 0, Approach);     // approach - translate camera along z axis

  // rotate the model
  glRotatef(ThetaY, 0, 1, 0);       // rotate model about x axis
  glRotatef(ThetaX, 1, 0, 0);       // rotate model about y axis

  DrawScene();

  glutSwapBuffers();
}

//
// Set up the projection matrix to be either orthographic or perspective
//
void updateProjection(){

  // initialize the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // determine the projection system and drawing coordinates
  if(Projection == ORTHO)
    glOrtho(-DRAWWIDTH/2, DRAWWIDTH/2, -DRAWHEIGHT/2, DRAWHEIGHT/2, NEAR, FAR);
  else{
    // scale drawing coords so center of cube is same size as in ortho
    // if it is at its nominal location
    double scale = fabs((double)NEAR / (double)DEPTH);
    double xmax = scale * DRAWWIDTH / 2;
    double ymax = scale * DRAWHEIGHT / 2;
    glFrustum(-xmax, xmax, -ymax, ymax, NEAR, FAR);
  }

  // restore modelview matrix as the one being updated
  glMatrixMode(GL_MODELVIEW);
}

/****************** MENU & CONTROL FUNCTIONS *********************/
//
//  On Reshape request, reshape viewing coordinates to keep the viewport set
//  to the original window proportions and to keep the window coordinates fixed
//
void doReshape(int w, int h){

  glViewport(0, 0, w, h);
  WinWidth = w;
  WinHeight = h;

  updateProjection();
}

//
//  Adjust mouse coordinates to match window coordinates
//
void AdjustMouse(int& x, int& y){

  /* reverse y, so zero at bottom, and max at top */
  y = int(WinHeight - y);

  /* rescale x, y to match current window size (may have been rescaled) */
  y = int(y * WINDOW_HEIGHT / WinHeight);
  x = int(x * WINDOW_WIDTH / WinWidth);
}

//
//  Watch mouse motion
//
void handleMotion(int x, int y){
    int delta;

    y = -y;
    int dy = y - MouseY;
    int dx = x - MouseX;

    switch(Button){
      case GLUT_LEFT_BUTTON:
        ThetaX -= ROTFACTOR * dy;
        ThetaY += ROTFACTOR * dx;
        glutPostRedisplay();
        break;
      case GLUT_MIDDLE_BUTTON:
        Pan -= ROTFACTOR * dx;
        Tilt += ROTFACTOR * dy;
        glutPostRedisplay();
        break;
      case GLUT_RIGHT_BUTTON:
        delta = (fabs(dx) > fabs(dy)? dx: dy);
        Approach += XLATEFACTOR * delta;
        glutPostRedisplay();
        break;
    }

    MouseX = x;
    MouseY = y;
}

//
//  Watch mouse button presses and handle them
//
void handleButton(int button, int state, int x, int y){
    if(state == GLUT_UP)
      Button = NONE;		// no button pressed
    else {
      MouseY = -y;			// invert y window coordinate to correspond with OpenGL
      MouseX = x;

      Button = button;		// store which button pressed
    }
}

//
// Keypress handling
//
void handleKey(unsigned char key, int x, int y){

  switch(key){
    case 'q':		// q - quit
    case 'Q':
    case 27:		// esc - quit
      exit(0);

    case 'p':			// P -- toggle between ortho and perspective
    case 'P':
      Projection = !Projection;
      updateProjection();
      glutPostRedisplay();
      break;

    case 's':
    case 'S':
      Stepped = !Stepped;
      if(Stepped) cout << "Stepped Mode";
      else cout << "Continous Mode";
      break;

    case 'd':
    case 'D':
        if(Started) {
            Started = false;
            Stopped = false;
            DrawScene();
            glutIdleFunc(Simulate);
        } else if(Stopped) {
            Stopped = false;
            glutIdleFunc(Simulate);
        } else {
            Stopped = true;
            glutIdleFunc(NULL);
        }
        break;

    case 'w':			// W -- toggle between wireframe and solid
    case 'W':
        Wireframe = !Wireframe;

        if(Wireframe == false) {
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
        }

        glutPostRedisplay();
        break;

    case 'r':
    case 'R':
        RestartSim();
      break;
    default:		// not a valid key -- just ignore it
      return;
  }

  glutPostRedisplay();
}


/********************* MAIN FUNCTION ***********************/
//
// Main program to set up display
//
int main(int argc, char* argv[]){

  InitSimulation(argc, argv);

  glutInit(&argc, argv);

  InitCamera();

  /* open window and establish coordinate system on it */
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  glutCreateWindow("Springy Meshes: Butterfly Wings Simulation");

  /* register display and mouse-button callback routines */
  glutReshapeFunc(doReshape);
  glutDisplayFunc(drawDisplay);
  glutMouseFunc(handleButton);
  glutMotionFunc(handleMotion);
  glutKeyboardFunc(handleKey);

  /* Set up to clear screen to black */
  glClearColor(BG[0], BG[1], BG[2], 0);

  glutMainLoop();
  return 0;
}
