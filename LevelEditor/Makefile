CC = 
ODIR=
DELOP=
LINKER_FLAGS = -lSDL2main -lSDL2 -lmingw32 -lSDL2_ttf -lSDL2_image
ifeq ($(OS),Windows_NT)
	CC += g++.exe
	LINKER_FLAGS += -lcomdlg32 -lole32 -loleaut32
	ODIR+=$(shell cd)
	DELOP=$(shell del *.o; del *.exe)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		CC += g++.exe
        ODIR+=$(shell pwd)
		DELOP=$(shell rm *.o; rm *.exe)
    endif
endif

#Update include / lib folders for SDL
SDLMAIN_i = -IPathToSDL\include\SDL2
SDLMAIN_l = -LPathToSDL\lib\x86

SDLIMG_i = -IPathToSDLImage\include\SDL2
SDLIMG_l = -LPathToSDLImage\lib\x86

SDLTTF_i = -IPathToSDLTTF\include\SDL2
SDLTTF_l = -LPathToSDLTTF\lib\x86

INC = -I. $(SDLMAIN_i) $(SDLIMG_i) $(SDLTTF_i)
LIB = -L. $(SDLMAIN_l) $(SDLIMG_l) $(SDLTTF_l)
CFLAGS = -g
DEPS = tinyfiledialogs.h

%.o: %.cpp $(DEPS)
	$(CC) -c *.cpp $(INC) $(LIB) $(CFLAGS) $(LINKER_FLAGS)
main: main.o
	$(CC) -o main.exe *.o $(INC) $(LIB) $(CFLAGS) $(LINKER_FLAGS)
clean:
	cd $(ODIR)
	$(DELOP)