/**
   This class holds fields and methods that are used for manipulating the players.

   Copyright 2009-2015 Sandro Kalbermatter

    LICENSE:
    This program is licenced under GPL v3.0, see http://www.gnu.org/licenses/gpl-3.0
    It comes with absolutely no warranty.
**/

#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>
#include <SDL/SDL.h>
#include "settings.h"
#include "powerup.h"
#include "gamescreen.h"

class Player
{
    typedef struct Point{
        int x;
        int y;
    } Point;

public: // See .cpp file for comments about these functions
    Player(const int newHoleSize, const int newHoleDelay);
    ~Player();
    bool isEnabled();
    bool lives();
    int getLeftKeyCode();
    int getRightKeyCode();
    int getPts();
    SDL_Color getColor();
    void setColor(SDL_Color newColor);
    void setLeftKeyCode(int newKey);
    void setRightKeyCode(int newKey);
    void enable();
    void disable();
    void toggleEnabled();
    void resetPts();
    void initRound(int prgw, int prgh, Settings *settings);
    void drawBirth(SDL_Surface *sScreen, int slipLevel);
    void startPowerup();
    void react(Uint8* keystate, int keyPressDelay, double turnSpeed, int bombMaxSize, int bombMinSize, int prgw, int prgh, SDL_Surface *sScreen, Player *players[], GameScreen *gameScreen);
    void moveHead(const double stepSize);
    void drawHead(GameScreen *gameScreen, int slipLevel);
    void movePowerup(const double stepSize, int prgw, int prgh);
    void drawPowerup(GameScreen *gameScreen);
    bool collisionTestWithKill(GameScreen *gameScreen, int prgw, int prgh);
    void startBomb(int bombMaxSize, int bombMinSize, int prgw, int prgh, SDL_Surface *sScreen, Player *players[], GameScreen *gameScreen);
    bool receiveBomb(SDL_Rect rBomb, SDL_Surface* sBomb);
    void startTeleport(int prgw, int prgh, SDL_Surface *sScreen, GameScreen *gameScreen);
    void doHoles(int slipLevel, GameScreen *gameScreen);
    void givePoints(int amount);


private:
    // Private functions: see .cpp file for comments about these functions
    bool isLeftDown(Uint8 *keystate);
    bool isRightDown(Uint8 *keystate);

    bool enabled; ///< Does the player exist in the game?
    bool alive; ///< Is the player alive this round?
    int pts; ///< How many points did the player collect in this game?
    SDL_Color color; ///< Color of the player's line
    double xDouble,yDouble; ///< Current real position of the head of the line
    SDL_Rect pos; ///< Current position on screen of the head of the line (rounded to closest pixel)
    double angle; ///< What direction is the player's line currently facing?
    bool bombAvail; ///< Does the player have the right to use the bomb now?
    bool teleportAvail; ///< Does the player have the right to use teleport now?
    struct Powerup powerup; ///< Powerup of the player
    int leftKeyCode, rightKeyCode; ///< This holds the keymap
    int lPressed, rPressed; ///< This saves the timecode of the moment when the left / right button was pressed (or 0 if it's released right now)
    Point* holeBuffer; ///< Keeps track of points where a hole will be drawn.
    int holeSize; ///< Size (length) of the holes
    int holeDelay; ///< Distance to run before new hole is drawn.
    int stepsSinceLastHole; ///< Counter that tells us how far we have gone since the last hole was drawn. Resetted when we have reached holeSize + holeDelay.
};

#endif // PLAYER_H
