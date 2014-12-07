/**
Sanform 1-1     MODIFIED FOR POWERLINES 2-3
Copyright 2010-2014 Sandro Kalbermatter
Lib dependencies: SDL, SDL_Image, SDL_Text
File dependencies: sanform_1-1.cpp , sanform_1-1.h , biondi.ttf

Note: I programmed this when I was learning C++, so things are very simplistic and not always elegant.
Still, this set of functions (it's supposed to be something like a library) is very convenient for the options dialogue.
**/

#include "sanform_1-1.h"

using namespace std;

sanform::sanform()
{
    m_type = -1;
    m_text="";
    m_label="# Unknown Form #";
    m_value=0;
    m_max=100;
    m_rect.x=0; m_rect.y=0; m_rect.w=150; m_rect.h=50;
    for(int i=0;i<15;i++){m_surf[i]=NULL; m_sflags[i]=0;}
    m_color.r=50; m_color.g=50; m_color.b=50;
    m_transparence=255;
    m_enabled=true;
    m_locked=false;
    m_clicked=false;
    m_touched=false;
    m_flags=0;
    m_font=TTF_OpenFont("fonts/biondi.ttf",18); if(m_font==NULL){fprintf(stderr,"Font biondi.ttf could not be loaded!"); exit(EXIT_FAILURE);}
    m_focus=false;
    
    m_mouseWentOut=false;
    m_mouseWentOut=false;
    
    m_oldMousePos.x=m_oldMousePos.y=m_oldMousePos.w=m_oldMousePos.h=0;
}

sanform::sanform(int typ)
{
    m_type = typ;
    m_text="";
    switch(typ)
    {
        case SF_BUTTON:
            m_label="Button";
        break;

        case SF_LABEL:
            m_label="Label";
        break;

        case SF_CHECKBOX:
            m_label="Checkbox";
        break;

        case SF_PROGRESS:
            m_label="Progress Bar";
        break;

        case SF_TRACKBAR:
            m_label="Track Bar";
        break;

        default:
            m_label="# Unknown Form #";
        break;
    }
    m_value=0;
    m_max=100;
    m_rect.x=0; m_rect.y=0; m_rect.w=150; m_rect.h=50;
    for(int i=0;i<15;i++){m_surf[i]=NULL; m_sflags[i]=0;}
    m_color.r=50; m_color.g=50; m_color.b=50;
    m_transparence=255;
    m_enabled=true;
    m_locked=false;
    m_clicked=false;
    m_touched=false;
    m_flags=0;
    m_font=TTF_OpenFont("fonts/biondi.ttf",18); if(m_font==NULL){fprintf(stderr,"Font biondi.ttf could not be loaded!"); exit(EXIT_FAILURE);}
    m_focus=false;
    
    m_mouseWentOut=false;
    m_mouseWentOut=false;
    
    m_oldMousePos.x=m_oldMousePos.y=m_oldMousePos.w=m_oldMousePos.h=0;
}

sanform::~sanform()
{
    for(int i=0;i<15;i++)
    {
        if(m_surf[i]!=NULL){SDL_FreeSurface(m_surf[i]);}
    }
}

sanform sanform::operator=(const sanform form)
{
    m_type=form.m_type;
    m_text=form.m_text;
    m_label=form.m_label;
    m_value=form.m_value;
    m_max=form.m_max;
    m_rect=form.m_rect;
    for(int i=0;i<15;i++){m_surf[i]=form.m_surf[i];}
    m_transparence=form.m_transparence;
    m_enabled=form.m_enabled;
    m_locked=form.m_locked;
    m_clicked=form.m_clicked;
    m_touched=form.m_touched;
    m_flags=form.m_flags;
    m_font=form.m_font;
    m_focus=form.m_focus;
    
    m_mouseWentOut=form.m_mouseWentOut;
    m_mouseWentOut=form.m_mouseWentOut;
    
    m_oldMousePos=form.m_oldMousePos;

    return *this;
}



//  Setters


void sanform::type(int typ)
{
    m_type=typ;
}

void sanform::rect(int x,int y,int w,int h)
{
    m_rect.x=x;
    m_rect.y=y;
    m_rect.w=w;
    m_rect.h=h;
}

void sanform::x(int x)
{
    m_rect.x=x;
}

void sanform::y(int y)
{
    m_rect.y=y;
}

void sanform::w(int w)
{
    m_rect.w=w;
}

void sanform::h(int h)
{
    m_rect.h=h;
}

void sanform::label(string label)
{
    m_label=label;
}

void sanform::text(string text)
{
    m_text=text;
}

void sanform::value(int value)
{
    m_value=value;
}

void sanform::max(int max)
{
    m_max=max;
}


void sanform::surf(SDL_Surface *surf, int nr)
{
    m_surf[nr]=surf;
}

void sanform::color(SDL_Color color)
{
    m_color=color;
}

void sanform::color(int r,int g,int b)
{
    m_color.r=r;
    m_color.g=g;
    m_color.b=b;
}

void sanform::r(int r)
{
    m_color.r=r;
}

void sanform::g(int g)
{
    m_color.g=g;
}

void sanform::b(int b)
{
    m_color.b=b;
}

void sanform::transparence(int transparence)
{
    m_transparence=transparence;
}

void sanform::enabled(bool aktiv)
{
    m_enabled=aktiv;
}

void sanform::locked(bool locked)
{
    m_locked=locked;
}

void sanform::clicked(bool clicked)
{
    m_clicked=clicked;
}

void sanform::touched(bool touched)
{
    m_touched=touched;
}

void sanform::flags(int flags)
{
    m_flags=flags;
}

void sanform::sflags(Uint32 sflags,int nr)
{
    m_sflags[nr]=sflags;
}

void sanform::font(TTF_Font *font)
{
    m_font=font;
}

void sanform::focus(bool focus)
{
    m_focus=focus;
}


//  Getter functions

int sanform::type()
{
    return m_type;
}

SDL_Rect sanform::rect()
{
    return m_rect;
}

int sanform::x()
{
    return m_rect.x;
}

int sanform::y()
{
    return m_rect.y;
}

int sanform::w()
{
    return m_rect.w;
}

int sanform::h()
{
    return m_rect.h;
}

string sanform::label()
{
    return m_label;
}

string sanform::text()
{
    return m_text;
}

int sanform::value()
{
    return m_value;
}

int sanform::max()
{
    return m_max;
}


SDL_Surface *sanform::surf(int nr)
{
    return m_surf[nr];
}

SDL_Color sanform::color()
{
    return m_color;
}

int sanform::r()
{
    return m_color.r;
}

int sanform::g()
{
    return m_color.g;
}

int sanform::b()
{
    return m_color.b;
}

int sanform::transparence()
{
    return m_transparence;
}

bool sanform::enabled()
{
    return m_enabled;
}

bool sanform::locked()
{
    return m_locked;
}

bool sanform::clicked()
{
    return m_clicked;
}

bool sanform::touched()
{
    return m_touched;
}

int sanform::flags()
{
    return m_flags;
}

Uint32 sanform::sflags(int nr)
{
    return m_sflags[nr];
}

TTF_Font *sanform::font()
{
    return m_font;
}

bool sanform::focus()
{
    return m_focus;
}


bool sanform::mouseWentIn()
{
    return m_mouseWentOut;
}

bool sanform::mouseWentOut()
{
    return m_mouseWentOut;
}



//  Event handlers
bool sanform::isClicked()
{
    if(m_clicked){m_clicked=false; return true;}else{return false;}
}

bool sanform::isTouched()
{
    if(m_touched){m_touched=false; return true;}else{return false;}
}

void sanform::linkEvent(SDL_Event event)
{
    if(m_enabled&&!m_locked)
    {

        if(event.type==SDL_MOUSEMOTION) // The mouse was moved
        {
            if((event.motion.x>m_rect.x)&&(event.motion.y>m_rect.y)&&(event.motion.x<(m_surf[0]->w+m_rect.x))&&(event.motion.y<(m_surf[0]->h+m_rect.y)))
            {
                m_touched=true;
                if(!((m_oldMousePos.x>m_rect.x)&&(m_oldMousePos.y>m_rect.y)&&(m_oldMousePos.x<(m_surf[0]->w+m_rect.x))&&(m_oldMousePos.y<(m_surf[0]->h+m_rect.y))))
                {m_mouseWentOut=true; m_focus=true;}else{m_mouseWentOut=false;}
            }else{
                if(((m_oldMousePos.x>m_rect.x)&&(m_oldMousePos.y>m_rect.y)&&(m_oldMousePos.x<(m_surf[0]->w+m_rect.x))&&(m_oldMousePos.y<(m_surf[0]->h+m_rect.y))))
                {m_mouseWentOut=true; m_focus=false;}else{m_mouseWentOut=false;}
            }


            SDL_PumpEvents();
            if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(1))  //  A mouse button is pressed
            {
                if((event.motion.x>m_rect.x)&&(event.motion.y>m_rect.y)&&(event.motion.x<(m_surf[0]->w+m_rect.x))&&(event.motion.y<(m_surf[0]->h+m_rect.y)))
                {
                    if(m_type==SF_TRACKBAR){m_value=int((double)(event.motion.x-((double)m_surf[0]->h/4)-m_rect.x+(double)m_surf[0]->w/m_max/2)/(m_surf[0]->w-(double)m_surf[0]->h/2)*m_max);}  //  Achtung: Siehe unten
                }
            }
            m_oldMousePos.x=event.motion.x;
            m_oldMousePos.y=event.motion.y;
        }else if(event.type==SDL_MOUSEBUTTONDOWN)  //  A click (press + release) has been made
        {
            if((event.button.x>m_rect.x)&&(event.button.y>m_rect.y)&&(event.button.x<(m_surf[0]->w+m_rect.x))&&(event.button.y<(m_surf[0]->h+m_rect.y)))
            {
                m_clicked=true;
                if(m_type==SF_CHECKBOX){m_value=!m_value;}
                if(m_type==SF_TRACKBAR){m_value=int((double)(event.button.x-((double)m_surf[0]->h/4)-m_rect.x+(double)m_surf[0]->w/m_max/2)/(m_surf[0]->w-(double)m_surf[0]->h/2)*m_max);}  //  Achtung: Siehe oben

            }
            m_oldMousePos.x=event.button.x;
            m_oldMousePos.y=event.button.y;
        }
    }
}



//  Display

int sanform::refresh()
{
    if(m_enabled)
    {
        for(int i=0;i<15;i++){SDL_FreeSurface(m_surf[i]); m_surf[i]=NULL;}
        switch(m_type)
        {
            case SF_BUTTON:
            {
                m_surf[0]=SDL_CreateRGBSurface(SDL_HWSURFACE,m_rect.w, m_rect.h, 32, 0, 0, 0, 0);
                SDL_FillRect(m_surf[0],NULL,SDL_MapRGB(m_surf[0]->format,m_color.r,m_color.g,m_color.b));
                SDL_Color tempcolor;
                if((m_color.r+m_color.g+m_color.b)/3>80)
                {
                    tempcolor.r=tempcolor.g=tempcolor.g=0;
                }else{
                    tempcolor.r=tempcolor.g=tempcolor.g=255;
                }
                if(m_font==NULL){fprintf(stderr,"Error: font biondi is not loaded!"); exit(EXIT_SUCCESS);}
                m_surf[1]=TTF_RenderText_Blended(m_font,m_label.c_str(),tempcolor);
                m_sflags[1]=(SFF_CENTER|SFF_MIDDLE|SFF_NOTRANSPARENCE);

                // Markers at mouseover
                if(!(m_flags & SFF_NOANIM))
                {
                    if(m_focus)
                    {
                        m_surf[2]=SDL_CreateRGBSurface(SDL_HWSURFACE,5,m_rect.h,32,0,0,0,0);
                        SDL_FillRect(m_surf[2],NULL,SDL_MapRGB(m_surf[0]->format,0,255,0));

                        m_surf[3]=SDL_CreateRGBSurface(SDL_HWSURFACE,5,m_rect.h,32,0,0,0,0);
                        SDL_FillRect(m_surf[3],NULL,SDL_MapRGB(m_surf[0]->format,0,0,255));
                        m_sflags[3]=SFF_RIGHT;
                    }
                }
                break;
            }

            case SF_LABEL:
            {
                m_surf[0]=TTF_RenderText_Blended(m_font,m_label.c_str(),m_color);
                m_sflags[0]=(SFF_NOTRANSPARENCE);
                m_flags|=SFF_UNI;
                break;
            }

            case SF_CHECKBOX:
            {
                m_surf[0]=SDL_CreateRGBSurface(SDL_HWSURFACE,m_rect.w,m_rect.h,32,0,0,0,0);
                SDL_FillRect(m_surf[0],NULL,SDL_MapRGB(m_surf[0]->format,m_color.r,m_color.g,m_color.b));
                
                if(m_value)
                {
                    m_surf[1]=SDL_CreateRGBSurface(SDL_HWSURFACE,int(m_rect.w/3*2),int(m_rect.h/3*2),32,0,0,0,0);
                    SDL_FillRect(m_surf[1],NULL,SDL_MapRGB(m_surf[1]->format,0,255,0));
                    m_sflags[1]=SFF_CENTER|SFF_MIDDLE;
                }
                
                // Marker at mouseover
                if(!(m_flags & SFF_NOANIM))
                {
                    if(m_focus)
                    {
                        m_surf[2]=SDL_CreateRGBSurface(SDL_HWSURFACE,int(m_rect.w/5*2),int(m_rect.h/5*2),32,0,0,0,0);
                        SDL_FillRect(m_surf[2],NULL,SDL_MapRGB(m_surf[2]->format,100,100,100));
                        m_sflags[2]=SFF_CENTER|SFF_MIDDLE;
                    }
                }
                break;
            }

            case SF_PROGRESS:
            {
                if(m_value<0){m_value=0;}
                if(m_value>100){m_value=100;}
                
                m_surf[0]=SDL_CreateRGBSurface(SDL_HWSURFACE,m_rect.w,m_rect.h,32,0,0,0,0);
                SDL_FillRect(m_surf[0],NULL,SDL_MapRGB(m_surf[0]->format,m_color.r,m_color.g,m_color.b));
                
                m_surf[1]=SDL_CreateRGBSurface(SDL_HWSURFACE,int(m_rect.w*m_value/100),int(m_rect.h/5)+1,32,0,0,0,0);
                SDL_FillRect(m_surf[1],NULL,SDL_MapRGB(m_surf[1]->format,0,255,0));
                m_sflags[1]=SFF_MIDDLE;
            }
            break;

            case SF_TRACKBAR:
            {
                if(m_value<0){m_value=0;}
                if(m_value>m_max){m_value=m_max;}
                
                m_surf[0]=SDL_CreateRGBSurface(SDL_HWSURFACE,m_rect.w,m_rect.h,32,0,0,0,0);
                SDL_FillRect(m_surf[0],NULL,SDL_MapRGB(m_surf[0]->format,m_color.r,m_color.g,m_color.b));
                
                m_surf[14]=SDL_CreateRGBSurface(SDL_HWSURFACE,(m_rect.h/2),int(m_rect.h/2),32,0,0,0,0);
                SDL_FillRect(m_surf[14],NULL,SDL_MapRGB(m_surf[14]->format,0,200,0));
                m_sflags[14]=SFF_MIDDLE;
                
                // Markers at mouseover
                if(!(m_flags & SFF_NOANIM))
                {
                    if(m_focus)
                    {
                        SDL_FillRect(m_surf[14],NULL,SDL_MapRGB(m_surf[14]->format,200,200,0));
                    }
                }
            }
            break;

            default:
                fprintf(stderr,"Refresh: One or more forms have an undefined type!\n");
        }

    }// end if enabled
    return 1;
}

void sanform::blit(SDL_Surface *screen)
{
    if(m_enabled)
    {
        SDL_Rect temprect;

        for(int i=0;i<15;i++)
        {if(m_surf[i]!=NULL)
            {
                temprect=m_rect;

                //  Sflags: Horizontal
                if(m_sflags[i] & SFF_CENTER)         //  Center surface i in base surface horizontally
                {
                    temprect.x=int(((m_surf[0]->w)/2)-((m_surf[i]->w)/2)+m_rect.x);
                }

                if(m_sflags[i] & SFF_RIGHT)          //  Right align surface i in base surface
                {
                    temprect.x=(m_rect.x+m_surf[0]->w-m_surf[i]->w);
                }


                // Sflags: Vertikal
                if(m_sflags[i] & SFF_MIDDLE)         //  surface i in base surface vertically
                {
                    temprect.y=int(((m_surf[0]->h)/2)-((m_surf[i]->h)/2)+m_rect.y);
                }

                if(m_sflags[i] & SFF_DOWN)          //  bottom align surface i in base surface
                {
                    temprect.y=(m_rect.y+m_surf[0]->h-m_surf[i]->h);
                }


                // Transparency
                if(!(m_sflags[i] & SFF_NOTRANSPARENCE)){SDL_SetAlpha(m_surf[i],SDL_SRCALPHA,m_transparence);}

                // If this is surface at level 14, Set its position according to m_value
                if(i==14){temprect.x+=int((double)m_value/m_max*(m_surf[0]->w-m_surf[0]->h/2));}

                // Blit to screen
                SDL_BlitSurface(m_surf[i],NULL,screen,&temprect);

                //  Color transition effect
                const int effect=100;  // 0=nothing    255=max
                if(!i&&!(m_flags & SFF_UNI))
                {
                    SDL_Surface *tempsurf=NULL;
                    SDL_Color tempcolor;
                    int plus=0;

                    for(int n=0;n<m_surf[0]->w;n++)
                    {
                        plus=int(((double)n/m_surf[0]->w)*effect)-int(effect/2);
                        if(m_color.r+plus>255){plus=255-m_color.r;}   if(m_color.r+plus<0){plus=0-m_color.r;}
                        tempcolor.r=m_color.r+plus;
                        plus=int(((double)n/m_surf[0]->w)*effect)-int(effect/2);
                        if(m_color.g+plus>255){plus=255-m_color.g;}   if(m_color.g+plus<0){plus=0-m_color.g;}
                        tempcolor.g=m_color.g+plus;
                        plus=int(((double)n/m_surf[0]->w)*effect)-int(effect/2);
                        if(m_color.b+plus>255){plus=255-m_color.b;}   if(m_color.b+plus<0){plus=0-m_color.b;}
                        tempcolor.b=m_color.b+plus;

                        SDL_FreeSurface(tempsurf); tempsurf=NULL;
                        tempsurf=SDL_CreateRGBSurface(SDL_HWSURFACE,1,m_surf[0]->h, 32, 0, 0, 0, 0);
                        SDL_FillRect(tempsurf,NULL,SDL_MapRGB(m_surf[0]->format,tempcolor.r,tempcolor.g,tempcolor.b));

                        temprect.x=n+m_rect.x;

                        SDL_BlitSurface(tempsurf,NULL,screen,&temprect);
                    }
                }// end transition effect
            } // end if
        }// end for
    } // end if enabled
}


int sanform::ablit(SDL_Surface *screen)
{
    if(m_enabled)
    {
        if(!refresh()){return 0;}
        blit(screen);
        SDL_Flip(screen);
    }
    return 1;
}
