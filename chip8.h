#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>

#define VIDEO_LENGTH 64
#define NUM_KEYS 16
typedef struct {
   uint8_t memory[4096];	
   uint16_t stack[16]; // original interpreters had usually a 16 two-byte entries stack.
   uint8_t sp; // stack pointer	
   uint32_t video[VIDEO_LENGTH]; // a 64x32 (=2048)  pixels grid - each entry is a 32 bit integer.
   uint16_t index; // index register - practiclaly, only 12 bits are addressed because of the 4Kb memory limit (2^12=4096)  
   uint16_t pc; // program counter - ibidem 
   uint8_t timer; // delay timer decremented at a rate of 60Hz until it gets to 0
   uint8_t sound_timer; // same as "timer", but it beeps a sound until it gets to 0
   uint8_t registers[16]; //general purpose registers (from 0 to F) called "V0" and "VF" 
   uint16_t keyboard; // flag register - MSB = F, LSB = 0	
   uint8_t flag_draw_video;

} t_chip8; 

// ROM programs starts at 0x200 of memory
// The first 512 bytes (0x000-0x1FF) can have anything
extern const uint16_t START_ADDRESS;
extern const uint16_t FONTS_START_ADDRESS; 


// Fonts are 4x5 pixel sprites (20 bit), but for easier memory access we use 8x5 (40 bit, 5 bytes) entries.
// We have 16 characters (0-9 and A-F), so 16*5 bytes is 80 bytes, so we need 80 bytes to store them. 
// Interpreters used to use the space 0x050 0x09F and so we do.
extern const uint8_t FONTSET_SIZE; 

// 0:
// 1 1 1 1 0 0 0 0 
// 1 0 0 1 0 0 0 0 
// 1 0 0 1 0 0 0 0 
// 1 0 0 1 0 0 0 0 
// 1 1 1 1 0 0 0 0 

extern uint8_t fontset[80]; 

// Forward declarations
struct Platform;
t_chip8 NewChip();
uint16_t Fetch(t_chip8 *chip);
void CleanVideo(t_chip8 *chip);
void DecodeAndExecute(t_chip8 *chip, uint16_t opcode);
void Cycle(t_chip8 *chip);
#endif
