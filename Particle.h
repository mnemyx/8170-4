/********************************************************
  Particle.h

  Header File for Particle Class

  Gina Guerrero - Fall 2013
********************************************************/


#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include "Attributes.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

class Particle {
	private:
		int InUse;						// particle off/on

		double Birth;					// store NTimeSteps of when the particle is "born" (for age)

		Vector3d *history;		        // particle's history of centers
		int nhistory;					// history indx
		int maxhistory;                 // max history

		int Blend;                      // draw in blend mode toggle

	public:
        Attributes A;                   // attributes class

		Particle();						// defaults...
		~Particle();
		Particle(const Particle& other);
		Particle& operator= (const Particle& other);

        void SetMaxHistory(int blendsize);

		void Reset();					// gets called by the constructor. kind of cleans up..
		void Draw();					// draws the particle
        void AddHistory(Vector3d c);	// adds history

		//////////// SETTERS //////////////
		void SetBirth(double timestep);
		void SetInUse(int type);
		void SetBlend(int blend);

		//////////// GETTERS ///////////////
        double GetBirth();
		double GetAge(double currentTimestep);
		int IsInUse();
		int Getnhistory();

		////////// DEBUGGING //////////////
		void PrintInfo();
};

#endif
