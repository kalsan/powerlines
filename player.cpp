/**
   Copyright 2009-2015 Sandro Kalbermatter

    LICENSE:
    This program is licenced under GPL v3.0, see http://www.gnu.org/licenses/gpl-3.0
    It comes with absolutely no warranty.
**/

#include "player.h"

/// Constructor / destructor -----------------------------------------------------

Player::Player(const int newHoleSize, const int newHoleDelay){
    holeSize=newHoleSize;
    holeBuffer=new Point[holeSize];
    holeDelay=newHoleDelay;
    enabled=false;
    stepsSinceLastHole=0;
}

Player::~Player(){
    delete holeBuffer;
}

/// Getters -----------------------------------------------------------------------

bool Player::isEnabled(){
    return enabled;
}

bool Player::lives(){
    return enabled && alive;
}

int Player::getLeftKeyCode(){
    return leftKeyCode;
}

int Player::getRightKeyCode(){
    return rightKeyCode;
}

int Player::getPts(){
    return pts;
}

SDL_Color Player::getColor(){
    return color;
}

/// Setters -----------------------------------------------------------------------

void Player::setColor(SDL_Color newColor){
    color=newColor;
}

void Player::setLeftKeyCode(int newKey){
    leftKeyCode=newKey;
}

void Player::setRightKeyCode(int newKey){
    rightKeyCode=newKey;
}

void Player::enable(){
    enabled=true;
}

void Player::disable(){
    enabled=false;
}

void Player::toggleEnabled(){
    enabled=!enabled;
}

void Player::resetPts(){
    pts=0;
}

/// More advanced functions----------------------------------------------------------

void Player::initRound(int prgw, int prgh, Settings* settings){
    /// This function initializes the player and makes it ready for the round.
    /// Must be called before beginning a round.

    // Ramdonly determine start position on screen
    SDL_Rect tempRect;
    int MAX=prgw-100,MIN=100;
    tempRect.x= (rand() % (MAX - MIN + 1)) + MIN;
    MAX=(prgh-100),MIN=100;
    tempRect.y= (rand() % (MAX - MIN + 1)) + MIN;
    pos=tempRect;
    xDouble=tempRect.x;
    yDouble=tempRect.y;

    // Random start angle
    angle=rand()%361;

    // Birth (or resurrection) of the player
    alive=true;

    // Init powerup
    powerup.running=false;
    if(settings->get("powerupsEnabled")){powerup.available=true;}else{powerup.available=false;}

    // Init bomb and teleport
    if(settings->get("bombsEnabled")){bombAvail=true;}else{bombAvail=false;}
    if(settings->get("teleportEnabled")){teleportAvail=true;}else{teleportAvail=false;}

    // Force keys off
    lPressed=0;
    rPressed=0;

    // Reset hole counter
    stepsSinceLastHole=0;
}

void Player::drawBirth(SDL_Surface* sScreen, int slipLevel){
    /// Draw the player's starting position and wait 2ms.
    /// Note: The starting position is NOT registred to evilPixels.

    if(enabled){
        // This is without evilPixels
        SDL_Rect location;
        location.x=pos.x-int(slipLevel/2);
        location.y=pos.y-int(slipLevel/2);
        location.w=slipLevel;
        location.h=slipLevel;
        SDL_FillRect(sScreen,&location,SDL_MapRGB(sScreen->format,color.r,color.g,color.b));
        SDL_Flip(sScreen);
        SDL_Delay(200);
    }
}

void Player::startPowerup(){
    /// Function for launching the powerup.
    /// Powerup starts at the player's current position and goes off to both sides.

    if(powerup.available){
        powerup.available=false;
        powerup.running=true;
        powerup.angle[0]=angle-90;
        powerup.angle[1]=angle+90;
        powerup.pos[0]=pos;
        powerup.pos[1]=pos;
        powerup.xDouble[0]=powerup.xDouble[1]=pos.x;
        powerup.yDouble[0]=powerup.yDouble[1]=pos.y;
    }
}

void Player::react(Uint8 *keystate, int keyPressDelay, double turnSpeed, int bombMaxSize, int bombMinSize, int prgw, int prgh, SDL_Surface* sScreen,
                   Player* players[10], GameScreen* gameScreen){
    /// Read the keymap and react to it.
    /// This is responsible for turning and launching powerup, bomb or teleport

    if(lives()){
        if(isLeftDown(keystate) && isRightDown(keystate) && lPressed && rPressed){
            if((abs(rPressed-lPressed)<=keyPressDelay) && powerup.available){
                startPowerup();
            }else if(lPressed-rPressed>keyPressDelay && bombAvail){
                startBomb(bombMaxSize,bombMinSize,prgw,prgh,sScreen,players,gameScreen);
            }else if(rPressed-lPressed>keyPressDelay && teleportAvail){
                startTeleport(prgw,prgh,sScreen,gameScreen);
            }
        }else if(isLeftDown(keystate)){
            angle-=turnSpeed; // Turn left
        }else if(isRightDown(keystate)){
            angle+=turnSpeed; // Turn right
        }
        if(!isLeftDown(keystate)){lPressed=0;} // When left button is released, reset its timecode
        if(!isRightDown(keystate)){rPressed=0;} // Same thing for right button
        if(!lPressed && isLeftDown(keystate)){lPressed=SDL_GetTicks();} // When left button is pressed, register its timecode
        if(!rPressed && isRightDown(keystate)){rPressed=SDL_GetTicks();} // Same thing for right button
    }
}

void Player::moveHead(const double stepSize){
    /// Determine the new position of the player's head
    /// Must be called at each step.

    if(lives()){
        // Find out new position
        xDouble+=(double)stepSize*cos(angle); // Move line head by one step
        yDouble+=(double)stepSize*sin(angle);
        pos.x=(int)round(xDouble); // Round screen position to closest pixel
        pos.y=(int)round(yDouble);
    }
}

void Player::drawHead(GameScreen *gameScreen, int slipLevel){
    /// Draw the player's head at its new position and register it to evilPixels
    /// This must be called after collision test.

    if(lives()){
        gameScreen->fillRect(pos.x-int(slipLevel/2), pos.y-int(slipLevel/2),slipLevel,slipLevel,color);
    }
}

void Player::movePowerup(const double stepSize, int prgw, int prgh){
    /// Determine the new position of the powerup's two heads.
    /// Must be called at each step.

    if(lives() && powerup.running){
        // Move first arm
        powerup.xDouble[0]+=3*(double)stepSize*cos(powerup.angle[0]);
        powerup.yDouble[0]+=3*(double)stepSize*sin(powerup.angle[0]);
        powerup.pos[0].x=(int)round(powerup.xDouble[0]);
        powerup.pos[0].y=(int)round(powerup.yDouble[0]);

        // Move second arm
        powerup.xDouble[1]+=3*(double)stepSize*cos(powerup.angle[1]);
        powerup.yDouble[1]+=3*(double)stepSize*sin(powerup.angle[1]);
        powerup.pos[1].x=(int)round(powerup.xDouble[1]);
        powerup.pos[1].y=(int)round(powerup.yDouble[1]);

        // Test if powerup collided with wall
        if((powerup.pos[0].x<10)||(powerup.pos[0].y<10)||(powerup.pos[0].x>prgw-10)||(powerup.pos[0].y>prgh-10)
                || (powerup.pos[1].x<10)||(powerup.pos[1].y<10)||(powerup.pos[1].x>prgw-10)||(powerup.pos[1].y>prgh-10))
        {
            powerup.running=false;
            return;
        }
    }
}

void Player::drawPowerup(GameScreen *gameScreen){
    /// Draw the powerup's two heads at their new positions and register it to evilPixels
    /// This must be called after collision test.

    if(lives() && powerup.running){
        // Draw both arms
        gameScreen->fillRect(powerup.pos[0].x-2, powerup.pos[0].y-1,3,2,color);
        gameScreen->fillRect(powerup.pos[1].x-2, powerup.pos[1].y-1,2,3,color);
    }
}

bool Player::collisionTestWithKill(GameScreen* gameScreen, int prgw, int prgh){
    /// Returns true iff player dies right now
    /// This must be called after move functions and before draw functions.

    if(enabled && alive){
        // Check if the player has touched the screen
        if((pos.x<10)||(pos.y<10)||(pos.x>prgw-10)||(pos.y>prgh-10)){
            // The player is outside the borders! Did he get out with a good angle? If yes, transfer him to the other side of the screen
            bool angleIsGood=false;
            if(pos.x<10){if((sin(angle)>-0.174)&&(sin(angle)<0.174)){angleIsGood=true; pos.x=prgw-15;}}
            if(pos.y<10){if((cos(angle)>-0.174)&&(cos(angle)<0.174)){angleIsGood=true; pos.y=prgh-15;}}
            if(pos.x>prgw-10){if((sin(angle)>-0.174)&&(sin(angle)<0.174)){angleIsGood=true; pos.x=15;}}
            if(pos.y>prgh-10){if((cos(angle)>-0.174)&&(cos(angle)<0.174)){angleIsGood=true; pos.y=15;}}
            if(angleIsGood){
                // Player made the transfer to the other screen: Adjust his real position
                xDouble=pos.x;
                yDouble=pos.y;
            }else{
                // o_O player didn't make it. He's gotta die
                alive=false;
                return true;
            }
        }
        // Player is inside screen bounds, now check for collisions with other players
        if(gameScreen->hasCollision(pos.x, pos.y,1,1)){
            // o_O player touched something evil and will now die
            alive=false;
            return true;
        }
    }
    return false; // No collision detected.
}

void Player::startBomb(int bombMaxSize, int bombMinSize, int prgw, int prgh, SDL_Surface* sScreen, Player* players[10], GameScreen *gameScreen){
    /// Launches bomb and draws it to screen (flips). Finds opponents in range and kills them using receiveBomb().
    /// For each opponent killed, receive 2 points.

    bombAvail=false;

    // Prepare drawing bomb
    SDL_Surface *sBomb=NULL;
    SDL_Rect rBomb;

    // Randomly determine bomb size (w and h)
    int nw=(rand()%(bombMaxSize-bombMinSize+1))+bombMinSize;
    int nh=(rand()%(bombMaxSize-bombMinSize+1))+bombMinSize;

    // Make sure the bomb is inside the window
    if(pos.x-int((double)nw/2)<0){nw=(pos.x)*2-5;}
    if(pos.y-int((double)nh/2)<0){nh=(pos.y)*2-5;}
    if(pos.x+int((double)nw/2)>prgw){nw=(prgw-(pos.x))*2-5;}
    if(pos.y+int((double)nh/2)>prgw){nh=(prgh-(pos.y))*2-5;}

    // Build the bomb
    sBomb=SDL_CreateRGBSurface(SDL_HWSURFACE,nw,nh,32,0,0,0,0);
    SDL_FillRect(sBomb,NULL,SDL_MapRGB(sBomb->format,color.r,color.g,color.b));
    rBomb.x=pos.x-int((double)sBomb->w/2);
    rBomb.y=pos.y-int((double)sBomb->h/2);

    // Draw the bomb
    SDL_BlitSurface(sBomb,NULL,sScreen,&rBomb);
    SDL_Flip(sScreen);

    alive=false;  // Ugly hack: Temporarly kill the current player to avoid getting points for oneself

    // Collect 2 points for each killed opponent, kill the victim
    for(int i=0;i<10;i++){
        if(players[i]->receiveBomb(rBomb,sBomb)){
            givePoints(2);
        }
    }
    // Undo ugly hack from above
    alive=true;

    // Make a dramatic pause (maybe remove this in future version?)
    SDL_Delay(100);

    // Area left by bomb shall turn black
    gameScreen->clearRect(rBomb.x,rBomb.y,sBomb->w,sBomb->h);
    // TODO: Need to flip here?
    SDL_FreeSurface(sBomb);
}

bool Player::receiveBomb(SDL_Rect rBomb, SDL_Surface* sBomb){
    /// This is called when someone else has launched a bomb.
    /// Check if we're in the range of that bomb and die if yes.
    /// Returns true iff we were killed by that bomb.

    if((pos.x>rBomb.x)&&(pos.y>rBomb.y)&&(pos.x<rBomb.x+sBomb->w)&&(pos.y<rBomb.y+sBomb->h)){
        if(enabled && alive){
            alive=false;
            return true;
        }
    }
    return false;
}

void Player::startTeleport(int prgw, int prgh, SDL_Surface *sScreen, GameScreen *gameScreen){
    /// Launches teleport and writes it to screen, flips it.
    /// Draws a survival square of 100 pixels around the target position.

    teleportAvail=false;

    // Determine where the player will arrive and put him there
    SDL_Rect targetPosition;
    int rMax=prgw-101,rMin=101;
    targetPosition.x=(rand()%(rMax-rMin+1))+rMin;
    rMax=prgh-101;
    targetPosition.y=(rand()%(rMax-rMin+1))+rMin;
    pos=targetPosition;
    xDouble=targetPosition.x;
    yDouble=targetPosition.y;

    // Draw a square that will help him survive there
    gameScreen->clearRect(targetPosition.x-50,targetPosition.y-50,100,100);
    SDL_Flip(sScreen);
}

void Player::doHoles(int slipLevel, GameScreen *gameScreen){
    /// Register current position to hole buffer. If we have gone far enough,
    /// draw the hole. If we're far away from the last hole, reset counter to record new hole at next step.

    if(lives()){
        // If we are currently at a pos where a hole will be drawn, register current pos to holeBackupPos
        if(stepsSinceLastHole<holeSize){
            holeBuffer[stepsSinceLastHole].x=pos.x;
            holeBuffer[stepsSinceLastHole].y=pos.y;
        }

        // If we are 3 steps away from the hole that will be drawn next, draw it
        if(stepsSinceLastHole>=(holeSize+3)){
            for(int j=0;j<holeSize;j++){
                int x=holeBuffer[j].x;
                int y=holeBuffer[j].y;
                gameScreen->clearRect(x-int(slipLevel/2), y-int(slipLevel/2),slipLevel,slipLevel);
            }
        }

        // If we have gone far enough for a new hole, reset stepsSinceLastHole to initiate the new hole
        if(stepsSinceLastHole>=(holeSize+holeDelay)){
            stepsSinceLastHole=0;
        }else{
            stepsSinceLastHole++;
        }
    }
}

void Player::givePoints(int amount){
    /// Give 'amount' points to this player.

    pts+=amount;
}


// Private functions:

bool Player::isLeftDown(Uint8 *keystate){
    /// Reads the one key out of the keystate which corresponds to the left key of this player.
    /// Supports mouse.

    if(leftKeyCode){ // This is keyboard controlled
        return keystate[leftKeyCode];
    }else{
        return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1));
    }
}

bool Player::isRightDown(Uint8 *keystate){
    /// Reads the one key out of the keystate which corresponds to the right key of this player.
    /// Supports mouse.

    if(rightKeyCode){ // This is keyboard controlled
        return keystate[rightKeyCode];
    }else{
        return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3));
    }
}
