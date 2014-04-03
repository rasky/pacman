#include "hw.h"
#include <SDL/SDL.h>

int main(int argc, char *argv[])
{
    hw_init();

    while (hw_poll())
    {
    }

    return 0;
}
