#include "gfx.h"
#include <string.h>

#define BIT(x,n) (((x)>>(n))&1)

#define NUM_CHARS    256
#define NUM_SPRITES  64
uint8_t CHARS[256][(8*8)];
uint8_t COLORS[32][4];
uint8_t SPRITES[NUM_SPRITES][16*16];
uint8_t PALETTES[64][4];

void decode_chars(uint8_t *rom)
{
    int t,x,y;

    uint8_t *curchar = (uint8_t*)CHARS;
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

void decode_sprites(uint8_t *rom)
{
    int i,x,y;
    static const int xtab[16] = {8,16,24,0};
    static const int ytab[16] = {0,1,2,3,4,5,6,7,32,33,34,35,36,37,38,39};

    uint8_t *cursprite = (uint8_t*)SPRITES;
    for (i=0;i<NUM_SPRITES;i++)
    {
        for (y=0;y<16;y++)
        {
            for (x=0;x<4;x++)
            {
                uint8_t r = rom[xtab[x]+ytab[y]];
                cursprite[3] = BIT(r,0) | (BIT(r,4)<<1);
                cursprite[2] = BIT(r,1) | (BIT(r,5)<<1);
                cursprite[1] = BIT(r,2) | (BIT(r,6)<<1);
                cursprite[0] = BIT(r,3) | (BIT(r,7)<<1);
                cursprite+=4;
            }
        }

        rom += 64;
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

static void draw_char(uint8_t *screen, int pitch, int numchar, int numpalette)
{
    uint8_t *tile = CHARS[numchar];
    uint8_t *palette = PALETTES[numpalette & 0x3F];

    int x, y;
    for (y = 0; y < 8; y++)
    {
        uint8_t *dst = screen + y*pitch;
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
}

void draw_charmap(uint8_t *screen, int pitch)
{
    enum {
        OFFSET_SPECIAL_LINE1 = 2,
        OFFSET_SPECIAL_LINE2 = OFFSET_SPECIAL_LINE1+32,
        OFFSET_SPECIAL_LINE3 = 64 + 32*28 + 2,
        OFFSET_SPECIAL_LINE4 = OFFSET_SPECIAL_LINE3+32
    };

    uint8_t *vram = VIDEO_RAM+64;
    uint8_t *cram = COLOR_RAM+64;
    int i,j;

    for (j = 0; j < 28; ++j)
    {
        uint8_t *row = screen + j*8*pitch;

        draw_char(row, pitch, VIDEO_RAM[OFFSET_SPECIAL_LINE3+j], COLOR_RAM[OFFSET_SPECIAL_LINE3+j]);
        row += 8*4;
        draw_char(row, pitch, VIDEO_RAM[OFFSET_SPECIAL_LINE4+j], COLOR_RAM[OFFSET_SPECIAL_LINE4+j]);
        row += 8*4;

        for (i = 0; i < 32; ++i)
        {
            draw_char(row, pitch, *vram++, *cram++);
            row += 8*4;
        }

        draw_char(row, pitch, VIDEO_RAM[OFFSET_SPECIAL_LINE1+j], COLOR_RAM[OFFSET_SPECIAL_LINE1+j]);
        row += 8*4;
        draw_char(row, pitch, VIDEO_RAM[OFFSET_SPECIAL_LINE2+j], COLOR_RAM[OFFSET_SPECIAL_LINE2+j]);
    }
}

void draw_sprites(uint8_t *screen, int pitch)
{
    uint8_t *SPRITE_RAM = &RAM[0x7F0];
    int i;

    for (i=0;i<8;i++)
    {
        int index = SPRITE_RAM[i*2] >> 2;
        int flip = SPRITE_RAM[i*2] & 3;
        int palette = SPRITE_RAM[i*2+1] & 63;
        int sx = 272 - SPRITEPOS_RAM[i*2+1];
        int sy = SPRITEPOS_RAM[i*2] - 31;

        if (sx < 0 || sy < 0)
            continue;

        int x,y;

        #define OFFPIXEL(sx,sy)  ((sx)<=0 || (sy)<=0 || (sx)>=288 || (sy)>=224)

        uint8_t *row = screen + sy*pitch + sx*4;
        uint8_t *src = SPRITES[index];

        for (y=0;y<16;y++)
        {
            uint8_t *dst = row;
            for (x=0;x<16;x++)
            {
                uint8_t *rgb = COLORS[PALETTES[palette][*src]];
                if (!OFFPIXEL(sx+x, sy+y))
                {
                    *dst++=*rgb++;
                    *dst++=*rgb++;
                    *dst++=*rgb++;
                    *dst++=0;
                }
                src++;
            }
            row += pitch;
        }
    }
}


void gfx_draw(uint8_t *screen, int pitch)
{
    draw_charmap(screen, pitch);
    draw_sprites(screen, pitch);
}
