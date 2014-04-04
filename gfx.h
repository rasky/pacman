#include <stdint.h>

extern uint8_t VIDEO_RAM[0x400];
extern uint8_t COLOR_RAM[0x400];

void gfx_draw(uint8_t *screen, int pitch);

void decode_chars(uint8_t *rom);
void decode_colors(uint8_t *rom);
void decode_palettes(uint8_t *rom);

