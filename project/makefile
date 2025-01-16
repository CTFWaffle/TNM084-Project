# set this variable to the directory in which you saved the common files
commondir = ../common/

all : clean lab4-1

lab4-1 : lab4.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Mac/MicroGlut.m noise1234.c 
	gcc -Wall -o lab4-1 -I$(commondir) -I../common/Mac -I noise -DGL_GLEXT_PROTOTYPES lab4.cpp $(commondir)GL_utilities.c $(commondir)LoadTGA.c $(commondir)Mac/MicroGlut.m noise1234.c -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lSDL2_image  -framework OpenGL -framework Cocoa -lm -Wno-deprecated-declarations

clean :
	rm lab4-1
