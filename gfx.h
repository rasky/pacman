#include <stdint.h>

extern uint8_t VIDEO_RAM[0x400];

void gfx_draw(uint8_t *screen, int pitch);

void decode_chars(uint8_t *rom);
