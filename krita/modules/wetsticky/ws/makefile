CFLAGS = -O3
#CFLAGS =  -g #-std
LINT_FLAGS = -bchxu

SOURCES = load_ppm.c canvas.c engine.c x_interface.c main.c 
OBJECTS = x_interface.o load_ppm.o engine.o canvas.o
OBJECTS2= ogl_interface.o load_ppm.o engine.o canvas.o

GFX_LIB=-L/usr/X11R6/lib \
	-lXaw -lXmu -lXt -lX11 -lm  -lppm -lpgm -lpbm

#PPM_LIBS = ppm/libppm.a  ppm/libpgm.a  ppm/libpbm.a

OGL_LIBS=-L/usr/X11R6/lib \
	 -laux -lGLU -lGL -lXext -lX11 -limp -lm -lppm -lpgm -lpbm

OGL_INCS=-I/usr2/share/src/OpenGL/libaux

CC=cc

wet+sticky:  constants.h canvas.h engine.h $(OBJECTS) main.c
	${CC} -o wet+sticky  $(CFLAGS) main.c $(OBJECTS) $(GFX_LIB)

wet+sticky2: constants.h canvas.h engine.h $(OBJECTS2) main.c
	${CC} -o wet+sticky2  $(CFLAGS) main.c $(OBJECTS2) $(OGL_LIBS) 

cmap: cmap.o
	${CC} -g -o cmap  cmap.o $(GFX_LIB)

anim:  constants.h canvas.h engine.h anim.c
	${CC} -o anim $(CFLAGS) anim.c $(GFX_LIB)

engine.o:  types.h constants.h canvas.h engine.h
	${CC} -c  $(CFLAGS)  engine3.c

canvas.o:  types.h constants.h canvas.h
	${CC} -c  $(CFLAGS)  canvas.c

x_interface.o: constants.h types.h canvas.h engine.h x_interface.c
	${CC} -c $(CFLAGS)  x_interface.c

ogl_interface.o:  constants.h types.h canvas.h engine.h ogl_interface.c
	${CC} -c $(CFLAGS) ${OGL_INCS} ogl_interface.c

load_ppm.o: constants.h types.h canvas.h engine.h load_ppm.c
	${CC} -c $(CFLAGS) load_ppm.c

cmap.o: cmap.c
	${CC} -c -g cmap.c

clean:
	/bin/rm -f core
	/bin/rm -f *.o

lint:	alint $(LINT_FLAGS) $(SOURCES)
