# Note: Use this if you want to compile manually

VERSION = 2-2
CFLAGS = -O3
LDFLAGS = `sdl-config --cflags --libs` -lSDL_image -lSDL_ttf 

powerlines: main.cpp  gamescreen.cpp gamescreen.h player.cpp player.h powerup.cpp powerup.h settings.cpp settings.h sanform/sanform_1-1.cpp sanform/sanform_1-1.h
	g++ $(CFLAGS) -std=gnu++11 -o build-Powerlines-Desktop-Debug/Powerlines main.cpp gamescreen.cpp player.cpp powerup.cpp settings.cpp sanform/sanform_1-1.cpp $(LDFLAGS)

run: powerlines
	cd build-Powerlines-Desktop-Debug && ./Powerlines
