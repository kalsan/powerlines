/**
  Copyright 2009-2015 Sandro Kalbermatter

  LICENSE:
  This program is licenced under GPL v3.0, see http://www.gnu.org/licenses/gpl-3.0
  It comes with absolutely no warranty.
**/

#include "gamescreen.h"

/// Constructor / destructor -----------------------------------------------------

GameScreen::GameScreen(SDL_Surface* screen, int newPrgw, int newPrgh){
    prgw=newPrgw;
    prgh=newPrgh;
    sScreen=screen;
    evilPixels=new bool[prgw*prgh];
    clearAll();
}

GameScreen::~GameScreen(){
    delete evilPixels;
}

/// More advanced functions----------------------------------------------------------

void GameScreen::clearAll(){
    /// Clear evilPixels, but don't draw anything.

    for(int i=0;i<prgw*prgh;i++){
        evilPixels[i]=false;
    }
}

void GameScreen::fillRect(const int x, const int y, const int w, const int h, const Uint32 color){
    /// Draw a rectancle to the screen and also register it to evilPixels

    // Draw
    SDL_Rect location;
    location.x=x;
    location.y=y;
    location.w=w;
    location.h=h;
    SDL_FillRect(sScreen,&location,color);

    // Register to evilPixels
    for(int nw=x;nw<x+w;nw++){
        for(int nh=y;nh<y+h;nh++){
            evilPixels[nh*prgw+nw]=true;
        }
    }
}

void GameScreen::fillRect(const SDL_Rect inRect, const Uint32 color){
    /// Oveloaded function, this one deals with an SDL_Rect and a color in Uint32 format

    fillRect(inRect.x,inRect.y,inRect.w,inRect.h,color);
}

void GameScreen::fillRect(const int x, const int y, const int w, const int h, const SDL_Color color){
    /// Oveloaded function, this one deals with a color in SDL_Color format

    fillRect(x,y,w,h,SDL_MapRGB(sScreen->format,color.r,color.g,color.b));
}

void GameScreen::fillRect(const SDL_Rect inRect, const SDL_Color color){
    /// Oveloaded function, this one deals with an SDL_Rect and a color in SDL_Color format

    fillRect(inRect,SDL_MapRGB(sScreen->format,color.r,color.g,color.b));
}

void GameScreen::clearRect(const int x, const int y, const int w, const int h){
    /// Removes a rectangle from evilPixels and draws it on the screen using black color

    // Draw black
    SDL_Rect location;
    location.x=x;
    location.y=y;
    location.w=w;
    location.h=h;
    SDL_FillRect(sScreen,&location,SDL_MapRGB(sScreen->format,0,0,0));

    // Delete from evilPixels
    // Register to evilPixels
    for(int nw=x;nw<x+w;nw++){
        for(int nh=y;nh<y+h;nh++){
            evilPixels[nh*prgw+nw]=false;
        }
    }
}

void GameScreen::clearRect(const SDL_Rect inRect){
    /// Overloaded function, this one deals with an SDL_Rect

    clearRect(inRect.x,inRect.y,inRect.w,inRect.h);
}

void GameScreen::flip(){
    /// Write changes to screen

    SDL_Flip(sScreen);
}

bool GameScreen::hasCollision(const int x, const int y, const int w, const int h){
    /// Checks if anything in the specified rectangle collides with an evilPixel
    /// If so, returns true.

    for(int nw=x;nw<x+w;nw++){
        for(int nh=y;nh<y+h;nh++){
            if(evilPixels[nh*prgw+nw]){
                return true;
            }
        }
    }
    return false;
}

bool GameScreen::hasCollision(const SDL_Rect inRect){
    /// Overloaded function, this one deals with an SDL_Rect

    return hasCollision(inRect.x,inRect.y,inRect.w,inRect.h);
}

void GameScreen::debug_drawEvilPixels(){
    /// Will display black for harmless pixel and red for evilpixel. Use for debugging evilPixels.
    /// Usage: put a breakpoint right after call to this function, then look at the screen to see all evilPixels.

    SDL_FillRect(sScreen,NULL,SDL_MapRGB(sScreen->format,0,0,0));
    for(int i=0;i<prgh*prgw;i++){
        if(evilPixels[i]){
            SDL_Rect location;
            location.x=i%prgw;
            location.y=(int)floor(i/prgw);
            location.w=1;
            location.h=1;
            SDL_FillRect(sScreen,&location,SDL_MapRGB(sScreen->format,255,0,0));
        }
    }
    SDL_Flip(sScreen);
}
