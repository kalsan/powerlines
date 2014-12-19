/**
   Copyright 2009-2014 Sandro Kalbermatter

    LICENSE:
    This program is licenced under GPL v3.0, see http://www.gnu.org/licenses/gpl-3.0
    It comes with absolutely no warranty.
**/

#ifndef POWERUP_H
#define POWERUP_H

#include "SDL/SDL.h"

class Powerup
{
public:
    Powerup();
    ~Powerup();

public: // TODO: This shall be private, but first need to implement some nice setters and getters
    // Note: Powerups automatically thake the color of their player.
    bool running; // I the powerup currently being drawn on screen (until it touches the wall)?
    bool available; // May the player use the powerup?
    SDL_Rect pos[2]; // Position of the 2 heads of the powerup (on the screen, rounded)
    double angle[2]; // Angles of the 2 heads
    double xDouble[2]; // real x-position of the 2 heads
    double yDouble[2]; // real y-position of the 2 heads
};

#endif // POWERUP_H
