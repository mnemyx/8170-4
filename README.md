PROJECT
==================================================================

	Gina Guerrero

	CpSc8170 - Fall 2013

	Project #4 - Springy Meshes (Butterfly Wings Simulation)

	C++/OpenGL/ImageMagick



DESCRIPTION
==================================================================

	Springy Meshes - Butterfly Wings Simulation

	Gauss/Matrix/Vector/Utility by Dr. House
	Model Class modified to create an isosphere & plane

	Color/Face/File/Group/ImageFile/Line/MakeSpace/Material
	MTLFile/OBJFile/Pixmap/PolySurf by Dr. House
	These were re-used from CpSc 405 - Spring 2011's Hwk #6 

	OBJ/MTL files generated in Maya

	Usage: particles [param_filename]


KEY COMMANDS
==================================================================
	m or M		IMPORTANT: transforming camera view to simulation control (default: camera)
	p or P		switches from ortho to perspective views (default: perspective)
	r or R      	resets the simulation, so if you change parameters - this will re-read them
	1 to 0		should trigger a dispersing force
				1 = +X, +Y, +Z quadrant
				2 = +X, -Y, +Z quadrant
				3 = -X, -Y, +Z quadrant
				4 = -X, +Y, +Z quadrant
				5 = +X, +Y, -Z quadrant
				6 = +X, -Y, -Z quadrant
				7 = -X, -Y, -Z quadrant
				8 = -X, +Y, -Z quadrant
				9 = random point...somewhere...over the rainbow
				0 = center of dispersal force is (0,0,0)
	q or ESC	quit


MOUSE COMMANDS
==================================================================
	SIMULATION:
	RMB		opens menu
	LMB 		triggers selected option

	CAMERA: 		  left button		 |	   middle button  	|		right button
	left drag	(-) rotation: model's y	 |  (+)-r: camera's y	|  (+) translation: camera's z
	right drag	(+) rotation: model's y	 |  (-)-r: camera's y	|  (-) translation: camera's z
	down drag	(+) rotation: model's x	 |  (+)-r: camera's x	|  (+) translation: camera's z
	up drag		(-) rotation: model's x	 |  (-)-r: camera's x	|  (-) translation: camera's z


MENU COMMANDS
==================================================================
	Continuous/Step		changes simulation between continuous or step
	Quit				quit


FILES
==================================================================
	particles.cpp (main program)
	State.cpp, State.h
	Struts.cpp, Struts.h
	Butterfly.cpp, Butterfly.h
	Model.cpp, Model.h
	Entity.cpp, Entity.h
	Attributes.cpp, Attributes.h
	Pgenerator.cpp, Pgenerator.h
	Pmanager.cpp, Pmanager.h,
	Particle.cpp, Particle.h


MISC FILES
==================================================================
	README.md
	Makefile
	MakeSpace.h
	Color.cpp, Color.h
	Face.cpp, Face.h
	File.cpp, File.h
	Group.cpp, Group.h
	ImageFile.cpp, ImageFile.h
	Line.cpp, Line.h
	Material.cpp, Material.h
	MTLFile.cpp, MTLFile.h
	OBJFile.cpp, OBJFile.h
	Pixmap.cpp, Pixmap.h
	PolySurf.cpp, PolySurf.h
	Matrix.cpp, Matrix.h
	Utility.cpp, Utility.h
	Vector.cpp, Vector.h
	gauss.cpp, gauss.h
	p


SAMPLE OUTPUT FILES
==================================================================
	screencap.png


TOTAL FILE COUNT
==================================================================
	????
