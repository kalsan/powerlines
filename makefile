# Note: Use this if you want to compile manually

VERSION = 2-2
CFLAGS = -O0
LDFLAGS = `sdl-config --cflags --libs` -lSDL_image -lSDL_ttf 

powerlines: main.cpp  sanform/sanform_1-1.cpp sanform/sanform_1-1.h
	g++ $(CFLAGS) -std=gnu++11 -o powerlines main.cpp sanform/sanform_1-1.cpp $(LDFLAGS)

run: powerlines
	./powerlines
