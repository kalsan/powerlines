/**
   This class allows to simultaneously draw to screen and register to evilPixels.
   It takes care of the management of any "dangerous" object on the screen during the game.

   Copyright 2009-2014 Sandro Kalbermatter

    LICENSE:
    This program is licenced under GPL v3.0, see http://www.gnu.org/licenses/gpl-3.0
    It comes with absolutely no warranty.
**/

#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include <math.h>
#include "SDL/SDL.h"

class GameScreen
{
public: // See .cpp file for comments about these functions.
    GameScreen(SDL_Surface *screen, int newPrgw, int newPrgh);
    ~GameScreen();
    void clearAll();
    void fillRect(const int x, const int y, const int w, const int h, const Uint32 color);
    void fillRect(const SDL_Rect inRect, const Uint32 color);
    void fillRect(const int x, const int y, const int w, const int h, const SDL_Color color);
    void fillRect(const SDL_Rect inRect, const SDL_Color color);
    void clearRect(const int x, const int y, const int w, const int h);
    void clearRect(const SDL_Rect inRect);
    void flip();
    bool hasCollision(const int x, const int y, const int w, const int h);
    bool hasCollision(const SDL_Rect inRect);

    void debug_drawEvilPixels();

private:
    int prgw; // Stores the width of the window
    int prgh; // Stores the height of the window
    SDL_Surface *sScreen; // Stores SDL_Surface to which we draw
    bool* evilPixels; // Datastructure used for collision detection. This has one entry for each pixel on the screen.
                      // If a player lands on a pixel where evilPixels is true, a collision is caused and the player dies.
                      // Order: height * prgw + width (left-to-right, top-to-bottom)
};

#endif // GAMESCREEN_H
