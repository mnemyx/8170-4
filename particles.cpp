/********************************************************
  particles.cpp

  CPSC8170 - Proj 2   GBG   9/2013
*********************************************************/

#include "Pmanager.h"
#include "Pgenerator.h"
#include "Entity.h"
#include "time.h"

#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <fstream>

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

static double TimeStep;
static double DispTime;
static int TimerDelay;

static double Time = 0;
static int NTimeSteps = -1;

Pmanager Manager;
Pgenerator Generator1;
Pgenerator Generator2;

struct Env {
    Vector3d G;
    Vector3d Wind;
    double Viscosity;
 } env;

static int ProcessClick = 0;
static int Quadrant = -1;

/***********************  avoidance constants *********************/
double Ka = 1;
double Kv = .5;
double Kc = .75;

/************** DRAWING & SHADING FUNCTIONS ***********************/
//
// Get the shading setup for the objects
//
void GetShading(int hueIndx) {
  float ambient_color[4];
  float diffuse_color[4];
  float specular_color[4];
  int shininess;

    // set up material colors to current hue.
    for(int i = 0; i < 3; i++)
      ambient_color[i] = diffuse_color[i] = specular_color[i] = 0;

	for(int i = 0; i < 3; i++) {
		ambient_color[i] = AMBIENT_FRACTION * hues[hueIndx][i];
		diffuse_color[i] = DIFFUSE_FRACTION * hues[hueIndx][i];
		specular_color[i] = SPECULAR_FRACTION * hues[0][i];
		shininess = 0;
	}

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular_color);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

//
// Draw the moving objects
//
void DrawMovingObj() {
    GetShading(2);
    Manager.DrawSystem(NTimeSteps%2);
}

//
// Draw the non moving objects
//
void DrawNonMovingObj() {
    Generator1.DrawGenerator();
}

//
//  Draw the ball, its traces and the floor if needed
//
void DrawScene(int collision){

  int i,j;
  Model p;

  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);

  //DrawNonMovingObj();
  DrawMovingObj();

  glutSwapBuffers();
}

/********************* CALLED BY SIMULATE() ***********************/
Vector3d Displace(int a, int b) {
    Vector3d temp;

    temp.set(fmod((a + drand48()), (b - a)), fmod((a + drand48()), (b - a)), fmod((a + drand48()), (b - a)));

    return temp;
}

Vector3d Accelerate(State s, double  t, double m, int indx) {
    int nmaxp = s.GetSize();
    Vector3d acc;
    Vector3d xij, uij;
    Vector3d aaij, avij, acij, ai;
    Vector3d yforce, xforce, zforce;
    Vector3d ptu, ptacc, ptcenter;
    double ptd, ptg;
    int i;
    double Dij, dij, aa, av, ac, amax, ares;
    double r1 = 15.0;
    double r2 = 30.0;

    if (env.Wind.x == 0 && env.Wind.y == 0 && env.Wind.z == 0)
        acc = env.G - env.Viscosity * s[indx + nmaxp];
    else
        acc = env.G + env.Viscosity * (env.Wind - s[indx + nmaxp]);
    //cout << "before ";
    //acc.print();
    //cout << endl;

    if(indx != 0 ) {
        for (i = 0; i < nmaxp; i++) {
            if (i != indx) {

                //cout << "i: " << i << " indx: " << indx << endl;
                xij = s[i] - s[indx];
                //cout << "s[i]: " << s[i] << endl;
                //cout << "s[indx]: " << s[indx] << endl;
                //cout << "xij " << xij << endl;

                dij = xij.norm();
                uij = xij.normalize();

                if(dij < 40 || i == 0) {

                    //cout << " dij: " << dij << endl;

                    //if(i == 0) {
                        //Dij = 1;
                    //} else {
                        if(dij <= r1) Dij = 1.0;
                        else if (dij > r2) Dij = 0.0;
                        else Dij = 1.0 - (dij - r1) / (r2 - r1);
                    //}

                    //cout << Dij << endl;

                    if(i == 0) Kv = 0.0; else Kv = 1;
                    aaij = -Dij/m * Ka/dij * uij;
                    avij = Dij/m * Kv * (s[i + nmaxp] - s[indx + nmaxp]);
                    acij = Dij/m * Kc * dij * uij;

                    amax = 5.0;

                    aa = min(aaij.norm(), amax);
                    ares = amax - aa;

                    if(ares > 0) {
                        av = min(avij.norm(), ares);
                        ares = ares - av;

                        if(ares > 0) {
                            ac = min(acij.norm(), ares);
                        } else {
                            ac = 0.0;
                        }
                    } else {
                        av = 0.0;
                    }

                    ai = aa * uij + av * uij + ac * uij;

                    acc = acc + ai;
                }
            }
            //}
        }

        //acc.y = 0;

        if(ProcessClick) {
            //cout << "processing click" << endl;
            ProcessClick = false;

            double cpd;
            Vector3d cpu, cpacc, cpcen;
            double cpg = -10;

            switch(Quadrant) {
                case 0: cpcen.set(0,0,0); break;
                case 1: cpcen.set(40,40,40); break;
                case 2: cpcen.set(40,-40,40); break;
                case 3: cpcen.set(-40,-40,40); break;
                case 4: cpcen.set(-40,40,40); break;
                case 5: cpcen.set(40,40,-40); break;
                case 6: cpcen.set(40,-40,-40); break;
                case 7: cpcen.set(-40,-40,-40); break;
                case 8: cpcen.set(-40,40,-40); break;
                case 9: cpcen.set(Displace(-80, 81)); break;
                default: cpcen.set(Displace(-80, 81)); break;
            }

            cpd = (s[indx] - cpcen).norm();
            cpu = (s[indx] - cpcen).normalize();

            if (cpd < 20) {
                cpacc = - cpg * (1/(cpd*cpd)) * cpu;
                acc = acc + cpacc;
            }
        }

    } else {

        //point attractor at center
        ptcenter.set(0,0,0);
        ptd = (s[indx] - ptcenter).norm();
        ptu = (s[indx] - ptcenter).normalize();
        ptg = 5;


        ptacc = - ptg * ptu;
        acc = acc + ptacc;

    }


    if(-70 - s[indx].y > 0 ) {
        //yforce = 1.0/(-60 - s[indx].y) * (10/m) * Vector(0,1,0);
        //acc = acc + yforce;
        acc.y = 3;
    } else if (70 - s[indx].y < 0 ) {
        //yforce = 1.0/(-60 - s[indx].y) * (10/m) * Vector(0,-1,0);
        //acc = acc + yforce;
        acc.y = -3;
    }

    if(-70 - s[indx].x > 0 ) {
        //xforce = 1.0/(-60 - s[indx].x) * (10/m) * Vector(1,0,0);
        //acc = acc + xforce;
        //acc.x = acc.x + 1.0/(-60 - s[indx].x) * acc.x;
        //acc.x = 3;
    } else if (70 - s[indx].x < 0 ) {
        //xforce = 1.0/(-60 - s[indx].x) * (10/m) * Vector(-1,0,0);
        //acc = acc + xforce;
        //acc.x = acc.x - 1.0/(-60 - s[indx].x) * acc.x;
        acc.x = -3;
    }

    if(-70 - s[indx].z > 0 ) {
        //zforce = 1.0/(-60 - s[indx].z) * (10/m) * Vector(0,0,1);
        //acc = acc + zforce;
        //acc.z = acc.z + 1.0/(-60 - s[indx].z) * acc.z;
        acc.z = 3;
    } else if (70 - s[indx].x < 0 ) {
        //zforce = 1.0/(-60 - s[indx].z) * (10/m) * Vector(0,0,-1);
        //acc = acc + zforce;
        //acc.z = acc.z - 1.0/(-60 - s[indx].z) * acc.z;
        acc.z = -3;
    }


    //acc.print();
    //cout << " after"<< endl;

    return acc;
}

State F(State s, double m, double t) {
    int i;
    int nmaxp = s.GetSize();
    State x;

    x.SetSize(nmaxp);

    for (i = 0; i < nmaxp; i++) {
        //if(i == 0) { // our guiding boid
            //x[i] = s[nmaxp + i];

            // steering force = desired velocity - current velocity
            // what would the resulting velocity if the collision were to occur?
            // pure reflection of the current velocity could work.

            //  lissajous:
            //      ampA * sin(freqA * t + phasephi),
            //      ampB * sin(freqB * t + phasetri),
            //      ampC * sin(freqC * t + phasex)

            //x[i].x = x[i].x + (100 * (sin(1.5 * t + 30)));
            //x[i].y = x[i].y + (20 * (sin(.5 * t + 90)));
            //x[i].z = x[i].z + (100 * (sin(1 * t + 30)));

        //} else {
            x[i] = s[nmaxp + i]  + Displace(-5, 6);

        //}

        x[nmaxp + i] = (1 / m) * Accelerate(s, t, m, i);
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
    if(Manager.IsStopped()) {
        return;
    }

    //cout << "ADDING!! " << endl;
    // generate particles if we can
    if(Manager.HasFreeParticles()) {
    //cout << "Manager.FreePLeft(): " << Manager.FreePLeft() << endl;
        for(i = 0; i < Manager.GetMaxParticles(); i++) {
            Generator1.GenerateAttr(0);
            Generator2.GenerateAttr(0);
            if(i == 0)
            Manager.UseParticle(Vector(25, 5,0), Vector(2,0,0), Time, Vector(1,0,0,1), .0005, Generator1.GetCoefff(), Generator1.GetCoeffr(), false);
            else {
                if( i < (Manager.GetMaxParticles()/2) )
                Manager.UseParticle(Generator1.GenC0(), Vector(1,0,0), Time, Generator1.GenCol(), .0005, Generator1.GetCoefff(), Generator1.GetCoeffr(), false);
                else
                Manager.UseParticle(Generator2.GenC0(), Vector(1,0,0), Time, Generator1.GenCol(), .0005, Generator1.GetCoefff(), Generator1.GetCoeffr(), false);
            }
        }
    }

    //filebuf buf;
    //buf.open(("testlog"), ios::out);
    //streambuf* oldbuf = cout.rdbuf( &buf ) ;

    //Manager.S.PrintState();
    //cout << "Before & After " << endl;
    DrawScene(0);
    Manager.S = RK4(Manager.S, 1, Time, TimeStep);
   // Manager.S.PrintState();
    //cout.rdbuf(oldbuf);

    // advance the real timestep
    Time += TimeStep;
    NTimeSteps++;


    // set up time for next timestep if in continuous mode
    glutIdleFunc(NULL);
    if(Manager.IsStep())
        Manager.SetStopped(true);
    else{
        Manager.SetStopped(false);
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
void LoadParameters(char *filename){

    FILE *paramfile;

    double psize;

    if((paramfile = fopen(filename, "r")) == NULL){
        fprintf(stderr, "error opening parameter file %s\n", filename);
        exit(1);
    }

    ParamFilename = filename;

    if(fscanf(paramfile, "%lf %lf", &TimeStep, &psize)!= 2){
        fprintf(stderr, "error reading parameter file %s\n", filename);
        fclose(paramfile);
        exit(1);
    }

    Manager.SetMaxPart((int)psize, 1);

    Generator1.SetBaseAttr(4, 0.0, 0.0, 0.0, 0.0, Vector(0,0,0,1), 0.0, 0.0, 0.0, 0.0);
    Generator1.SetPlanePts(Vector(-80,-80,-10), Vector(-80,60,-10), Vector(80,80,-10), Vector(80,-80,-10));
    Generator1.SetModel();

    Generator2.SetBaseAttr(4, 0.0, 0.0, 0.0, 0.0, Vector(0,0,0,1), 0.0, 0.0, 0.0, 0.0);
    Generator2.SetPlanePts(Vector(80,80,-10), Vector(80,-60,-10), Vector(-80,-80,-10), Vector(-80,80,-10));
    Generator2.SetModel();

    env.G.set(0,0,0);
    env.Wind.set(0,0,0);
    env.Viscosity = 0;

    TimerDelay = int(0.5 * TimeStep * 1000);
}

//
// Routine to restart the ball at the top
//
void RestartSim(){

  LoadParameters(ParamFilename); // reload parameters in case changed

  glutIdleFunc(NULL);
  Time = 0;
  NTimeSteps = -1;
  DrawScene(0);
}


/************************* INITIALIZATIONS ****************************/
//
//  Initialize the Simulation
//
void InitSimulation(int argc, char* argv[]){

  if(argc != 2){
    fprintf(stderr, "usage: particles paramfile\n");
    exit(1);
  }

  LoadParameters(argv[1]);

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

  MenuAttached = false;

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
  const float light_position1[] = {-1, -3, -1, 1};
  const float light_position2[] = {1, 3, 1, 0};

  // clear the window to the background color
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);  // solid - clear depth buffer
  // establish shading model, flat or smooth
  glShadeModel(GL_SMOOTH);

  // light is positioned in camera space so it does not move with object
  glLoadIdentity();
  glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
  glLightfv(GL_LIGHT0, GL_AMBIENT, CRIMSON);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, CRIMSON);
  glLightfv(GL_LIGHT0, GL_SPECULAR, CRIMSON);

  glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
  glLightfv(GL_LIGHT1, GL_AMBIENT, VIOLET);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, VIOLET);
  glLightfv(GL_LIGHT1, GL_SPECULAR, VIOLET);

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

  DrawScene(0);

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
    //glOrtho(0, DRAWWIDTH, 0, DRAWHEIGHT, NEAR, FAR);
  else{
    // scale drawing coords so center of cube is same size as in ortho
    // if it is at its nominal location
    double scale = fabs((double)NEAR / (double)DEPTH);
    double xmax = scale * DRAWWIDTH / 2;
    double ymax = scale * DRAWHEIGHT / 2;
    glFrustum(-xmax, xmax, -ymax, ymax, NEAR, FAR);
    //glFrustum(0.0, xmax * 2, 0.0, xmax * 2, NEAR, FAR);
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
  if(!MenuAttached) {
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
}

//
//  Watch mouse button presses and handle them
//
void handleButton(int button, int state, int x, int y){

  if(MenuAttached) {

    if(button == GLUT_MIDDLE_BUTTON)
      MiddleButton = (state == GLUT_DOWN);

    if(button != GLUT_LEFT_BUTTON)
      return;

    AdjustMouse(x, y);	/* adjust mouse coords to current window size */

    if(state == GLUT_UP){
      if(Manager.IsStarted()){
        Manager.SetStarted(false);
        Manager.SetStopped(false);
        // need to re-initialize...should move to key press?
        DrawScene(0);
        glutIdleFunc(Simulate);
      }
      else if(Manager.IsStopped()){
        Manager.SetStopped(false);
        glutIdleFunc(Simulate);
      }
      else{
        Manager.SetStopped(true);
        glutIdleFunc(NULL);
      }
    }
  } else {

    if(state == GLUT_UP)
      Button = NONE;		// no button pressed
    else{
      MouseY = -y;			// invert y window coordinate to correspond with OpenGL
      MouseX = x;

      Button = button;		// store which button pressed

        // attempting interactive mode...
        /**
        GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
        GLdouble modelview[16];
            glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        GLdouble projection[16];
            glGetDoublev(GL_PROJECTION_MATRIX, projection);
        GLfloat winX, winY, winZ;

        AdjustMouse(x, y);
        winX = (float)x;
        winY = (float)y;
        glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

        GLdouble posX, posY, posZ;

        gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

        ProcessClick = true;
        ClickXYZ.set(posX/2,posY/2,posZ/2);

        cout << ClickXYZ << endl;
        **/

    }

  }
}

//
//  Menu callback
//
void HandleMenu(int index){

  switch(index){

  case MenuContinuous:
    if(Manager.IsStep()){
      Manager.SetStep(false);
      glutChangeToMenuEntry(index, "Step", index);
    }
    else{
      Manager.SetStep(true);
      glutChangeToMenuEntry(index, "Continuous", index);
    }
    break;

  case MenuQuit:
    exit(0);
  }
}

//
//  Set up pop-up menu on right mouse button
//
void MakeMenu(){
  int id = glutCreateMenu(HandleMenu);

  glutAddMenuEntry("Step", MenuContinuous);
  glutAddMenuEntry("Quit", MenuQuit);

  glutSetMenu(id);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
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

    case 'm':
    case 'M':
      MenuAttached = !MenuAttached;
      if(MenuAttached) { MakeMenu(); }
      else glutDetachMenu(GLUT_RIGHT_BUTTON);
      break;

    case '1':
      ProcessClick = true;
      Quadrant = 1;
      break;

    case '2':
      ProcessClick = true;
      Quadrant = 2;
      break;

    case '3':
      ProcessClick = true;
      Quadrant = 3;
      break;

    case '4':
      ProcessClick = true;
      Quadrant = 4;
      break;

    case '5':
      ProcessClick = true;
      Quadrant = 5;
      break;

    case '6':
      ProcessClick = true;
      Quadrant = 6;
      break;

    case '7':
      ProcessClick = true;
      Quadrant = 7;
      break;

    case '8':
      ProcessClick = true;
      Quadrant = 8;
      break;

    case '9':
      ProcessClick = true;
      Quadrant = 9;
      break;

    case '0':
      ProcessClick = true;
      Quadrant = 0;
      break;

    case 'r':
    case 'R':
        Manager.KillAll();
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
  glutCreateWindow("I think I like butterflies too much (aka flocking simulation)");

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
