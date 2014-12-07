/**
  Version 2-2
  Copyright 2009-2014 Sandro Kalbermatter

  LICENSE:
  This program is licenced under GPL v3.0     TODO: Link goes here
  It comes with absolutely no warranty.

  NOTE:
  I wrote Powerlines while learning C++ in grade school. At that time, I had absolutely
  no knowledge about programming and therefore, the original coding style is horrible.
  I tried to enhance the code 5 years later, starting with version 2-2, but I don't have
  enough spare time to rewrite the entire program.
  I understand that the result is still not state-of-the-art. Sorry for that!

  CHANGELOG since 2-1:
   - Removed feature: Fullscreen
   - Added feature: pauseAllowed
   - Main OS is now Linux
   - Removed any dependencies of SDL_draw
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
   **/


#include <cstdlib>
#include <iostream>
#include <SDL/SDL.h>
#include <SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <time.h>
#include <math.h>

#include "sanform/sanform_1-1.h"

using namespace std;

// User-defined Option globals
bool pauseAllowed=false;
bool showButtons=true;
bool holesEnabled=true;
bool powerupsEnabled=true;
bool handicapsEnabled=false;
bool bombsEnabled=true;
bool teleportEnabled=true;
int slipLevel=3;
int gameDelay=30; // This is for da speed
int amountRoundsInGame=10;

// Internal global constants
const int amountHandicaps=25; // Defines how many handicaps there will be on the screen
const int keyPressDelay=200;  // When a human player presses both buttons, this is the maximum time distance (between keydowns) for powerups.
// If more time passes by, a bomb / teleport will be released.
const int prgw=739, prgh=555; // Window sizes, ratio 4:3
const char* fileNameSettings="powerlines-settings"; // File name of /the settings file
const int holeDelay=30; // Each holeDelay steps, a hole will be drawn. Must be >= 3
const int holeSize=5; // Defines how long holes are (value in steps)
const double turnSpeed=0.1; // This value will be added to the angle of each player at each step (if the player is rotating).
const double stepSize=3; // Line head will advance by stepSize pixels at each step.
const int bombMaxSize=150,bombMinSize=60;

struct Powerup
{   // Note: Powerups automatically thake the color of their player.
    bool running; // I the powerup currently being drawn on screen (until it touches the wall)?
    bool available; // May the player use the powerup?
    SDL_Rect pos[2]; // Position of the 2 heads of the powerup (on the screen, rounded)
    double angle[2]; // Angles of the 2 heads
    double xDouble[2]; // real x-position of the 2 heads
    double yDouble[2]; // real y-position of the 2 heads
};

typedef struct Player{
    bool enabled; // does the player exist in the game?
    bool alive; // is the player alive this round?
    int pts; // How many points did the player collect in this game?
    SDL_Color color; // Color of the player's line
    double xDouble,yDouble; // Current real position of the head of the line
    SDL_Rect pos; // Current position on screen of the head of the line (rounded to closest pixel)
    double angle; // What direction is the player's line currently facing?
    bool bombAvail; // Does the player have the right to use the bomb now?
    bool teleportAvail; // Does the player have the right to use teleport now?
    struct Powerup powerup;
    int leftKeyCode, rightKeyCode; // This holds the keymap
} Player;

struct playerpts{ // Player score display convenience datastructure
    int pts;
    int nr;
    bool enabled;
};

// Functions
int options(SDL_Surface *screen);
bool isClickInRect(SDL_Event event, int x0, int y0, int x1, int y1);
int checkGo(Player* players);
void myDrawFillRect(SDL_Surface *super,int x,int y, int w, int h, Uint32 color);
int boolScanf(const char* instring, const char* format, bool* target);

int main(int argc, char *argv[])
{
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    TTF_Init();

    // Init random generator
    srand(time(NULL));

    // Read / Write settings
    FILE *fSettings;
    fSettings=fopen(fileNameSettings,"r");
    if(fSettings){
        // The settings file exists. Read it and apply settings.
        char lineBuf[500];
        bool readSuccess=true;
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(boolScanf(lineBuf,"%i\t%*",&pauseAllowed) != 1){pauseAllowed=false;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(sscanf(lineBuf,"%i\t%*",&slipLevel) != 1 || slipLevel<1 || slipLevel >3){slipLevel=3;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(boolScanf(lineBuf,"%i\t%*",&holesEnabled) != 1){holesEnabled=true;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(sscanf(lineBuf,"%i\t%*",&amountRoundsInGame) != 1){amountRoundsInGame=-1;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(sscanf(lineBuf,"%i\t%*",&gameDelay) != 1 || gameDelay<0){gameDelay=30;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(boolScanf(lineBuf,"%i\t%*",&showButtons) != 1){showButtons=true;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(boolScanf(lineBuf,"%i\t%*",&powerupsEnabled) != 1){powerupsEnabled=true;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(boolScanf(lineBuf,"%i\t%*",&handicapsEnabled) != 1){handicapsEnabled=true;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(boolScanf(lineBuf,"%i\t%*",&bombsEnabled) != 1){bombsEnabled=true;}
        if(!fgets(lineBuf,500,fSettings)){readSuccess=false;}
        else if(boolScanf(lineBuf,"%i\t%*",&teleportEnabled) != 1){teleportEnabled=true;}
        if(!readSuccess){
            fprintf(stderr,"WARNING: Corrupted settings file. Restoring to default.\n");
            // Restore default settings file as the actual file is corrupted
            fSettings=fopen(fileNameSettings,"w+");
            if(fSettings){
                fprintf(fSettings,"0\tPausing game: 1=allowed, 0=disallowed\n3\tSlipLevel (1-3), 1 means that you can pass easily through other lines, 3 = impossible to pass through lines\n1\tHoles in the lines: 1=active, 0=inactive\n-1\tNumber of rounds per game. 0 = Never-ending game, -1 = Decide after points\n30\tGame speed (std=30, more=slower, less=faster)\n1\tShow Buttons during the game: 1=yes 0=no\n1\tUse Powerups      1=yes   0=no\n0\tActivate handicaps       1=yes     0=no\n1\tActivate bombs       1=yes    0=no\n1\tActivate teleport       1=yes     0=no");
            }else{
                fprintf(stderr,"WARNING: Could not create settings file %s!\n",fileNameSettings);
            }
            pauseAllowed=0;
            slipLevel=3;
            holesEnabled=true;
            amountRoundsInGame=-1;
            gameDelay=30;
            showButtons=true;
            powerupsEnabled=true;
            handicapsEnabled=false;
            bombsEnabled=true;
            teleportEnabled=true;
        }
    }else{
        // No settings file found. Create one and set settings to default.
        fSettings=fopen(fileNameSettings,"w+");
        if(fSettings){
            fprintf(fSettings,"0\tPausing game: 1=allowed, 0=disallowed\n3\tSlipLevel (1-3), 1 means that you can pass easily through other lines, 3 = impossible to pass through lines\n1\tHoles in the lines: 1=active, 0=inactive\n-1\tNumber of rounds per game. 0 = Never-ending game, -1 = Decide after points\n30\tGame speed (std=30, more=slower, less=faster)\n1\tShow Buttons during the game: 1=yes 0=no\n1\tUse Powerups      1=yes   0=no\n0\tActivate handicaps       1=yes     0=no\n1\tActivate bombs       1=yes    0=no\n1\tActivate teleport       1=yes     0=no");
        }else{
            fprintf(stderr,"WARNING: Could not create settings file %s!\n",fileNameSettings);
        }
        pauseAllowed=0;
        slipLevel=3;
        holesEnabled=true;
        amountRoundsInGame=-1;
        gameDelay=30;
        showButtons=true;
        powerupsEnabled=true;
        handicapsEnabled=false;
        bombsEnabled=true;
        teleportEnabled=true;
    }
    fclose(fSettings);

    // Declare program variables
    Player players[10]; // This table holds the player structure.
    struct playerpts ppts[10]; // This table holds the player's points at the end of the rounds (TODO: entire pts system sould be reworked)
    int lpressed[10],rpressed[10];  // These tables store what key is pressed for each player, int=SDL_GetTicks()
    // TODO: Integrate lpressed and rpressed into players[]

    // SDL general variables
    SDL_Surface *sScreen,*sWelcomeScreen,*sIntroScreen,*sExitScreen,*sPlayerSelect,*sTick,*sFont;
    sScreen=NULL; sWelcomeScreen=NULL; sIntroScreen=NULL; sExitScreen=NULL; sPlayerSelect=NULL;sTick=NULL, sFont=NULL;
    SDL_Rect rWelcomeScreen,rExitScreen,rPlayerSelect,rTick1,rTick2,rTick3,rTick4,rTick5,rTick6,rTick7,rTick8,rTick9,rTick10,rFont;
    SDL_Event event;
    TTF_Font *fontDigitLarge=NULL, *fontDigitSmall=NULL;

    // Player colors
    players[0].color={255,   0,   0};
    players[1].color={194, 112,   2};
    players[2].color={255, 255,   0};
    players[3].color={102, 255,  51};
    players[4].color={  1, 103,  18};
    players[5].color={112,  48, 160};
    players[6].color={ 50,  55,  94};
    players[7].color={  0,   0, 255};
    players[8].color={  0, 255, 255};
    players[9].color={255, 255, 255};
    
    // Key codes for each player (except for p10 which is mouse)
    players[0].leftKeyCode=SDLK_1,         players[0].rightKeyCode=SDLK_q;
    players[1].leftKeyCode=SDLK_s,         players[1].rightKeyCode=SDLK_x;
    players[2].leftKeyCode=SDLK_r,         players[2].rightKeyCode=SDLK_t;
    players[3].leftKeyCode=SDLK_c,         players[3].rightKeyCode=SDLK_v;
    players[4].leftKeyCode=SDLK_u,         players[4].rightKeyCode=SDLK_i;
    players[5].leftKeyCode=SDLK_n,         players[5].rightKeyCode=SDLK_m;
    players[6].leftKeyCode=SDLK_LEFT,      players[6].rightKeyCode=SDLK_UP;
    players[7].leftKeyCode=SDLK_l,         players[7].rightKeyCode=SDLK_p;
    players[8].leftKeyCode=SDLK_KP_DIVIDE, players[8].rightKeyCode=SDLK_KP_MULTIPLY;
    players[9].leftKeyCode=0,              players[9].rightKeyCode=0; // This is special because player 10 is controlled by mouse.

    // Init evilPixels
    // If the head of a player touches an evil pixel, he will die.
    bool evilPixels[prgw][prgh];
    for(int i=0;i<prgw;i++){
        for(int j=0;j<prgh;j++){
            evilPixels[i][j]=false;
        }
    }

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
                        if(event.key.keysym.sym==players[i].leftKeyCode){
                            players[i].enabled=true;
                        }else if(event.key.keysym.sym==players[i].rightKeyCode){
                            players[i].enabled=false;
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
                            if(!players[0].enabled){
                                for(int i=0;i<10;i++){
                                    players[i].enabled=true;
                                }
                            }else{
                                for(int i=0;i<10;i++){
                                    players[i].enabled=false;
                                }
                            }
                        break;

                            // Options-key
                        case SDLK_o:
                            options(sScreen);
                        break;

                            // Go-key
                        case SDLK_SPACE:
                        {
                            if(checkGo(players)){go=true;}
                        }
                        break;
                    }
                break;

                case SDL_MOUSEBUTTONDOWN:
                    // Find out coordinates (uncomment for debug)
                    // cout<<"Click on "<<event.button.x<<"."<<event.button.y<<endl;

                    // Player enable / disable
                    if(isClickInRect(event,  20,  30, 330,  80)){ players[0].enabled = !players[0].enabled; }
                    if(isClickInRect(event,  20,  80, 330, 140)){ players[1].enabled = !players[1].enabled; }
                    if(isClickInRect(event,  20, 140, 330, 190)){ players[2].enabled = !players[2].enabled; }
                    if(isClickInRect(event,  20, 190, 330, 240)){ players[3].enabled = !players[3].enabled; }
                    if(isClickInRect(event,  20, 240, 330, 290)){ players[4].enabled = !players[4].enabled; }
                    if(isClickInRect(event, 370,  30, 700,  80)){ players[5].enabled = !players[5].enabled; }
                    if(isClickInRect(event, 370,  80, 700, 140)){ players[6].enabled = !players[6].enabled; }
                    if(isClickInRect(event, 370, 140, 700, 190)){ players[7].enabled = !players[7].enabled; }
                    if(isClickInRect(event, 370, 190, 700, 240)){ players[8].enabled = !players[8].enabled; }
                    if(isClickInRect(event, 370, 240, 700, 290)){ players[9].enabled = !players[9].enabled; }

                    // Other Buttons
                    if(isClickInRect(event, 464, 344, 651, 414)){ options(sScreen); } // Options Button
                    if(isClickInRect(event, 696, 0, 737, 38)){ quit=true; } // Exit Button
                    if(isClickInRect(event, 466, 423, 649, 487)) // Go-Button
                    {
                        if(checkGo(players)){go=true;}
                    }
                break;
            }

            // Draw that stuff
            SDL_BlitSurface(sPlayerSelect,NULL,sScreen,&rPlayerSelect);
            if(players[0].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick1);}
            if(players[1].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick2);}
            if(players[2].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick3);}
            if(players[3].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick4);}
            if(players[4].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick5);}
            if(players[5].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick6);}
            if(players[6].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick7);}
            if(players[7].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick8);}
            if(players[8].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick9);}
            if(players[9].enabled){SDL_BlitSurface(sTick,NULL,sScreen,&rTick10);}
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
            players[i].pts=0;
        }
        int currentRound=0; // This counts how many rounds we have had so far
        
        //////////////////////////////////////////////////////  BEGIN ROUND  //////////////////////////////////////////////////////////////

        while(go&&!quit){
            // Initial drawing
            SDL_FillRect(sScreen,NULL,SDL_MapRGB(sScreen->format,255,0,0)); // Draw red frame
            myDrawFillRect(sScreen, 10, 10, prgw-20, prgh-20, SDL_MapRGB(sScreen->format,0,0,0)); // Paint rest of the screen black

            // Set all evilPixels to "harmless"
            for(int i=0;i<prgw;i++)
            {
                for(int j=0;j<prgh;j++)
                {
                    evilPixels[i][j]=false;
                }
            }

            // Initialize and draw handicaps for this round
            if(handicapsEnabled)
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
                    myDrawFillRect(sScreen,x,y,15,15,SDL_MapRGB(sScreen->format,200,200,200));

                    // Register the handicap into evilPixels
                    for(int j=0;j<15;j++)
                    {
                        for(int k=0;k<15;k++)
                        {
                            evilPixels[x+j][y+k]=true;
                        }
                    }

                } // We're done initializing the handicaps

            }

            // Draw the in-game buttons
            if(showButtons){
                SDL_BlitSurface(sQuitButton,NULL,sScreen,&rQuitButton);
                SDL_BlitSurface(sPauseButton,NULL,sScreen,&rPauseButton);
                SDL_BlitSurface(sPlayButton,NULL,sScreen,&rPlayButton);
                SDL_BlitSurface(sStopButton,NULL,sScreen,&rStopButton);
                SDL_BlitSurface(sGreyLogo,NULL,sScreen,&rGreyLogo);
            }
            SDL_Flip(sScreen);

            // Determine the start position of each player
            SDL_Rect tempRect;
            int MAX=prgw-100,MIN=100;
            for(int i=0;i<10;i++){
                // Random coordinates
                tempRect.x= (rand() % (MAX - MIN + 1)) + MIN;
                MAX=(prgh-100),MIN=100;
                tempRect.y= (rand() % (MAX - MIN + 1)) + MIN;
                players[i].pos=tempRect;
                players[i].xDouble=tempRect.x;
                players[i].yDouble=tempRect.y;

                // Random start angle
                players[i].angle=rand()%361;

                // Resurrection of the player
                players[i].alive=true;

                // Init powerup
                players[i].powerup.running=false;
                players[i].powerup.available=true;

                // Init bomb and teleport
                if(bombsEnabled){players[i].bombAvail=true;}else{players[i].bombAvail=false;}
                if(teleportEnabled){players[i].teleportAvail=true;}else{players[i].teleportAvail=false;}

                // Force keys off
                lpressed[i]=0;
                rpressed[i]=0;
            } // Done initializing players

            // Display the players' birth
            Uint32 color;
            for(int i=0;i<10;i++){
                if(players[i].enabled){
                    color=SDL_MapRGB(sScreen->format,players[i].color.r,players[i].color.g,players[i].color.b);
                    myDrawFillRect(sScreen, players[i].pos.x-1, players[i].pos.y-1,3,3, color);
                    SDL_Flip(sScreen);
                    SDL_Delay(200);
                }
            }

            // Wait a second for the humans to get ready
            SDL_Delay(1000);

            // Init round
            int stepsSinceLastHole=0; // Ring that wraps every time a hole is drawn
            SDL_Rect holeBackupPos[10][holeSize]; // This will save each pixel where a hole will need to be drawn
            for(int i=0;i<10;i++)
            {
                for(int j=0;j<holeSize;j++)
                {
                    holeBackupPos[i][j].x=0;
                    holeBackupPos[i][j].y=0;
                }
            }

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
                                if(pauseAllowed){
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

                        }
                    break;

                    case SDL_MOUSEBUTTONDOWN:
                    {
                        //cout<<"GClick on "<<event.button.x<<"."<<event.button.y<<endl; // For debug only.
                        if((event.button.x>0)&&(event.button.x<50)&&(event.button.y>0)&&(event.button.y<80)){status=PLAY;}
                        if((event.button.x>50)&&(event.button.x<100)&&(event.button.y>0)&&(event.button.y<80) && pauseAllowed){status=PAUSE;}
                        if((event.button.x>100)&&(event.button.x<150)&&(event.button.y>0)&&(event.button.y<80)){status=STOP;}
                        if((event.button.x>prgw-50)&&(event.button.x<prgw)&&(event.button.y>0)&&(event.button.y<80)){quit=true;}
                    }

                }





                // Check status to know if we are stopped
                if(status==STOP){go=false;} // TODO: Would be nice to have go and status combined

                // If we are not paused, get the keystate
                if(status==PLAY)
                {
                    // Analyze keystate and react to it
                    Uint8 *keystate=SDL_GetKeyState(NULL);

                    for(int i=0;i<9;i++){
                        if(!(abs(rpressed[i]-lpressed[i])>keyPressDelay) && (keystate[players[i].leftKeyCode]) && (keystate[players[i].rightKeyCode]) && players[i].powerup.available){
                            // Release a powerup!
                            players[i].powerup.available=false;
                            players[i].powerup.running=true;
                            players[i].powerup.angle[0]=players[i].angle-90;
                            players[i].powerup.angle[1]=players[i].angle+90;
                            players[i].powerup.pos[0]=players[i].pos;
                            players[i].powerup.pos[1]=players[i].pos;
                            players[i].powerup.xDouble[0]=players[i].powerup.xDouble[1]=players[i].pos.x;
                            players[i].powerup.yDouble[0]=players[i].powerup.yDouble[1]=players[i].pos.y;
                        }else if(keystate[players[i].leftKeyCode]){
                            // Turn left
                            players[i].angle=players[i].angle-turnSpeed;
                        }else if(keystate[players[i].rightKeyCode]){
                            // Turn right
                            players[i].angle=players[i].angle+turnSpeed;
                        }
                        if(!keystate[players[i].leftKeyCode]){lpressed[i]=0;} // When left button is released, reset its timecode
                        if(!keystate[players[i].rightKeyCode]){rpressed[i]=0;} // Same thing for right button
                        if(!lpressed[i]&&keystate[players[i].leftKeyCode]){lpressed[i]=SDL_GetTicks();} // When left button is pressed, register its timecode
                        if(!rpressed[i]&&keystate[players[i].rightKeyCode]){rpressed[i]=SDL_GetTicks();} // Same thing for right button
                    }

                    // Player 10 is special because he's got the mouse
                    // For explanation, refer to comments above
                    SDL_PumpEvents();
                    if(!(abs(rpressed[9]-lpressed[9])>keyPressDelay)&&(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))&&(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(3))&&players[9].powerup.available){
                        players[9].powerup.available=false;
                        players[9].powerup.running=true;
                        players[9].powerup.angle[0]=players[9].angle-90;
                        players[9].powerup.angle[1]=players[9].angle+90;
                        players[9].powerup.pos[0]=players[9].pos;
                        players[9].powerup.pos[1]=players[9].pos;
                        players[9].powerup.xDouble[0]=players[9].powerup.xDouble[1]=players[9].pos.x;
                        players[9].powerup.yDouble[0]=players[9].powerup.yDouble[1]=players[9].pos.y;
                    }else if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1)){
                        players[9].angle=players[9].angle-turnSpeed;
                    }else if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(3)){
                        players[9].angle=players[9].angle+turnSpeed;
                    }
                    if(!(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))){lpressed[9]=0;}
                    if(!(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(3))){rpressed[9]=0;}
                    if(!lpressed[9]&&(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))){lpressed[9]=SDL_GetTicks();}
                    if(!rpressed[9]&&(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(3))){rpressed[9]=SDL_GetTicks();}

                    // Determine new positions of each player's head and powerups
                    for(int i=0;i<10;i++)
                    {
                        // Line head
                        players[i].xDouble+=(double)stepSize*cos(players[i].angle); // Move line head by one step
                        players[i].yDouble+=(double)stepSize*sin(players[i].angle);
                        players[i].pos.x=(int)round(players[i].xDouble); // Round screen position to closest pixel
                        players[i].pos.y=(int)round(players[i].yDouble);

                        // Powerup first arm
                        players[i].powerup.xDouble[0]+=3*(double)stepSize*cos(players[i].powerup.angle[0]);
                        players[i].powerup.yDouble[0]+=3*(double)stepSize*sin(players[i].powerup.angle[0]);
                        players[i].powerup.pos[0].x=(int)round(players[i].powerup.xDouble[0]);
                        players[i].powerup.pos[0].y=(int)round(players[i].powerup.yDouble[0]);
                        // Powerup second arm
                        players[i].powerup.xDouble[1]+=3*(double)stepSize*cos(players[i].powerup.angle[1]);
                        players[i].powerup.yDouble[1]+=3*(double)stepSize*sin(players[i].powerup.angle[1]);
                        players[i].powerup.pos[1].x=(int)round(players[i].powerup.xDouble[1]);
                        players[i].powerup.pos[1].y=(int)round(players[i].powerup.yDouble[1]);
                    }

                    // Check for collisions... here comes the Reaper
                    for(int i=0;i<10;i++){
                        if(players[i].enabled&&players[i].alive){
                            // Check if the player has touched the screen
                            if((players[i].pos.x<10)||(players[i].pos.y<10)||(players[i].pos.x>prgw-10)||(players[i].pos.y>prgh-10)){
                                // The player is outside the borders! Did he get out with a good angle? If yes, transfer him to the other side of the screen
                                bool angleIsGood=false;
                                if(players[i].pos.x<10){if((sin(players[i].angle)>-0.174)&&(sin(players[i].angle)<0.174)){angleIsGood=true; players[i].pos.x=prgw-15;}}
                                if(players[i].pos.y<10){if((cos(players[i].angle)>-0.174)&&(cos(players[i].angle)<0.174)){angleIsGood=true; players[i].pos.y=prgh-15;}}
                                if(players[i].pos.x>prgw-10){if((sin(players[i].angle)>-0.174)&&(sin(players[i].angle)<0.174)){angleIsGood=true; players[i].pos.x=15;}}
                                if(players[i].pos.y>prgh-10){if((cos(players[i].angle)>-0.174)&&(cos(players[i].angle)<0.174)){angleIsGood=true; players[i].pos.y=15;}}
                                if(angleIsGood){
                                    // Player made the transfer to the other screen: Adjust his real position
                                    players[i].xDouble=players[i].pos.x;
                                    players[i].yDouble=players[i].pos.y;
                                }else{
                                    // He's gotta die
                                    players[i].alive=false;

                                    // Give other players points // TODO: This is supposed to be a function
                                    for(int j=0;j<10;j++){
                                        if(i!=j){
                                            if(players[j].enabled && players[j].alive){
                                                players[j].pts++;
                                            }
                                        }
                                    }
                                }
                            }

                            // Player is inside screen bounds, now check for collisions with other players
                            if(players[i].alive){ // Is he STILL alive?
                                if(evilPixels[players[i].pos.x][players[i].pos.y]){ // o_O player touched something evil and will now die
                                    players[i].alive=false;
                                    // Give other players points // TODO: This is supposed to be a function
                                    for(int j=0;j<10;j++){
                                        if(i!=j){
                                            if(players[j].enabled && players[j].alive){
                                                players[j].pts++;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    stepsSinceLastHole++;

                    // Check for bomb request and execute
                    for(int i=0;i<10;i++){
                        if(players[i].enabled && players[i].alive && players[i].bombAvail && lpressed[i] && rpressed[i] && (lpressed[i]-rpressed[i]>keyPressDelay)){
                            // Bomb key combination detected!
                            players[i].bombAvail=false;

                            // Prepare drawing bomb
                            SDL_Surface *sbomb=NULL;
                            SDL_Rect rbomb;

                            // Randomly determine bomb size (w and h)
                            int nw=(rand()%(bombMaxSize-bombMinSize+1))+bombMinSize;
                            int nh=(rand()%(bombMaxSize-bombMinSize+1))+bombMinSize;

                            // Make sure the bomb is inside the window
                            if(players[i].pos.x-int((double)nw/2)<0){nw=(players[i].pos.x)*2-5;}
                            if(players[i].pos.y-int((double)nh/2)<0){nh=(players[i].pos.y)*2-5;}
                            if(players[i].pos.x+int((double)nw/2)>prgw){nw=(prgw-(players[i].pos.x))*2-5;}
                            if(players[i].pos.y+int((double)nh/2)>prgw){nh=(prgh-(players[i].pos.y))*2-5;}

                            // Build the bomb
                            sbomb=SDL_CreateRGBSurface(SDL_HWSURFACE,nw,nh,32,0,0,0,0);
                            SDL_FillRect(sbomb,NULL,SDL_MapRGB(sbomb->format,players[i].color.r,players[i].color.g,players[i].color.b));
                            rbomb.x=players[i].pos.x-int((double)sbomb->w/2);
                            rbomb.y=players[i].pos.y-int((double)sbomb->h/2);

                            // Draw the bomb
                            SDL_BlitSurface(sbomb,NULL,sScreen,&rbomb);
                            SDL_Flip(sScreen);

                            players[i].alive=false;  // Ugly hack: Temporarly kill the current player to avoid getting points for oneself

                            // Collect 2 points for each killed opponent, kill the victim
                            for(int j=0;j<10;j++){
                                if((players[j].pos.x>rbomb.x)&&(players[j].pos.y>rbomb.y)&&(players[j].pos.x<rbomb.x+sbomb->w)&&(players[j].pos.y<rbomb.y+sbomb->h)){
                                    if(players[j].enabled && players[j].alive){players[i].pts=players[i].pts+2;}
                                    players[j].alive=false;
                                }
                            }
                            // Undo ugly hack from above
                            players[i].alive=true;

                            // Make a dramatic pause (maybe remove this in future version?)
                            SDL_Delay(100);

                            // Area left by bomb shall turn black
                            SDL_FillRect(sbomb,NULL,SDL_MapRGB(sbomb->format,0,0,0));
                            SDL_BlitSurface(sbomb,NULL,sScreen,&rbomb);
                            SDL_Flip(sScreen);
                            for(int n=rbomb.x;n<rbomb.x+sbomb->w;n++){
                                for(int m=rbomb.y;m<rbomb.y+sbomb->h;m++){
                                    evilPixels[n][m]=false;
                                }
                            }
                        }
                    }

                    // Check for teleport request and execute
                    for(int i=0;i<10;i++)
                    {
                        if(players[i].enabled && players[i].alive && lpressed[i] && rpressed[i] && (rpressed[i]-lpressed[i]>keyPressDelay) && players[i].teleportAvail)
                        { // Teleport keycombination detected!
                            players[i].teleportAvail=false;

                            // Determine where the player will arrive and put him there
                            SDL_Rect targetPosition;
                            int rMax=prgw-101,rMin=101;
                            targetPosition.x=(rand()%(rMax-rMin+1))+rMin;
                            rMax=prgh-101;
                            targetPosition.y=(rand()%(rMax-rMin+1))+rMin;
                            players[i].pos=targetPosition;
                            players[i].xDouble=targetPosition.x;
                            players[i].yDouble=targetPosition.y;

                            // Draw a square that will help him survive there
                            SDL_Surface *sSurvivalSquare;
                            targetPosition.x-=50; targetPosition.y-=50;
                            sSurvivalSquare=SDL_CreateRGBSurface(SDL_HWSURFACE,100,100,32,0,0,0,0);
                            SDL_FillRect(sSurvivalSquare,NULL,SDL_MapRGB(sSurvivalSquare->format,0,0,0));
                            SDL_BlitSurface(sSurvivalSquare,NULL,sScreen,&targetPosition);
                            SDL_Flip(sScreen);
                            for(int n=targetPosition.x;n<targetPosition.x+sSurvivalSquare->w;n++)
                            {
                                for(int m=targetPosition.y;m<targetPosition.y+sSurvivalSquare->h;m++)
                                {
                                    evilPixels[n][m]=false;
                                }
                            }
                        }
                    }

                    // Disable powerups that have touched the wall
                    for(int i=0;i<10;i++){
                        if(players[i].enabled && players[i].alive){
                            if((players[i].powerup.pos[0].x<10)||(players[i].powerup.pos[0].y<10)||(players[i].powerup.pos[0].x>prgw-10)||(players[i].powerup.pos[0].y>prgh-10)
                                    || (players[i].powerup.pos[1].x<10)||(players[i].powerup.pos[1].y<10)||(players[i].powerup.pos[1].x>prgw-10)||(players[i].powerup.pos[1].y>prgh-10))
                            {
                                players[i].powerup.running=false;
                            }
                        }
                    }

                    // Draw the new situation
                    for(int i=0;i<10;i++){
                        if(players[i].enabled && players[i].alive){
                            Uint32 currentColor=SDL_MapRGB(sScreen->format,players[i].color.r,players[i].color.g,players[i].color.b);
                            
                            // Draw player's head
                            myDrawFillRect(sScreen, players[i].pos.x-int(slipLevel/2), players[i].pos.y-int(slipLevel/2),slipLevel,slipLevel,currentColor);

                            // Register player's new position to evilPixels
                            evilPixels[players[i].pos.x][players[i].pos.y]=true;
                            if(slipLevel>2){evilPixels[players[i].pos.x-1][players[i].pos.y]=true;}
                            if(slipLevel>2){evilPixels[players[i].pos.x][players[i].pos.y-1]=true;}
                            if(slipLevel>2){evilPixels[players[i].pos.x-1][players[i].pos.y-1]=true;}
                            if(slipLevel>1){evilPixels[players[i].pos.x+1][players[i].pos.y]=true;}
                            if(slipLevel>1){evilPixels[players[i].pos.x][players[i].pos.y+1]=true;}
                            if(slipLevel>1){evilPixels[players[i].pos.x+1][players[i].pos.y+1]=true;}
                            if(slipLevel>2){evilPixels[players[i].pos.x-1][players[i].pos.y+1]=true;}
                            if(slipLevel>2){evilPixels[players[i].pos.x+1][players[i].pos.y-1]=true;}

                            // Draw player's powerup
                            if(players[i].powerup.running && powerupsEnabled){
                                // Draw first arm and register to evilPixels
                                myDrawFillRect(sScreen, players[i].powerup.pos[0].x-2, players[i].powerup.pos[0].y-1,3,2,currentColor);
                                evilPixels[players[i].powerup.pos[0].x-2][players[i].powerup.pos[0].y-1]=true;
                                evilPixels[players[i].powerup.pos[0].x-1][players[i].powerup.pos[0].y-1]=true;
                                evilPixels[players[i].powerup.pos[0].x][players[i].powerup.pos[0].y-1]=true;
                                evilPixels[players[i].powerup.pos[0].x-2][players[i].powerup.pos[0].y]=true;
                                evilPixels[players[i].powerup.pos[0].x-1][players[i].powerup.pos[0].y]=true;
                                evilPixels[players[i].powerup.pos[0].x][players[i].powerup.pos[0].y]=true;
                                
                                // Draw second arm and register to evilPixels
                                myDrawFillRect(sScreen, players[i].powerup.pos[1].x-2, players[i].powerup.pos[1].y-1,2,3,currentColor);
                                evilPixels[players[i].powerup.pos[1].x-2][players[i].powerup.pos[1].y-1]=true;
                                evilPixels[players[i].powerup.pos[1].x-1][players[i].powerup.pos[1].y-1]=true;
                                evilPixels[players[i].powerup.pos[1].x][players[i].powerup.pos[1].y-1]=true;
                                evilPixels[players[i].powerup.pos[1].x-2][players[i].powerup.pos[1].y]=true;
                                evilPixels[players[i].powerup.pos[1].x-1][players[i].powerup.pos[1].y]=true;
                                evilPixels[players[i].powerup.pos[1].x][players[i].powerup.pos[1].y]=true;
                            }
                            
                            // Code for the creation of the holes
                            if(holesEnabled){
                                // If we are currently at a pos where a hole will be drawn, register current pos to holeBackupPos
                                if(stepsSinceLastHole<holeSize){
                                    holeBackupPos[i][stepsSinceLastHole]=players[i].pos;
                                }

                                // If we are 3 steps away from the hole that will be drawn next, draw it
                                if(stepsSinceLastHole>=(holeSize+3)){
                                    for(int j=0;j<holeSize;j++){
                                        myDrawFillRect(sScreen, holeBackupPos[i][j].x-int(slipLevel/2), holeBackupPos[i][j].y-int(slipLevel/2),slipLevel,slipLevel,SDL_MapRGB(sScreen->format,0,0,0));
                                        evilPixels[holeBackupPos[i][j].x][holeBackupPos[i][j].y]=false;
                                        if(slipLevel>2){evilPixels[holeBackupPos[i][j].x-1][holeBackupPos[i][j].y]=false;}
                                        if(slipLevel>2){evilPixels[holeBackupPos[i][j].x][holeBackupPos[i][j].y-1]=false;}
                                        if(slipLevel>2){evilPixels[holeBackupPos[i][j].x-1][holeBackupPos[i][j].y-1]=false;}
                                        if(slipLevel>1){evilPixels[holeBackupPos[i][j].x+1][holeBackupPos[i][j].y]=false;}
                                        if(slipLevel>1){evilPixels[holeBackupPos[i][j].x][holeBackupPos[i][j].y+1]=false;}
                                        if(slipLevel>1){evilPixels[holeBackupPos[i][j].x+1][holeBackupPos[i][j].y+1]=false;}
                                        if(slipLevel>2){evilPixels[holeBackupPos[i][j].x-1][holeBackupPos[i][j].y+1]=false;}
                                        if(slipLevel>2){evilPixels[holeBackupPos[i][j].x+1][holeBackupPos[i][j].y-1]=false;}
                                    }
                                }

                                // If we have gone far enough for a new hole, reset stepsSinceLastHole to initiate the new hole
                                if(stepsSinceLastHole>=(holeSize+holeDelay)){stepsSinceLastHole=0;}
                            }
                        }
                    }
                    SDL_Flip(sScreen);

                    // Is there just one more player left alive? If so, terminate the round
                    int p=0;
                    for(int i=0;i<10;i++){
                        if(players[i].enabled && players[i].alive){p++;}
                    }
                    if(p<=1){roundEnabled=false;}

                }

                // Now, wait a moment to slow the game down
                // TODO: Measure the actually used time in order to guarantee fixed FPS
                SDL_Delay(gameDelay);
            }

            ////////////////////////////////////////////////////  END OF ROUND ///////////////////////////////////////////////////////////
            // Show the points of the round (game is not over yet, there might be another round)
            if(go && !quit){
                // Collect the points into a convenience datastructure
                // TODO: This stuff ist just ugly and confusing... I should create a standard datastructure that has some elegance.
                int nr=0;
                for(int i=0;i<10;i++){
                    if(players[i].enabled){
                        ppts[nr].pts=players[i].pts;
                        ppts[nr].nr=i;
                        ppts[nr].enabled=true;
                        nr++;
                    }
                }
                for(int i=nr;i<10;i++){ppts[i].enabled=false;}

                // Sort
                // TODO: Ugh, this is an ugly bubble sort... I should use something smarter.
                for (int i=0;i<nr;i++){
                    for (int j=0; j<nr-1-i; j++){
                        if ((ppts[j].pts)<(ppts[j+1].pts)){
                            int knr=ppts[j].nr; // swap
                            int kpts=ppts[j].pts;
                            ppts[j].nr=ppts[j+1].nr;
                            ppts[j].pts=ppts[j+1].pts;
                            ppts[j+1].nr=knr;
                            ppts[j+1].pts=kpts;
                        }
                    }
                }

                // Display the points list
                for(int i=0;i<10;i++){
                    if(ppts[i].enabled){
                        char tt[500];
                        sprintf(tt,"Player %i: %i",ppts[i].nr+1,ppts[i].pts);
                        sFont=TTF_RenderText_Blended(fontDigitLarge,tt,players[ppts[i].nr].color);
                        rFont.x=250;
                        rFont.y=((i)*50+10);
                        SDL_BlitSurface(sFont,NULL,sScreen,&rFont);
                        SDL_Flip(sScreen);
                    }
                }

                // Display footer message (if user termination criteria is reached, already show that we're done)
                // TODO: The behaviour described in () above makes not much sence
                if((amountRoundsInGame>0)&&(currentRound+1>=amountRoundsInGame)){sFont=TTF_RenderText_Blended(fontDigitSmall,"Finished! Press Space...",players[0].color);}
                else{sFont=TTF_RenderText_Blended(fontDigitSmall,"Press Space...",players[9].color);}
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
                if(players[i].enabled){amountActivePlayers++; if(players[i].pts>bestScore){bestScore=players[i].pts;}}
            }

            bool gameOver=false;
            if(amountRoundsInGame==-1){if(bestScore>=10*(amountActivePlayers-1)){gameOver=true;}} // termination criteria set to auto
            else if((amountRoundsInGame>0)&&(currentRound>=amountRoundsInGame)){gameOver=true;} // user set termination citeria

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
                        sprintf(tt,"Player %i:%i",ppts[i].nr+1,ppts[i].pts);
                        sFont=TTF_RenderText_Blended(fontDigitLarge,tt,players[ppts[i].nr].color);
                        rFont.x=250;
                        rFont.y=((i)*50+10);
                        SDL_BlitSurface(sFont,NULL,sScreen,&rFont);
                        SDL_Flip(sScreen);
                    }
                }
                sFont=TTF_RenderText_Blended(fontDigitSmall,"Finished! Press Space...",players[0].color);
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
    
    return EXIT_SUCCESS;
}




int options(SDL_Surface *screen)
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
    lsliplevel.label("sliplevel (chance to pass through lines)");
    
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

    if(pauseAllowed){kpause.value(1);}else{kpause.value(0);}
    if(showButtons){kbuttons.value(1);}else{kbuttons.value(0);}
    if(holesEnabled){kholes.value(1);}else{kholes.value(0);}
    if(powerupsEnabled){kpowerups.value(1);}else{kpowerups.value(0);}
    if(handicapsEnabled){khandicaps.value(1);}else{khandicaps.value(0);}
    if(bombsEnabled){kbombs.value(1);}else{kbombs.value(0);}
    if(teleportEnabled){kteleport.value(1);}else{kteleport.value(0);}
    
    kspeed.value(99-gameDelay);
    kspeed.refresh();
    
    ksliplevel.value(3-slipLevel);
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
                }
            }// end Keydown
        }// end switch

        if(kok.isClicked()){ok=true;}
        if(kcancel.isClicked()){cancel=true;}
        if(kreset.isClicked()){reset=true;}

        if(ok){
            pauseAllowed=kpause.value();
            showButtons=kbuttons.value();
            holesEnabled=kholes.value();
            handicapsEnabled=khandicaps.value();
            bombsEnabled=kbombs.value();
            teleportEnabled=kteleport.value();
            powerupsEnabled=kpowerups.value();

            gameDelay=99-kspeed.value();
            slipLevel=3-ksliplevel.value();

            FILE *fSettings;
            fSettings=fopen(fileNameSettings,"w+");
            if(fSettings){
                if(pauseAllowed){fprintf(fSettings,"1\tPausing game: 1=allowed, 0=disallowed\n");}else{fprintf(fSettings,"0\tPausing game: 1=allowed, 0=disallowed\n");}
                fprintf(fSettings,"%i\tSlipLevel (1-3), 1 means that you can pass easily through other lines, 3 = impossible to pass through lines\n",slipLevel);
                if(holesEnabled){fprintf(fSettings,"1\tHoles in the lines: 1=active, 0=inactive\n");}else{fprintf(fSettings,"0\tHoles in the lines: 1=active, 0=inactive\n");}
                fprintf(fSettings,"%i\tNumber of rounds per game. 0 = Never-ending game, -1 = Decide after points\n",amountRoundsInGame);
                fprintf(fSettings,"%i\tGame speed (std=30, more=slower, less=faster)\n",gameDelay);
                if(showButtons){fprintf(fSettings,"1\tShow Buttons during the game: 1=yes 0=no");}else{fprintf(fSettings,"0\tShow Buttons during the game: 1=yes 0=no");}
                if(powerupsEnabled){fprintf(fSettings,"\n1\tUse Powerups      1=yes   0=no\n");}else{fprintf(fSettings,"\n0\tUse Powerups      1=yes   0=no\n");}
                if(handicapsEnabled){fprintf(fSettings,"1\tActivate handicaps       1=yes     0=no");}else{fprintf(fSettings,"0\tActivate handicaps       1=yes     0=no");}
                if(bombsEnabled){fprintf(fSettings,"\n1\tActivate bombs       1=yes    0=no\n");}else{fprintf(fSettings,"\n0\tActivate bombs       1=yes    0=no\n");}
                if(teleportEnabled){fprintf(fSettings,"1\tActivate teleport       1=yes     0=no");}else{fprintf(fSettings,"0\tActivate teleport       1=yes     0=no");}
                fclose(fSettings);
            }else{
                fprintf(stderr,"WARNING: Could not create settings file %s!\n",fileNameSettings);
            }
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


int checkGo(Player* players){
    /// This function ensures that there is a valid set of players for the game. Yes: returns true, No: Suggests a corrected set.

    // If necessary, add one or two players to form a valid set of players
    int n=0;
    for(int i=0;i<10;i++)
    {
        if(players[i].enabled){n++;}
    }
    switch(n){
        case 0:
            players[0].enabled=true;
            players[7].enabled=true;
        break;

        case 1:
            if(players[0].enabled){
                players[7].enabled=true;
            }else{
                players[0].enabled=true;
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
