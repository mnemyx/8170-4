#----------------------------------------------
#  Makefile for CpSc8170 - Proj4: Butterfly Springy Mesh
#----------------------------------------------
#
#  Gina Guerrero
#  August 28, 2013
#

CC      = g++
C	= cpp
H	= h
CFLAGS		= -g `Magick++-config --cppflags`
LFLAGS		= -g `Magick++-config --ldflags`

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL -lMagick++ -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS     = -lglut -lGL -lMagick++ -lm -L/usr/local/lib -lGLU
  endif
endif

HFILES 	= File.${H} OBJFile.${H} MTLFile.${H} ImageFile.${H} PolySurf.${H} Face.${H} Line.${H} Group.${H} Material.${H} Color.${H} Pixmap.${H} MakeSpace.${H} Matrix.${H} Vector.${H} Utility.${H} Model.${H} Attributes.${H} Entity.${H} gauss.${H} Particle.${H} Pmanager.${H} Pgenerator.${H} State.${H}
OFILES 	= File.o OBJFile.o MTLFile.o ImageFile.o PolySurf.o Face.o Line.o Group.o Material.o Pixmap.o Color.o Matrix.o Vector.o Utility.o Model.o  Attributes.o Entity.o gauss.o Particle.o Pmanager.o Pgenerator.o State.o
PROJECT = particles

${PROJECT}:	${PROJECT}.o $(OFILES)
	${CC} $(LFLAGS) -o ${PROJECT} ${PROJECT}.o $(OFILES) $(LDFLAGS)

${PROJECT}.o: ${PROJECT}.${C} $(HFILES)
	${CC} $(CFLAGS) -c ${PROJECT}.${C}

State.o: State.${C} State.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} State.${C}

Pgenerator.o: Pgenerator.${C} Pgenerator.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} Pgenerator.${C}

Pmanager.o: Pmanager.${C} Pmanager.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} Pmanager.${C}

Particle.o: Particle.${C} Particle.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} Particle.${C}

gauss.o: gauss.${C} gauss.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} gauss.${C}

Entity.o: Entity.${C} Entity.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} Entity.${C}

Attributes.o: Attributes.${C} Attributes.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} Attributes.${C}
	
File.o:  File.${C} File.${H}
	${CC} ${CFLAGS} -c File.${C}

OBJFile.o:  OBJFile.${C} File.${H} OBJFile.${H} MTLFile.${H} Vector.${H} Utility.${H}
	${CC} ${CFLAGS} -c OBJFile.${C}

MTLFile.o:  MTLFile.${C} File.${H} MTLFile.${H} ImageFile.${H} Color.${H} PolySurf.${H} Pixmap.${H}
	${CC} ${CFLAGS} -c MTLFile.${C}

ImageFile.o:  ImageFile.${C} File.${H} ImageFile.${H} Pixmap.${H} 
	${CC} ${CFLAGS} -c ImageFile.${C}

PolySurf.o:  PolySurf.${C} PolySurf.${H} Pixmap.${H} Vector.${H} Utility.${H}  MakeSpace.${H}
	${CC} ${CFLAGS} -c PolySurf.${C}

Face.o:  Face.${C} Face.${H} Color.${H} Pixmap.${H} Vector.${H} Utility.${H}
	${CC} ${CFLAGS} -c Face.${C}

Line.o:  Line.${C} Line.${H} Vector.${H} Utility.${H} MakeSpace.${H}
	${CC} ${CFLAGS} -c Line.${C}

Group.o:  Group.${C} Group.${H} MakeSpace.${H}
	${CC} ${CFLAGS} -c Group.${C}

Material.o:  Material.${C} Material.${H} Color.${H} Pixmap.${H} Vector.${H} Utility.${H}
	${CC} ${CFLAGS} -c Material.${C}

Pixmap.o:  Pixmap.${C} Pixmap.${H} 
	${CC} ${CFLAGS} -c Pixmap.${C}

Color.o:  Color.${C} Color.${H} 
	${CC} ${CFLAGS} -c Color.${C}
	
Model.o: Model.${C} Model.${H}
	${CC} ${CFLAGS} -c ${INCFLAGS} Model.${C}

Matrix.o: Matrix.${C} Matrix.${H} Vector.${H} Utility.${H}
	${CC} $(CFLAGS) -c Matrix.${C}

Vector.o: Vector.${C} Vector.${H} Utility.${H}
	${CC} $(CFLAGS) -c Vector.${C}

Utility.o: Utility.${C} Utility.${H}
	${CC} $(CFLAGS) -c Utility.${C}

MakeSpace.o:  MakeSpace.${C} MakeSpace.${H}
	${CC} ${CFLAGS} -c MakeSpace.${C}


debug:
	make 'DFLAGS = /usr/lib/debug/malloc.o'

clean:
	rm *.o *~ ${PROJECT}
