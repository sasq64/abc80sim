#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void sound_init();
void sound_destroy();
void sound_event(uint8_t data);

#endif // SOUND_H
