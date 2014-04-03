#ifndef __HW_H__
#define __HW_H__

#include <stdint.h>

void hw_init(void);
int hw_poll(void);

void hw_beginframe(uint8_t **screen, int *pitch);
void hw_endframe();

#endif

