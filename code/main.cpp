#include <Windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>
#include <cstring>
#include <cmath>
#include "mesh.h"

#include <iostream>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef int32_t  b32;

typedef float    r32;
typedef double   r64;

#define SCREENW 700
#define SCREENH 700

struct Input
{
    struct Mouse
    {
        int x,y;
        int dx,dy;
        int lmb;
        int rmb;
        int mmb;
    } mouse,prev_mouse;
    
    unsigned char keyboard[323];
    unsigned char prev_keyboard[323];

    void update()
    {    
        memcpy(prev_keyboard,keyboard,323);
        memcpy(keyboard,SDL_GetKeyboardState(NULL),323);

        int buttons;
        memcpy(&prev_mouse,&mouse,sizeof(mouse));
        buttons=SDL_GetMouseState(&mouse.x,&mouse.y);

        mouse.dx=mouse.x-prev_mouse.x;
        mouse.dy=mouse.y-prev_mouse.y;

        mouse.lmb = (buttons & SDL_BUTTON(1)) ? 1 : 0;
        mouse.mmb = (buttons & SDL_BUTTON(2)) ? 1 : 0;
        mouse.rmb = (buttons & SDL_BUTTON(3)) ? 1 : 0;
    }
};


//globals begin
Input input;
bool quit = false;
Maillage M;
int drag_index = -1;
int hover_index = -1;
vector<float> colors;

//globals end

void draw_maillage()
{
    Sommet s0,s1,s2;
    for(int i=0;i<M.triangles.size();i++)
    {
        s0 = M.get_sommet(M.get_halfedge(M.triangles[i].e0).s0);
        s1 = M.get_sommet(M.get_halfedge(M.triangles[i].e1).s0);
        s2 = M.get_sommet(M.get_halfedge(M.triangles[i].e2).s0);
//        if(i!=0)continue;
        glBegin(GL_TRIANGLES);
        
        glColor3f(colors[3*i],colors[3*i+1],colors[3*i+2]);
        glVertex2f(s0.x,s0.y);
        glVertex2f(s1.x,s1.y);
        glVertex2f(s2.x,s2.y);
        glEnd();
        
    }
}

void draw_sommets()
{
    glColor3ub(0,0,128);
    glPointSize(5);
    glBegin(GL_POINTS);
    for(int i=0;i<M.sommets.size();i++)
    {
        glVertex2f(M.sommets[i].x,
                   M.sommets[i].y);
                   
    }
    glEnd();
    glColor3ub(255,255,255);
}

int point_in_rect(float x,float y, float rx,float ry, float w,float h)
{
    if(x>rx+w || x<rx || y>ry+h || y<ry)
        return 0;
    return 1;
}


int main(int agrc, char** argv)
{
    SDL_Window * window;
    SDL_GLContext glcontext;
    SDL_Event ev;

    SDL_Init(SDL_INIT_VIDEO);
    window=SDL_CreateWindow("TRIANGULATION", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            SCREENW, SCREENH, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
    glcontext = SDL_GL_CreateContext(window);

    glViewport(0,0,SCREENW,SCREENH);

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0,SCREENH,SCREENW,0);

#if 1
    srand(time(0));
    M.generate_sommets(85);
#else
    //*
    M.add_sommet(make_sommet(10,10));
    //M.sommets.push_back(new_sommet(410,10));
    M.add_sommet(make_sommet(80,60));
    M.add_sommet(make_sommet(400,10));
    M.add_sommet(make_sommet(10,400));
    M.add_sommet(make_sommet(210,210));
    //M.add_sommet(make_sommet(80,560));
    
#endif    

    M.triangulated = 0;
    
    int NN = 4;
    while(!quit)
    {
        while(SDL_PollEvent(&ev))
        {
            if(ev.type==SDL_QUIT)
                quit=1;
        }

        input.update();

        if(input.keyboard[SDL_SCANCODE_RETURN] && !input.prev_keyboard[SDL_SCANCODE_RETURN])
        {
            M.delaunay();
        }
        if(input.keyboard[SDL_SCANCODE_ESCAPE] && !input.prev_keyboard[SDL_SCANCODE_ESCAPE])
        {            
            M.triangulation_naive();

            colors.clear();
            for(int i=0;i<3*M.triangles.size();i++)
                colors.push_back(frandom());

        }

        hover_index=-1;
        for(int i=0;i<M.sommets.size();i++)
        {
            float w = 5;
            if(point_in_rect(input.mouse.x,input.mouse.y,
                             M.sommets[i].x - w, M.sommets[i].y - w,
                             2*w,2*w))
            {
                hover_index = i;
            }
        }
        
        if(input.mouse.lmb && !input.prev_mouse.lmb)
        {
            drag_index = hover_index;
        }

        if(!input.mouse.lmb && input.prev_mouse.lmb)
        {
            drag_index = -1;
        }

        if(drag_index !=-1)
        {
            M.sommets[drag_index].x = input.mouse.x;
            M.sommets[drag_index].y = input.mouse.y;
            M.triangulated = 0;
        }
        
        /*      if(input.mouse.lmb && !input.prev_mouse.lmb)
        {   
            M.arretes.clear();
            M.sommets.clear();
            M.triangles.clear();

            int seed = time(0);
            cout<<seed<<endl;
            srand(seed);
            
            M.generate_sommets(NN);
            M.triangulation_naive();
            NN++;
        }
        if(input.mouse.rmb && !input.prev_mouse.rmb)
        {
            M.arretes.clear();
            M.sommets.clear();
            M.triangles.clear();

            int seed = time(0);
            cout<<seed<<endl;
            srand(seed);
            
            M.generate_sommets(NN);
            M.triangulation_naive();
            NN--;
        }//*/
        
/*
        M.sommets[M.sommets.size()-1].x = input.mouse.x;
        M.sommets[M.sommets.size()-1].y = input.mouse.y;
        M.arretes.clear();
        M.triangles.clear();
        M.triangulation_naive();
//*/
        
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if(M.triangulated)
            draw_maillage();
        draw_sommets();
        
        if(hover_index!=-1)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColor4ub(255,0,0,128);
            float sx = M.sommets[hover_index].x;
            float sy = M.sommets[hover_index].y;
            glBegin(GL_QUADS);
            glVertex2f(sx-5,sy-5);
            glVertex2f(sx+5,sy-5);
            glVertex2f(sx+5,sy+5);
            glVertex2f(sx-5,sy+5);
            glEnd();
            glColor4ub(255,255,255,255);
            glDisable(GL_BLEND);
        }

        
        SDL_GL_SwapWindow(window);
//        SDL_Delay(5);
    }

    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
