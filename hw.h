#ifndef __HW_H__
#define __HW_H__

#include <stdint.h>

extern uint8_t *keystate;

void hw_init(void);
int hw_poll(void);

void hw_beginframe(uint8_t **screen, int *pitch);
void hw_endframe();


#define HW_AUDIO_FREQ         44100
#define HW_AUDIO_NUMSAMPLES   (HW_AUDIO_FREQ / 60)

void hw_beginaudio(int16_t **buf);
void hw_endaudio(void);


#endif

