#ifndef PLATFORM_H 
#define PLATFORM_H
#include <stdlib.h>
#include "chip8.h"

#define PLATFORM_STEP_MODE  0x00000001

typedef struct Platform {
	size_t screen_width;
	size_t screen_height;
	uint8_t mode;
	t_chip8 *chip8; 
	void (*init) (struct Platform*, t_chip8 *, size_t, size_t);
	void (*run)  (struct Platform*);
	void (*clean) (struct Platform*);
	
} Platform;

#endif



