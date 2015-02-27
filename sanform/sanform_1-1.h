/**
Sanform 1-1     MODIFIED FOR POWERLINES 2-3
Copyright 2010-2015 Sandro Kalbermatter
Lib dependencies: SDL, SDL_Image, SDL_Text
File dependencies: sanform_1-1.cpp , sanform_1-1.h , biondi.ttf

Note: I programmed this when I was learning C++, so things are very simplistic and not always elegant.
Still, this set of functions (it's supposed to be something like a library) is very convenient for the options dialogue.
**/

#ifndef DEF_sanform
#define DEF_sanform

#include <iostream>
#include <cstring>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

                                                    
enum{                                               
     SF_BUTTON,                                     //  Button, a clickable field
     SF_LABEL,                                      //  Shows a text label
     SF_CHECKBOX,                                   //  Checkbox (yes-no-field)
     SF_PROGRESS,                                   //  Shows a progress value in %
     SF_TRACKBAR                                    //  Slider to set a value between min and max
};

enum{
     SFF_NOANIM=1,                                  //  This avoids the animation at mouse hover
     SFF_UNI=2                                      //  This avoids that the form is drawn with color transition
};

enum{
     SFF_CENTER=1,                                  //  Horizontal orientation
     SFF_RIGHT=2,
     
     SFF_DOWN=4,                                    //  Vertical orientation
     SFF_MIDDLE=8,
     
     SFF_NOTRANSPARENCE=16                          //  Disables transparence of surface
};



class sanform
{
    public:
        sanform(int type);                          //  Constructor
        sanform();                                  //  Minimalistic constructor
        ~sanform();                                 //  Destructor
        sanform operator=(const sanform form);      //  Operator =
        
        //  Variable setters
        void type(int type);                        //  Set type
        void rect(int x,int y,int w,int h);         //  Set position and size
        void x(int x);                              //  Set x coordinate only
        void y(int y);                              //  Set y coordinate only
        void w(int w);                              //  Set width only
        void h(int h);                              //  Set height only
        void label(std::string label);              //  Set label
        void text(std::string text);                //  Set text
        void value(int value);                      //  Set value
        void max(int max);                          //  Set m_max
        void surf(SDL_Surface *surf,int nr);        //  Set surf[nr]
        void color(SDL_Color color);                //  Set color using an SDL_Color
        void color(int r,int g,int b);              //  Set color using three integers
        void r(int r);                              //  Set red only
        void g(int g);                              //  Set green only
        void b(int b);                              //  Set blue only
        void transparence(int transparence);        //  Set transparency (255=opaque, 0=invisible)
        void enabled(bool enabled);                 //  Enable / Disable form
        void locked(bool locked);                   //  Lock / Unlock form
        void clicked(bool clicked);                 //  Simulate a click on this form
        void touched(bool touched);                 //  Simulate a mouse hover on this form
        void flags(int flags);                      //  Set flags of form
        void sflags(Uint32 sflags,int nr);          //  Set flags of a surface
        void font(TTF_Font *font);                  //  Set the font
        void focus(bool focus);                     //  Set m_focus
        
        //  Variable getters
        int type();                                 //  Returns form type
        SDL_Rect rect();                            //  Return position and size as SDL_Rect
        int x();                                    //  Return x coordinate
        int y();                                    //  Return y coordinate
        int w();                                    //  Return width
        int h();                                    //  Return height
        std::string label();                        //  Return label
        std::string text();                         //  Return text
        int value();                                //  Return value
        int max();                                  //  Return m_max
        SDL_Surface *surf(int nr);                  //  Return surf[nr]
        SDL_Color color();                          //  Return color as SDL_Color
        int r();                                    //  Return red only
        int g();                                    //  Return green only
        int b();                                    //  Return blue only
        int transparence();                         //  Return transparency (255=opaque, 0=invisible)
        bool enabled();                             //  Return wheather the form is enabled
        bool locked();                              //  Return wheather the form is locked
        bool clicked();                             //  Return m_clicked
        bool touched();                             //  Return m_touched
        int flags();                                //  Return Flags
        Uint32 sflags(int nr);                      //  Returns the flags of a surface
        TTF_Font *font();                           //  Returns the font
        bool focus();                               //  Returns m_focus
        
        bool mouseWentIn();                         //  Return m_mouseWentIn
        bool mouseWentOut();                        //  Return m_mouseWentOut
        
        
        //  Events
        bool isClicked();                           //  Return wheater the form was clicked and sets m_clicked to false
        bool isTouched();                           //  Return wheater the form was clicked and sets m_touched to false
        
        void linkEvent(SDL_Event event);            //  Make the form react to an SDL_Event
        
        //  Anzeige
        int refresh();                              //  Refreshe surf
        void blit(SDL_Surface *screen);             //  Blit surf onto screen
        int ablit(SDL_Surface *screen);             //  Refresh surf und blit it onto screen
        
                                                    
                                                    
                                                    
    private: // Note: These vars have the m_ prefix to show that they are private
        int m_type;                                 //  Type of the form, set to enum SF_...
        std::string m_label;                        //  Fix text displayed by the form
        std::string m_text;                         //  Text editable by the user
        int m_value;                                //  Integer value, e.g. percentage, 1 or 0, ...
        int m_max;                                  //  Maximal value that can be reached by m_value
        SDL_Rect m_rect;                            //  Position and size of the form
        SDL_Surface *m_surf[15];                    //  Surface of the form, used for blitting    m_surf[0] is the base surface, m_surf[14] will be positionned according to m_value
        SDL_Color m_color;                          //  Farbe der Form
        int m_transparence;                         //  Transparency of the form (255=opaque, 0=invisible)
        bool m_enabled;                             //  Is the form enabled? If no, it will be ignored.
        bool m_locked;                              //  Set this to true to keep the user from altering the form / editing its content
        bool m_clicked;                             //  Becomes true when the form is clicked. Reset using clicked()
        bool m_touched;                             //  As m_clicked, but for mouseover (when the mouse just touches the form, no clicking)
        int m_flags;                                //  Flag container
        Uint32 m_sflags[15];                        //  Surface flag container
        TTF_Font *m_font;                           //  Font to be used
        bool m_focus;                               //  true at LinkEvent mouseWentIn, false at LinkEvent mouseWentOut
        
        //  Variables that only have getters, no setters
        bool m_mouseWentIn;                         //  Did the mouse just enter the form?
        bool m_mouseWentOut;                        //  Did the mouse just leave the form?
        
        //  Variables with no getters / setters at all
        SDL_Rect m_oldMousePos;                     //  Used for position changes of the mouse. linkEvent() uses this at the very end.
};                                                  
                                                    



#endif
