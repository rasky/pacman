#include "hw.h"
#include <SDL.h>
#include <SDL_framerate.h>

static SDL_Surface *screen;
static SDL_Surface *frame;
static FPSmanager fps;

#define SPLIT 20

void hw_init(void)
{
    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 )
    {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    screen=SDL_SetVideoMode(256+224+SPLIT, 256, 32,SDL_DOUBLEBUF);
    if (screen == NULL)
    {
       printf("Unable to set video mode: %s\n", SDL_GetError());
       exit(1);
    }

    frame = SDL_CreateRGBSurface(SDL_SWSURFACE, 32*8, 28*8, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x0);
    SDL_initFramerate(&fps);
    SDL_setFramerate(&fps, 60);
}

int hw_poll(void)
{
    SDL_Event event;

    while ( SDL_PollEvent(&event) )
    {
        if (event.type == SDL_QUIT)
            return 0;

        if ( event.type == SDL_KEYDOWN )
        {
            if ( event.key.keysym.sym == SDLK_ESCAPE )
                return 0;
        }
    }

    return 1;
}

void hw_beginframe(uint8_t **screen, int *pitch)
{
    SDL_LockSurface(frame);
    *screen = frame->pixels;
    *pitch = frame->pitch;
}

void hw_endframe(void)
{

    SDL_LockSurface(screen);

    uint8_t *spixels = screen->pixels;
    uint8_t *fpixels = frame->pixels;
    int x,y;
    for (y=0;y<224;y++)
    {
        uint8_t *framerow = fpixels + (223-y)*frame->pitch;
        for (x=0;x<256;x++)
        {
            int sx = 256+SPLIT+y;
            int sy = x;

            spixels[sy*screen->pitch + sx*4] = framerow[x*4];
            spixels[sy*screen->pitch + sx*4+1] = framerow[x*4+1];
            spixels[sy*screen->pitch + sx*4+2] = framerow[x*4+2];
            spixels[sy*screen->pitch + sx*4+3] = framerow[x*4+3];

            spixels[y*screen->pitch + x*4] = framerow[x*4];
            spixels[y*screen->pitch + x*4+1] = framerow[x*4+1];
            spixels[y*screen->pitch + x*4+2] = framerow[x*4+2];
            spixels[y*screen->pitch + x*4+3] = framerow[x*4+3];
        }
    }

    SDL_UnlockSurface(frame);
    SDL_UnlockSurface(screen);

    SDL_Flip(screen);
    SDL_framerateDelay(&fps);
}

