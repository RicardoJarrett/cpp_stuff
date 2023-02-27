CC = 
ODIR=
DELOP=
LINKER_FLAGS = -lSDL2main -lSDL2 -lSDL2_image
ifeq ($(OS),Windows_NT)
	CC += g++.exe
	LINKER_FLAGS += -lmingw32 -lcomdlg32 -lole32 -loleaut32
	ODIR+=$(shell cd)
	DELOP=$(shell del *.o; del *.exe)
else
   	UNAME_S := $(shell uname -s)
   	ifeq ($(UNAME_S),Linux)
		CC += g++
		ODIR+=$(shell pwd)
		DELOP=$(shell rm *.o; rm *.exe)
    	endif
endif

#Update include / lib folders for SDL
SDLMAIN_i = -IPathToSDL\include\SDL2
SDLMAIN_l = -LPathToSDL\lib\x86

SDLIMG_i = -IPathToSDLImage\include\SDL2
SDLIMG_l = -LPathToSDLImage\lib\x86

INC = -I. $(SDLMAIN_i) $(SDLIMG_i)
LIB = -L. $(SDLMAIN_l) $(SDLIMG_l)
CFLAGS = -g
DEPS = tinyfiledialogs.h

%.o: %.cpp $(DEPS)
	$(CC) -c *.cpp $(INC) $(LIB) $(CFLAGS) $(LINKER_FLAGS)

ifeq ($(OS),Windows_NT)
main: main.o
	$(CC) -o main.exe *.o $(INC) $(LIB) $(CFLAGS) $(LINKER_FLAGS)
endif
ifeq ($(UNAME_S),Linux)
main: main.o
	$(CC) -o main *.o $(INC) $(LIB) $(CFLAGS) $(LINKER_FLAGS)
endif
clean:
	cd $(ODIR)
	$(DELOP)
