CC = g++.exe
INC = -I. -IC:\Users\ricar\Documents\SDL\2.26.3\SDL2-2.26.3\x86_64-w64-mingw32\include\SDL2 -IC:\Users\ricar\Documents\SDL\2.26.3\SDL2_image-2.6.3\x86_64-w64-mingw32\include\SDL2 -IC:\Users\ricar\Documents\SDL\2.26.3\SDL2_ttf-2.20.2\x86_64-w64-mingw32\include\SDL2
LIB = -LC:\Users\ricar\Documents\SDL\SDL2-devel-2.26.2-VC\SDL2-2.26.2\lib\x86 -LC:\Users\ricar\Documents\SDL\SDL2_image-2.6.2\lib\x86 -LC:\Users\ricar\Documents\SDL\SDL2_ttf-2.20.1\lib\x86
CFLAGS = -mwindows -g
DEPS = tinyfiledialogs.h
ODIR=$(shell cd)
LINKER_FLAGS= -lSDL2main -lSDL2 -lmingw32 -lSDL2_ttf -lSDL2_image -lcomdlg32 -lole32 -loleaut32

%.o: %.cpp $(DEPS)
	$(CC) -c *.cpp $(INC) $(LIB) $(CFLAGS) $(LINKER_FLAGS)
main: main.o $(DEPS)
	$(CC) -o main.exe *.o $(INC) $(LIB) $(CFLAGS) $(LINKER_FLAGS)
clean:
	cd $(ODIR)
	del *.o
	del *.exe