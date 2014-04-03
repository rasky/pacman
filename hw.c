#include "hw.h"
#include <SDL.h>
#include <SDL_framerate.h>

static SDL_Surface *screen;
static SDL_Surface *frame;
static FPSmanager fps;

void hw_init(void)
{
    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 )
    {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    screen=SDL_SetVideoMode(256, 224,32,SDL_DOUBLEBUF);
    if (screen == NULL)
    {
       printf("Unable to set video mode: %s\n", SDL_GetError());
       exit(1);
    }

    frame = SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 224, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x0);
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
    SDL_UnlockSurface(frame);
    SDL_BlitSurface(frame, NULL, screen, NULL);
    SDL_Flip(screen);
    SDL_framerateDelay(&fps);
}

