#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"
#include "platform_sdl.h"

void PrintBuffer(uint8_t *buffer, size_t size, uint16_t offset) {
   for (uint8_t i=0; i<size; i++) {
	if (i % 5 == 0) {
		printf("\n");
		printf("%#04x\t\t", offset + i);
	}
	printf("%02x ", buffer[i]);
     
    }

}


void Load(t_chip8 *chip, char* filename) {

    // printf("Reading ROM from File %s", filename);
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
 	return;
    }

    // get size
    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    uint8_t *buffer = (uint8_t *) malloc(size * sizeof(uint8_t));
    fread(buffer,1, size, file);

    // PrintBuffer(buffer, size, START_ADDRESS);
    memcpy(chip->memory + START_ADDRESS, buffer, size);

  
    free(buffer);
} 

void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

int main(int argc, char **argv) {

	SDLPlatform platform; 
	InitSDLPlatform(&platform);
	SDLPlatform_SetMode(&platform, PLATFORM_STEP_MODE);
	Platform *p = &platform.base;

        
			
	t_chip8 chip = NewChip();
	Load(&chip, argv[1]);
//	Cycle(&chip);
	
	p->init(p, &chip, 64, 32);
	
//	uint32_t buffer[64];
//	for (int x = 0; x < 64; x++) {
//        	uint32_t col = 0;
//        	for (int y = 0; y < 32; y++) {
//		    // alternate pixels
//		    if ((x + y) % 2 == 0) {
//			col |= (1U << (31 - y)); // set the bit for white
//		    }
//        }
//       buffer[x] = col;
//	//memcpy(chip.video, buffer, sizeof(buffer)); 
	p->run(p);	
	
	return 0;
}
