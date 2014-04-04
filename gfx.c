#include "gfx.h"


#define BIT(x,n) (((x)>>(n))&1)

#define NUM_CHARS 256
uint8_t CHARS[256 * (8*8)];

void decode_chars(uint8_t *rom)
{
    int t,x,y;

    uint8_t *curchar = CHARS;
    for (t=0;t<NUM_CHARS;t++)
    {
        for (y=0;y<8;y++)
        {
            for (x=1;x>=0;x--)
            {
                uint8_t r = rom[x*8+y];
                curchar[3] = BIT(r,0) | (BIT(r,4)<<1);
                curchar[2] = BIT(r,1) | (BIT(r,5)<<1);
                curchar[1] = BIT(r,2) | (BIT(r,6)<<1);
                curchar[0] = BIT(r,3) | (BIT(r,7)<<1);
                curchar+=4;
            }
        }

        rom += 16;
    }
}

void draw_charmap(uint8_t *screen, int pitch)
{
    uint8_t *vram = VIDEO_RAM;
    int i,j;
    for (j = 0; j < 28; ++j)
    {
        uint8_t *row = screen;
        for (i = 0; i < 32; ++i)
        {
            uint8_t *tile = &CHARS[*vram++ * 64];

            int x, y;
            for (y = 0; y < 8; y++)
            {
                uint8_t *dst = row + y*pitch;
                for (x = 0; x < 8; x++)
                {
                    uint8_t rgb;
                    rgb = *tile * 64;
                    *dst++ = rgb;
                    *dst++ = rgb;
                    *dst++ = rgb;
                    *dst++ = 0;
                    ++tile;
                }
            }

            row += 8*4;
        }

        screen += pitch*8;
    }
}

void gfx_draw(uint8_t *screen, int pitch)
{
    draw_charmap(screen, pitch);
}
