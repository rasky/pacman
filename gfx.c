#include "gfx.h"
#include <string.h>

#define BIT(x,n) (((x)>>(n))&1)

#define NUM_CHARS 256
uint8_t CHARS[256 * (8*8)];
uint8_t COLORS[32][4];
uint8_t PALETTES[64][4];

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

void decode_colors(uint8_t *rom)
{
    int i;
    /* create a lookup table for the palette */
    for (i = 0; i < 32; i++)
    {
        int bit0, bit1, bit2;
        int r, g, b;

        /* red component */
        r = (rom[i] & 7);
        r = (r<<5) | (r << 2) | (r >> 1);

        /* green component */
        g = ((rom[i] >> 3) & 7);
        g = (g<<5) | (g << 2) | (g >> 1);

        /* blue component */
        b = ((rom[i] >> 6) & 3);
        b = (b<<6) | (b << 4) | (b << 2) | b;

        COLORS[i][0] = r;
        COLORS[i][1] = g;
        COLORS[i][2] = b;
        COLORS[i][3] = 0;
    }
}

void decode_palettes(uint8_t *rom)
{
    memcpy(PALETTES, rom, 64*4);
}

void draw_charmap(uint8_t *screen, int pitch)
{
    uint8_t *vram = VIDEO_RAM;
    uint8_t *cram = COLOR_RAM;
    int i,j;
    for (j = 0; j < 28; ++j)
    {
        uint8_t *row = screen;
        for (i = 0; i < 32; ++i)
        {
            uint8_t *tile = &CHARS[*vram++ * 64];
            uint8_t *palette = PALETTES[((*cram++) & 0x3F)];

            int x, y;
            for (y = 0; y < 8; y++)
            {
                uint8_t *dst = row + y*pitch;
                for (x = 0; x < 8; x++)
                {
                    uint8_t *rgb;
                    rgb = COLORS[palette[*tile&3] & 31];
                    *dst++ = rgb[0];
                    *dst++ = rgb[1];
                    *dst++ = rgb[2];
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
