/**
  Version 2-2
  Copyright 2009-2014 Sandro Kalbermatter

  LICENSE:
  This program is licenced under GPL v3.0, see http://www.gnu.org/licenses/gpl-3.0
  It comes with absolutely no warranty.

  NOTE:
  I wrote Powerlines while learning C++ in grade school. At that time, I had absolutely
  no knowledge about programming and therefore, the original coding style is horrible.
  I tried to enhance the code 5 years later, starting with version 2-2, but I don't have
  enough spare time to rewrite the entire program.
  I understand that the result is still not state-of-the-art. Sorry for that!

  CHANGELOG since 2-1:
   - Removed feature: Fullscreen
   - Added feature: settings->get("pauseAllowed")
   - Main OS is now Linux
   - Removed any dependencies of SDL_draw
   - Moved position registration in front of collision detection.
     As a consequence, it will not be possible to cross each other on the same pixel any more, even with great timing.
   - Code reforms:
   -- renaming diverse symbols
   -- code reordering
   -- bombs and teleport availability now checked at reset of those vars
   -- removed unused function GetPixel
   -- replaced class player by struct Player
   -- moved bomb used and teleport used into Player struct (now bombAvail and teleportAvail)
   -- moved powerups into Player struct, elminitating powers[10]
   -- Go with only 1 selected player will now just add another one
   -- new function isInRect that simplifies mouse click handing
   -- new function myDrawFillRect which works like drawFillRect but seems to be faster
   -- removed sanstring dependency
   -- settings now read using sscanf
   -- replaced struct Player by class Player (wich has nothing to do with the really old class)
   -- integrated lpressed and rpressed into Player
   -- replaced struct Powerup by class Powerup
   -- created a class for drawing and collision recognition: GameScreen, contains and replaces evilPixels
   -- transformed playerpts into something readable

   TODOs: Many things, in particular:
   - class Powerup is only provisory at the moment. Implement this class.

   **/


#include <cstdlib>
#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <time.h>
#include <math.h>

#include "sanform/sanform_1-1.h"
#include "settings.h"
#include "player.h"
#include "gamescreen.h"

using namespace std;

// Internal global constants
const int amountHandicaps=25; // Defines how many handicaps there will be on the screen
const int keyPressDelay=200;  // When a human player presses both buttons, this is the maximum time distance (between keydowns) for powerups.
// If more time passes by, a bomb / teleport will be released.
const int prgw=739, prgh=555; // Window sizes, ratio 4:3
const int holeDelay=30; // Each holeDelay steps, a hole will be drawn. Must be >= 3
const int holeSize=5; // Defines how long holes are (value in steps)
const double turnSpeed=0.1; // This value will be added to the angle of each player at each step (if the player is rotating).
const double stepSize=3; // Line head will advance by stepSize pixels at each step.
const int bombMaxSize=150,bombMinSize=60;

typedef struct Playerpts{ // Player score display convenience datastructure
    int pts;
    int id;
    bool enabled;
    bool operator() (const Playerpts &el1, const Playerpts &el2) { // This makes sorting really easy
        if(!el1.enabled){
            return false;
        }
        if(!el2.enabled){
            return true;
        }
        return el1.pts > el2.pts;
    }
} Playerpts;

// Functions
int options(SDL_Surface *screen, Settings *settings);
bool isClickInRect(SDL_Event event, int x0, int y0, int x1, int y1);
int checkGo(Player **players);
void myDrawFillRect(SDL_Surface *super,int x,int y, int w, int h, Uint32 color);
int boolScanf(const char* instring, const char* format, bool* target);

int main()
{
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    TTF_Init();

    // Init random generator
    srand(time(NULL));

    // Init settings
    Settings* settings=new Settings("powerlines-settings");
    settings->addSetting("pauseAllowed", "Pausing game: 1=allowed, 0=disallowed",0);
    settings->addSetting("slipLevel","SlipLevel (1-3), 1 means that you can pass easily through other lines, 3 = impossible to pass through lines",3);
    settings->addSetting("holesEnabled","Holes in the lines: 1=active, 0=inactive",1);
    settings->addSetting("amountRoundsInGame","Number of rounds per game. 0 = Never-ending game, -1 = Decide looking at the players' points",-1);
    settings->addSetting("gameDelay","Game speed (default=30, more=slower, less=faster)",30);
    settings->addSetting("showButtons","Show Buttons during the game: 1=yes 0=no",1);
    settings->addSetting("powerupsEnabled","Enable Powerups: 1=yes   0=no",1);
    settings->addSetting("handicapsEnabled","Enable Handicaps: 1=yes   0=no",0);
    settings->addSetting("bombsEnabled","Enable Bombs: 1=yes   0=no",1);
    settings->addSetting("teleportEnabled","Enable Teleport: 1=yes   0=no",1);
    settings->load();

    // Declare program variables
    Player* players[10]; // This table holds the player structure.
    for(int i=0;i<10;i++){
        players[i]=new Player(holeSize,holeDelay);
    }
    Playerpts ppts[10]; // This table holds the player's points at the end of the rounds

    // SDL general variables
    SDL_Surface *sScreen,*sWelcomeScreen,*sIntroScreen,*sExitScreen,*sPlayerSelect,*sTick,*sFont;
    sScreen=NULL; sWelcomeScreen=NULL; sIntroScreen=NULL; sExitScreen=NULL; sPlayerSelect=NULL;sTick=NULL, sFont=NULL;
    SDL_Rect rWelcomeScreen,rExitScreen,rPlayerSelect,rTick1,rTick2,rTick3,rTick4,rTick5,rTick6,rTick7,rTick8,rTick9,rTick10,rFont;
    SDL_Event event;
    TTF_Font *fontDigitLarge=NULL, *fontDigitSmall=NULL;

    // Player colors
    players[0]->setColor({255,   0,   0,   0});
    players[1]->setColor({194, 112,   2,   0});
    players[2]->setColor({255, 255,   0,   0});
    players[3]->setColor({102, 255,  51,   0});
    players[4]->setColor({  1, 103,  18,   0});
    players[5]->setColor({112,  48, 160,   0});
    players[6]->setColor({ 50,  55,  94,   0});
    players[7]->setColor({  0,   0, 255,   0});
    players[8]->setColor({  0, 255, 255,   0});
    players[9]->setColor({255, 255, 255,   0});
    
    // Key codes for each player (except for p10 which is mouse)
    players[0]->setLeftKeyCode(SDLK_1),         players[0]->setRightKeyCode(SDLK_q);
    players[1]->setLeftKeyCode(SDLK_s),         players[1]->setRightKeyCode(SDLK_x);
    players[2]->setLeftKeyCode(SDLK_r),         players[2]->setRightKeyCode(SDLK_t);
    players[3]->setLeftKeyCode(SDLK_c),         players[3]->setRightKeyCode(SDLK_v);
    players[4]->setLeftKeyCode(SDLK_u),         players[4]->setRightKeyCode(SDLK_i);
    players[5]->setLeftKeyCode(SDLK_n),         players[5]->setRightKeyCode(SDLK_m);
    players[6]->setLeftKeyCode(SDLK_LEFT),      players[6]->setRightKeyCode(SDLK_UP);
    players[7]->setLeftKeyCode(SDLK_l),         players[7]->setRightKeyCode(SDLK_p);
    players[8]->setLeftKeyCode(SDLK_KP_DIVIDE), players[8]->setRightKeyCode(SDLK_KP_MULTIPLY);
    players[9]->setLeftKeyCode(0),              players[9]->setRightKeyCode(0); // This is special because player 10 is controlled by mouse.

    // ---------------------------------------- GET WINDOW STARTED! ------------------------------------

    // Load fonts
    fontDigitLarge=TTF_OpenFont("fonts/digifaw.ttf",50);
    fontDigitSmall=TTF_OpenFont("fonts/digifaw.ttf",30);

    // Make Screen
    const char prgname[]="Powerlines_2-2";
    sScreen=SDL_SetVideoMode(prgw,prgh,32,SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_WM_SetCaption(prgname,NULL);
    SDL_WM_SetIcon(SDL_LoadBMP("images/logoklein.bmp"),NULL);
    SDL_FillRect(sScreen,NULL,SDL_MapRGB(sScreen->format,0,0,0));
    GameScreen* gameScreen=new GameScreen(sScreen,prgw,prgh);
    
    // Load and blit Welcome Screen
    rWelcomeScreen.x=0; rWelcomeScreen.y=0;
    sWelcomeScreen=IMG_Load("images/startbild.png");
    SDL_BlitSurface(sWelcomeScreen,NULL,sScreen,&rWelcomeScreen);
    SDL_Flip(sScreen);
    
    // Wait for timeout or event
    bool ok=false;
    int startTime=SDL_GetTicks();
    event.type=SDL_MOUSEMOTION;
    while(!ok)
    {
        SDL_PollEvent(&event);
        if(SDL_GetTicks()-startTime>2000){ok=true;}
        if(event.type==SDL_KEYDOWN){ok=true;}
        if(event.type==SDL_QUIT){ok=true;}
    }
    
    // Set geometry of elements to display
    rTick1.x=292; rTick1.y=27;
    rTick2.x=292; rTick2.y=77;
    rTick3.x=292; rTick3.y=129;
    rTick4.x=292; rTick4.y=182;
    rTick5.x=293; rTick5.y=233;
    rTick6.x=658; rTick6.y=24;
    rTick7.x=657; rTick7.y=75;
    rTick8.x=657; rTick8.y=128;
    rTick9.x=656; rTick9.y=182;
    rTick10.x=656; rTick10.y=235;
    rPlayerSelect.x=0; rPlayerSelect.y=0;

    // Load elements of the main screen (playerselect screen)
    sTick=IMG_Load("images/playerselected.png");
    sPlayerSelect=IMG_Load("images/playerselect.png");

    //////////////////////////////////////////////////////  GAME MAIN ROUTINE  //////////////////////////////////////////////////////////////

    bool quit=false;
    while(!quit) // Absolute Main routine; when this ends, the program is going to terminate
    {
        // Display the main screen ("Playerselect")
        SDL_EnableKeyRepeat(0,0);
        event.type=SDL_MOUSEMOTION;
        bool go=false; // Becomes true when the Go-button is pressed
        while(!quit && !go)
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    quit=true;
                break;

                case SDL_KEYDOWN:
                    // Enable- / Disable-keys
                    for(int i=0;i<9;i++){ // Player 10 is special as it has the mouse.
                        if(event.key.keysym.sym==players[i]->getLeftKeyCode()){
                            players[i]->enable();
                        }else if(event.key.keysym.sym==players[i]->getRightKeyCode()){
                            players[i]->disable();
                        }
                    }

                    // Check for other keys
                    switch(event.key.keysym.sym){

                        // Quit key
                        case SDLK_ESCAPE:
                            quit=true;
                        break;

                            // All-Players-key
                        case SDLK_a:
                            if(!players[0]->isEnabled()){
                                for(int i=0;i<10;i++){
                                    players[i]->enable();
                                }
                            }else{
                                for(int i=0;i<10;i++){
                                    players[i]->disable();
                                }
                            }
                        break;

                            // Options-key
                        case SDLK_o:
                            options(sScreen,settings);
                        break;

                            // Go-key
                        case SDLK_SPACE:
                        {
                            if(checkGo(players)){go=true;}
                        }
                        break;

                        default:
                        break;
                    }
                break;

                case SDL_MOUSEBUTTONDOWN:
                    // Find out coordinates (uncomment for debug)
                    // cout<<"Click on "<<event.button.x<<"."<<event.button.y<<endl;

                    // Player enable / disable
                    if(isClickInRect(event,  20,  30, 330,  80)){ players[0]->toggleEnabled(); }
                    if(isClickInRect(event,  20,  80, 330, 140)){ players[1]->toggleEnabled(); }
                    if(isClickInRect(event,  20, 140, 330, 190)){ players[2]->toggleEnabled(); }
                    if(isClickInRect(event,  20, 190, 330, 240)){ players[3]->toggleEnabled(); }
                    if(isClickInRect(event,  20, 240, 330, 290)){ players[4]->toggleEnabled(); }
                    if(isClickInRect(event, 370,  30, 700,  80)){ players[5]->toggleEnabled(); }
                    if(isClickInRect(event, 370,  80, 700, 140)){ players[6]->toggleEnabled(); }
                    if(isClickInRect(event, 370, 140, 700, 190)){ players[7]->toggleEnabled(); }
                    if(isClickInRect(event, 370, 190, 700, 240)){ players[8]->toggleEnabled(); }
                    if(isClickInRect(event, 370, 240, 700, 290)){ players[9]->toggleEnabled(); }

                    // Other Buttons
                    if(isClickInRect(event, 464, 344, 651, 414)){ options(sScreen,settings); } // Options Button
                    if(isClickInRect(event, 696, 0, 737, 38)){ quit=true; } // Exit Button
                    if(isClickInRect(event, 466, 423, 649, 487)) // Go-Button
                    {
                        if(checkGo(players)){go=true;}
                    }
                break;

                default:
                break;
            }

            // Draw that stuff
            SDL_BlitSurface(sPlayerSelect,NULL,sScreen,&rPlayerSelect);
            if(players[0]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick1);}
            if(players[1]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick2);}
            if(players[2]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick3);}
            if(players[3]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick4);}
            if(players[4]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick5);}
            if(players[5]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick6);}
            if(players[6]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick7);}
            if(players[7]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick8);}
            if(players[8]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick9);}
            if(players[9]->isEnabled()){SDL_BlitSurface(sTick,NULL,sScreen,&rTick10);}
            SDL_Flip(sScreen);

            // Save a few CPU cycles
            SDL_WaitEvent(&event);
        } // Code beyond here is reached when either quit or go are true

        if(quit) // In case the user has requested exit
        {
            goto leaveNow;
        }
        // else, we are ready to go!
        
        //////////////////////////////////////////////////////  BEGIN GAME  //////////////////////////////////////////////////////////////

        // Init Game
        SDL_Surface *sQuitButton,*sPauseButton,*sPlayButton,*sStopButton,*sGreyLogo;
        sQuitButton=NULL; sPauseButton=NULL; sPlayButton=NULL; sStopButton=NULL; sGreyLogo=NULL;
        SDL_Rect rQuitButton,rPauseButton,rPlayButton,rStopButton,rGreyLogo;

        rPlayButton.x=10; rPlayButton.y=5;
        rPauseButton.x=60; rPauseButton.y=5;
        rStopButton.x=110; rStopButton.y=5;
        rQuitButton.x=prgw-53; rQuitButton.y=5;
        rGreyLogo.x=0; rGreyLogo.y=prgh-70;

        sQuitButton=IMG_Load("images/Beenden.png");
        sPauseButton=IMG_Load("images/Pause.png");
        sPlayButton=IMG_Load("images/Play.png");
        sStopButton=IMG_Load("images/Stop.png");
        sGreyLogo=IMG_Load("images/logograu.png");

        enum {PLAY,PAUSE,STOP};
        int status; // This will take one of the 3 values of the enum above

        for(int i=0;i<10;i++){ // Set all points to zero
            players[i]->resetPts();
        }
        int currentRound=0; // This counts how many rounds we have had so far
        
        //////////////////////////////////////////////////////  BEGIN ROUND  //////////////////////////////////////////////////////////////

        while(go&&!quit){
            // Initial drawing
            gameScreen->fillRect(0,0,prgw,prgh,SDL_MapRGB(sScreen->format,255,0,0)); // Draw red frame
            gameScreen->clearRect(10, 10, prgw-20, prgh-20); // Paint rest of the screen black

            // Initialize and draw handicaps for this round
            if(settings->get("handicapsEnabled"))
            {
                int rMax,rMin,nrRand,maxX,minX,maxY,minY,x,y;
                for(int i=0;i<amountHandicaps;i++)
                {
                    // First, decide if this handicap goes left / right / top / bottom and set bounds accordingly
                    rMax=4,rMin=1;
                    nrRand=(rand()%(rMax-rMin+1))+rMin;
                    switch(nrRand)
                    {
                        case 1:
                            // Put this handicap left
                            minX=0;
                            minY=0;
                            maxX=75;
                            maxY=prgh-15;
                        break;

                        case 2:
                            // Put this handicap on top
                            minX=0;
                            minY=0;
                            maxX=prgw-15;
                            maxY=75;
                        break;

                        case 3:
                            // Put this handicap right
                            minX=prgw-75;
                            minY=0;
                            maxX=prgw-15;
                            maxY=prgh-15;
                        break;

                        case 4:
                            // Put this handicap on the bottom
                            minX=0;
                            minY=prgh-75;
                            maxX=prgw-15;
                            maxY=prgh-15;
                        break;
                    }// end switch

                    // Second, determine exact position on screen within the bounds determined by first step
                    x=(rand()%(maxX-minX+1))+minX;
                    y=(rand()%(maxY-minY+1))+minY;

                    // Now, draw the grey square representing the handicap
                    gameScreen->fillRect(x,y,15,15,SDL_MapRGB(sScreen->format,200,200,200));

                } // We're done initializing the handicaps

            }

            // Draw the in-game buttons
            if(settings->get("showButtons")){
                SDL_BlitSurface(sQuitButton,NULL,sScreen,&rQuitButton);
                SDL_BlitSurface(sPauseButton,NULL,sScreen,&rPauseButton);
                SDL_BlitSurface(sPlayButton,NULL,sScreen,&rPlayButton);
                SDL_BlitSurface(sStopButton,NULL,sScreen,&rStopButton);
                SDL_BlitSurface(sGreyLogo,NULL,sScreen,&rGreyLogo);
            }
            SDL_Flip(sScreen);

            // Determine the start position of each player
            for(int i=0;i<10;i++){
                players[i]->initRound(prgw,prgh,settings);
            }

            // Display the players' birth
            for(int i=0;i<10;i++){
                players[i]->drawBirth(sScreen,settings->get("slipLevel"));
            }

            // Wait a second for the humans to get ready
            SDL_Delay(1000);

            // Init round
            bool roundEnabled=true; // When this becomes false, the round will stop!

            SDL_EnableKeyRepeat(1,1);
            status=PLAY; // GO ROUND!

            //////////////////////////////////////////////////////  ROUND LOOP  //////////////////////////////////////////////////////////////

            while(roundEnabled&&!quit&&go){
                event.type=SDL_MOUSEMOTION; // Reset event in order to get clean results
                SDL_PollEvent(&event);
                switch(event.type)
                {
                    case SDL_QUIT:
                        quit=true;
                    break;

                    case SDL_KEYDOWN: // This only covers special keys, player control keys will be handled below
                        switch(event.key.keysym.sym){
                            case SDLK_ESCAPE:
                            {
                                status=STOP;
                                SDL_Event tempevent;
                                tempevent.type=SDL_MOUSEMOTION;
                                bool ok=false;
                                while(!ok){ // Make sure the Escape Key is released before we continue.
                                    SDL_WaitEvent(&tempevent);
                                    if((tempevent.type==SDL_KEYUP)&&(tempevent.key.keysym.sym==SDLK_ESCAPE)){ok=true;}
                                }
                            }
                            break;

                            case SDLK_SPACE:
                            {
                                if(settings->get("pauseAllowed")){
                                    if(status==PAUSE){status=PLAY;}
                                    else if(status==PLAY){status=PAUSE;}
                                    SDL_Event tempevent;
                                    tempevent.type=SDL_MOUSEMOTION;
                                    bool ok=false;
                                    while(!ok){ // Make sure the Space Key is released before we continue.
                                        SDL_WaitEvent(&tempevent);
                                        if((tempevent.type==SDL_KEYUP)&&(tempevent.key.keysym.sym==SDLK_SPACE)){ok=true;}
                                    }
                                }
                            }
                            break;

                            default:
                            break;
                        }
                    break;

                    case SDL_MOUSEBUTTONDOWN:
                    {
                        //cout<<"GClick on "<<event.button.x<<"."<<event.button.y<<endl; // For debug only.
                        if((event.button.x>0)&&(event.button.x<50)&&(event.button.y>0)&&(event.button.y<80)){status=PLAY;}
                        if((event.button.x>50)&&(event.button.x<100)&&(event.button.y>0)&&(event.button.y<80) && settings->get("pauseAllowed")){status=PAUSE;}
                        if((event.button.x>100)&&(event.button.x<150)&&(event.button.y>0)&&(event.button.y<80)){status=STOP;}
                        if((event.button.x>prgw-50)&&(event.button.x<prgw)&&(event.button.y>0)&&(event.button.y<80)){quit=true;}
                    }
                    break;

                    default:
                    break;
                }

                // Check status to know if we are stopped
                if(status==STOP){
                    go=false;
                } // TODO: Would be nice to have go and status combined

                // If we are not paused, get the keystate
                if(status==PLAY){
                    // Analyze keystate and react to it
                    Uint8 *keystate=SDL_GetKeyState(NULL);
                    for(int i=0;i<10;i++){
                        players[i]->react(keystate,keyPressDelay,turnSpeed,bombMaxSize,bombMinSize,prgw,prgh,sScreen,players,gameScreen);
                    }

                    for(int i=0;i<10;i++){
                        // Determine new positions of each player's head and powerups
                        players[i]->moveHead(stepSize);
                        players[i]->movePowerup(stepSize,prgw,prgh);
                    }

                    // Collision detection
                    for(int i=0;i<10;i++){
                        // Check for collisions... here comes the Reaper
                        if(players[i]->collisionTestWithKill(gameScreen,prgw,prgh)){
                            // Player just died, give other players points
                            for(int j=0;j<10;j++){
                                if(i!=j){
                                    if(players[j]->lives()){
                                        players[j]->givePoints(1);
                                    }
                                }
                            }
                        }
                    }

                    for(int i=0;i<10;i++){
                        // Draw and register to evilPixels
                        players[i]->drawHead(gameScreen,settings->get("slipLevel"));
                        players[i]->drawPowerup(gameScreen);
                    }

                    if(settings->get("holesEnabled")){
                        for(int i=0;i<10;i++){
                            players[i]->doHoles(settings->get("slipLevel"),gameScreen);
                        }
                    }
                    SDL_Flip(sScreen);

                    // Is there just one more player left alive? If so, terminate the round
                    int p=0;
                    for(int i=0;i<10;i++){
                        if(players[i]->lives()){p++;}
                    }
                    if(p<=1){roundEnabled=false;}

                }

                // Now, wait a moment to slow the game down
                // TODO: Measure the actually used time in order to guarantee fixed FPS
                SDL_Delay(settings->get("gameDelay"));
            }

            ////////////////////////////////////////////////////  END OF ROUND ///////////////////////////////////////////////////////////
            // Show the points of the round (game is not over yet, there might be another round)
            if(go && !quit){
                // Collect the points into a convenience datastructure
                for(int i=0;i<10;i++){
                    if(players[i]->isEnabled()){
                        ppts[i].pts=players[i]->getPts();
                        ppts[i].id=i;
                        ppts[i].enabled=true;
                    }else{
                        ppts[i].enabled=false;
                    }
                }

                // Sort
                std::sort(ppts,ppts+10, Playerpts());

                // Display the points list
                for(int i=0;i<10;i++){
                    if(ppts[i].enabled){
                        char tt[500];
                        sprintf(tt,"Player %i: %i",ppts[i].id+1,ppts[i].pts);
                        sFont=TTF_RenderText_Blended(fontDigitLarge,tt,players[ppts[i].id]->getColor());
                        rFont.x=250;
                        rFont.y=((i)*50+10);
                        SDL_BlitSurface(sFont,NULL,sScreen,&rFont);
                        SDL_Flip(sScreen);
                    }
                }

                // Display footer message (if user termination criteria is reached, already show that we're done)
                // TODO: The behaviour described in () above makes not much sence
                if((settings->get("amountRoundsInGame")>0)&&(currentRound+1>=settings->get("amountRoundsInGame"))){sFont=TTF_RenderText_Blended(fontDigitSmall,"Finished! Press Space...",players[0]->getColor());}
                else{sFont=TTF_RenderText_Blended(fontDigitSmall,"Press Space...",players[9]->getColor());}
                rFont.y=515;
                SDL_BlitSurface(sFont,NULL,sScreen,&rFont);
                SDL_Flip(sScreen);

                // Wait until user confirms with space or escape
                SDL_Event tempevent;
                tempevent.type=SDL_MOUSEMOTION;
                bool ok=false;
                while(!ok){
                    SDL_WaitEvent(&tempevent);
                    if((tempevent.type==SDL_QUIT)){ok=true; quit=true;}
                    if((tempevent.type==SDL_KEYDOWN)&&(tempevent.key.keysym.sym==SDLK_SPACE)){ok=true;}
                    if((tempevent.type==SDL_KEYDOWN)&&(tempevent.key.keysym.sym==SDLK_ESCAPE)){ok=true; go=false;}
                }
            }

            // We're done with this round
            currentRound++;

            // Check if the game is over (if a player has made enough points)
            int amountActivePlayers=0,bestScore=0;
            for(int i=0;i<10;i++){
                if(players[i]->isEnabled()){amountActivePlayers++; if(players[i]->getPts()>bestScore){bestScore=players[i]->getPts();}}
            }

            bool gameOver=false;
            if(settings->get("amountRoundsInGame")==-1){if(bestScore>=10*(amountActivePlayers-1)){gameOver=true;}} // termination criteria set to auto
            else if((settings->get("amountRoundsInGame")>0)&&(currentRound>=settings->get("amountRoundsInGame"))){gameOver=true;} // user set termination citeria

            ///////////////////////////////////////////////// GAME OVER //////////////////////////////////////////////////////////

            if(gameOver){
                // Terminate the game due to reached termination criteria
                go=false;
                SDL_FillRect(sScreen,NULL,SDL_MapRGB(sScreen->format,0,0,0));
                SDL_Flip(sScreen);

                // The space key must be released in order to continue
                Uint8 *keystate;
                bool ok=false;
                while(!ok){
                    SDL_PollEvent(&event);
                    keystate=SDL_GetKeyState(NULL);
                    if(keystate[SDLK_SPACE]){ok=false;}else{ok=true;}
                }

                // Show the very final points
                for(int i=0;i<10;i++){
                    if(ppts[i].enabled){
                        char tt[500];
                        sprintf(tt,"Player %i:%i",ppts[i].id+1,ppts[i].pts);
                        sFont=TTF_RenderText_Blended(fontDigitLarge,tt,players[ppts[i].id]->getColor());
                        rFont.x=250;
                        rFont.y=((i)*50+10);
                        SDL_BlitSurface(sFont,NULL,sScreen,&rFont);
                        SDL_Flip(sScreen);
                    }
                }
                sFont=TTF_RenderText_Blended(fontDigitSmall,"Finished! Press Space...",players[0]->getColor());
                rFont.y=515;
                SDL_BlitSurface(sFont,NULL,sScreen,&rFont);
                SDL_Flip(sScreen);

                // Wait for user to press space or escape
                SDL_Event tempevent;
                tempevent.type=SDL_MOUSEMOTION;
                ok=false;
                while(!ok){
                    SDL_WaitEvent(&tempevent);
                    if((tempevent.type==SDL_QUIT)){ok=true; quit=true;}
                    if((tempevent.type==SDL_KEYDOWN)&&(tempevent.key.keysym.sym==SDLK_SPACE)){ok=true;}
                    if((tempevent.type==SDL_KEYDOWN)&&(tempevent.key.keysym.sym==SDLK_ESCAPE)){ok=true; go=false;}
                }
            }
        }
        
        // End of game, free ingame surfaces
        if(sQuitButton!=NULL){SDL_FreeSurface(sQuitButton);}
        if(sPauseButton!=NULL){SDL_FreeSurface(sPauseButton);}
        if(sPlayButton!=NULL){SDL_FreeSurface(sPlayButton);}
        if(sStopButton!=NULL){SDL_FreeSurface(sStopButton);}
        if(sGreyLogo!=NULL){SDL_FreeSurface(sGreyLogo);}
    }

    ///////////////////////////////////////////////////////// PROGRAM EXIT ///////////////////////////////////////////
    
leaveNow:
    // Display exit screen
    rExitScreen.x=0; rExitScreen.y=0;
    sExitScreen=IMG_Load("images/adieu.png");
    SDL_BlitSurface(sExitScreen,NULL,sScreen,&rExitScreen);
    SDL_Flip(sScreen);
    
    // Wait for timeout or user input
    startTime=SDL_GetTicks();
    event.type=SDL_MOUSEMOTION;
    ok=false;
    while(!ok){
        // This is a busy wait loop to avoid being stuck at SDL_WaitEvent when no user input is given
        SDL_PollEvent(&event);
        if(SDL_GetTicks()-startTime>3000){ok=true;}
        if(event.type==SDL_KEYDOWN){ok=true;}
        if(event.type==SDL_QUIT){ok=true;}
    }
    
    // Free and Close
    if(sWelcomeScreen!=NULL){SDL_FreeSurface(sWelcomeScreen);}
    if(sIntroScreen!=NULL){SDL_FreeSurface(sIntroScreen);}
    if(sExitScreen!=NULL){SDL_FreeSurface(sExitScreen);}
    if(sPlayerSelect!=NULL){SDL_FreeSurface(sPlayerSelect);}
    if(sTick!=NULL){SDL_FreeSurface(sTick);}
    if(sFont!=NULL){SDL_FreeSurface(sFont);}
    
    TTF_CloseFont(fontDigitLarge);
    TTF_CloseFont(fontDigitSmall);
    
    TTF_Quit();
    SDL_Quit();

    delete settings;
    delete gameScreen;
    for(int i=0;i<10;i++){
        delete players[i];
    }
    
    return EXIT_SUCCESS;
}




int options(SDL_Surface *screen, Settings* settings)
{
    /// This is a function that displays the option menu.
    // And no, that's not elegant programming. I might change this if I'm up to it one day...
    SDL_Surface *soptions;
    soptions=IMG_Load("images/options.png");
    
    sanform kpause(SF_CHECKBOX);
    sanform kbuttons(SF_CHECKBOX);
    sanform kholes(SF_CHECKBOX);
    sanform khandicaps(SF_CHECKBOX);
    sanform kpowerups(SF_CHECKBOX);
    sanform kbombs(SF_CHECKBOX);
    sanform kteleport(SF_CHECKBOX);
    sanform kspeed(SF_TRACKBAR);
    sanform ksliplevel(SF_TRACKBAR);
    sanform kok(SF_BUTTON);
    sanform kcancel(SF_BUTTON);
    sanform kreset(SF_BUTTON);
    
    sanform lpause(SF_LABEL);
    sanform lbuttons(SF_LABEL);
    sanform lholes(SF_LABEL);
    sanform lhandicaps(SF_LABEL);
    sanform lpowerups(SF_LABEL);
    sanform lbombs(SF_LABEL);
    sanform lteleport(SF_LABEL);
    sanform lspeed(SF_LABEL);
    sanform lsliplevel(SF_LABEL);
    
    SDL_Event event;
    bool ok=false,cancel=false,reset=false;
    
    kpause.x(30); kpause.y(170); kpause.w(20); kpause.h(20);
    kbuttons.x(30); kbuttons.y(200); kbuttons.w(20); kbuttons.h(20);
    kholes.x(30); kholes.y(230); kholes.w(20); kholes.h(20);
    khandicaps.x(30); khandicaps.y(260); khandicaps.w(20); khandicaps.h(20);
    kpowerups.x(360); kpowerups.y(170); kpowerups.w(20); kpowerups.h(20);
    kbombs.x(360); kbombs.y(200); kbombs.w(20); kbombs.h(20);
    kteleport.x(360); kteleport.y(230); kteleport.w(20); kteleport.h(20);
    kspeed.x(30); kspeed.y(300); kspeed.w(300); kspeed.h(20);
    ksliplevel.x(30); ksliplevel.y(330); ksliplevel.w(200); ksliplevel.h(20);
    kok.x(300); kok.y(500); kok.w(130); kok.h(40);
    kcancel.x(450); kcancel.y(500); kcancel.w(130); kcancel.h(40);
    kreset.x(600); kreset.y(500); kreset.w(130); kreset.h(40);
    
    lpause.x(60); lpause.y(173);
    lbuttons.x(60); lbuttons.y(203);
    lholes.x(60); lholes.y(233);
    lhandicaps.x(60); lhandicaps.y(263);
    lpowerups.x(390); lpowerups.y(173);
    lbombs.x(390); lbombs.y(203);
    lteleport.x(390); lteleport.y(233);
    lspeed.x(340); lspeed.y(303);
    lsliplevel.x(240); lsliplevel.y(333);
    
    lpause.color(150,150,0);
    lbuttons.color(150,150,0);
    lholes.color(150,150,0);
    lhandicaps.color(150,150,0);
    lpowerups.color(150,150,0);
    lbombs.color(150,150,0);
    lteleport.color(150,150,0);
    lspeed.color(150,150,0);
    lsliplevel.color(150,150,0);
    
    lpause.label("allow game pausing");
    lbuttons.label("show buttons during game");
    lholes.label("activate holes in lines");
    lhandicaps.label("activate border handicaps");
    lpowerups.label("activate powerups");
    lbombs.label("activate bombs");
    lteleport.label("activate teleport");
    lspeed.label("speed (depending on your computer)");
    lsliplevel.label("SlipLevel (chance to pass through lines)");
    
    kok.label("ok");
    kcancel.label("cancel");
    kreset.label("reset");
    
    ksliplevel.color(50,0,200);
    kspeed.color(150,0,0);
    
    ksliplevel.max(2);
    kspeed.max(99);
    
    kok.color(0,100,0);
    kcancel.color(150,0,0);
    kreset.color(0,0,150);

    if(settings->get("pauseAllowed")){kpause.value(1);}else{kpause.value(0);}
    if(settings->get("showButtons")){kbuttons.value(1);}else{kbuttons.value(0);}
    if(settings->get("holesEnabled")){kholes.value(1);}else{kholes.value(0);}
    if(settings->get("powerupsEnabled")){kpowerups.value(1);}else{kpowerups.value(0);}
    if(settings->get("handicapsEnabled")){khandicaps.value(1);}else{khandicaps.value(0);}
    if(settings->get("bombsEnabled")){kbombs.value(1);}else{kbombs.value(0);}
    if(settings->get("teleportEnabled")){kteleport.value(1);}else{kteleport.value(0);}
    
    kspeed.value(99-settings->get("gameDelay"));
    kspeed.refresh();
    
    ksliplevel.value(3-settings->get("slipLevel"));
    ksliplevel.refresh();
    
    SDL_BlitSurface(soptions,NULL,screen,NULL);
    
    while(!ok&&!cancel){
        kpause.ablit(screen);
        kbuttons.ablit(screen);
        kholes.ablit(screen);
        khandicaps.ablit(screen);
        kpowerups.ablit(screen);
        kbombs.ablit(screen);
        kteleport.ablit(screen);
        kspeed.ablit(screen);
        ksliplevel.ablit(screen);
        kok.ablit(screen);
        kcancel.ablit(screen);
        kreset.ablit(screen);

        lpause.ablit(screen);
        lbuttons.ablit(screen);
        lholes.ablit(screen);
        lhandicaps.ablit(screen);
        lpowerups.ablit(screen);
        lbombs.ablit(screen);
        lteleport.ablit(screen);
        lspeed.ablit(screen);
        lsliplevel.ablit(screen);

        SDL_Flip(screen);


        event.type=SDL_MOUSEMOTION;
        SDL_WaitEvent(&event);

        kpause.linkEvent(event);
        kbuttons.linkEvent(event);
        kholes.linkEvent(event);
        khandicaps.linkEvent(event);
        kpowerups.linkEvent(event);
        kbombs.linkEvent(event);
        kteleport.linkEvent(event);
        kspeed.linkEvent(event);
        ksliplevel.linkEvent(event);
        kok.linkEvent(event);
        kcancel.linkEvent(event);
        kreset.linkEvent(event);




        switch(event.type){
            case SDL_QUIT:
                cancel=true;
            break;

            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        cancel=true;
                    break;

                    case SDLK_RETURN:
                        ok=true;
                    break;

                    case SDLK_KP_ENTER:
                        ok=true;
                    break;

                    case SDLK_r:
                        reset=true;
                    break;

                    default:
                    break;
                }
            }// end Keydown

            default:
            break;
        }// end switch

        if(kok.isClicked()){ok=true;}
        if(kcancel.isClicked()){cancel=true;}
        if(kreset.isClicked()){reset=true;}

        if(ok){
            settings->set("pauseAllowed",kpause.value());
            settings->set("showButtons",kbuttons.value());
            settings->set("holesEnabled",kholes.value());
            settings->set("handicapsEnabled",khandicaps.value());
            settings->set("bombsEnabled",kbombs.value());
            settings->set("teleportEnabled",kteleport.value());
            settings->set("powerupsEnabled",kpowerups.value());

            settings->set("gameDelay",99-kspeed.value());
            settings->set("slipLevel",3-ksliplevel.value());

            settings->save();
        }

        if(reset){
            kpause.value(0);
            kbuttons.value(1);
            kholes.value(1);
            kpowerups.value(1);
            khandicaps.value(0);
            kbombs.value(1);
            kteleport.value(1);

            kspeed.value(69);
            ksliplevel.value(0);
            reset=false;
        }
    }
    return 1;
}


bool isClickInRect(SDL_Event event, int x0, int y0, int x1, int y1){
    /// Returns true iff a click has been made within these screen coordinates

    if(event.type == SDL_MOUSEBUTTONDOWN){
        if(
                (event.button.x > x0) && // Top left corner
                (event.button.y > y0) &&
                (event.button.x < x1) && // Bottom right corner
                (event.button.y < y1)
                ){
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0; // Invalid event type, therefore this is not a click within these bounds
    }
}


int checkGo(Player** players){
    /// This function ensures that there is a valid set of players for the game. Yes: returns true, No: Suggests a corrected set.

    // If necessary, add one or two players to form a valid set of players
    int n=0;
    for(int i=0;i<10;i++)
    {
        if(players[i]->isEnabled()){n++;}
    }
    switch(n){
        case 0:
            players[0]->enable();
            players[7]->enable();
        break;

        case 1:
            if(players[0]->isEnabled()){
                players[7]->enable();
            }else{
                players[0]->enable();
            }
        break;

        default:
        return true;
        break;
    }

    return false;
}

void myDrawFillRect(SDL_Surface *super,int x,int y, int w, int h, Uint32 color){
    /// This function works exactly like drawFillRect, but it seems to be faster

    SDL_Rect location;
    location.x=x;
    location.y=y;
    location.w=w;
    location.h=h;
    SDL_FillRect(super,&location,color);
}

int boolScanf(const char* instring, const char* format, bool* target){
    /// This function works like a sscanf for exaclty one bool variable
    /// It's a rather ugly and lazy but very convenient of sscanf not supporting bool.

    int readInt;
    if(sscanf(instring,format,&readInt) != 1){return 0;}
    if(readInt){*target=true;}else{*target=false;}
    return 1;
}
