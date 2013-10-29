/***************************************************
  Entity.h

  Header file for Entity Class

  CPSC8170 - Proj 1   GBG   8/2013
****************************************************/

#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "Attributes.h"
#include "Model.h"


class Entity : public Model {		// entity is a model but has various variables to it...
  private:
      int Rest;
      int Stop;
      int Step;
      int Start;

      double Radius;

  public:
    Attributes A;

    // Constructor
    Entity();

	// Setters
	void SetRest(int type);
	void SetStop(int type);
	void SetStart(int type);
	void SetStep(int type);
	void SetRadius(double r);

	// Getters
	int IsRest();
	int IsStop();
	int IsStart();
	int IsStep();
	double GetRadius();

	// Functions
	// FOR SECOND PROJECT:
	int CheckCollision(Vector3d pcen, Vector3d pvel, Vector3d pnewcen);
};

#endif
