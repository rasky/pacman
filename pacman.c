#include "hw.h"
#include <SDL/SDL.h>

int main(int argc, char *argv[])
{
    hw_init();

    while (hw_poll())
    {
        uint8_t *screen;
        int pitch;

        hw_beginframe(&screen, &pitch);
        hw_endframe();
    }

    return 0;
}
