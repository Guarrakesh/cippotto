#include "chip8.h"
#include <stdlib.h>
#include <stdio.h>
#include "log.h"
const uint16_t START_ADDRESS = 0x200;
const uint16_t FONTS_START_ADDRESS = 0x050;
const uint8_t FONTSET_SIZE = 80;

uint8_t fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

t_chip8 NewChip() {
    t_chip8 chip;
    chip.flag_draw_video = 0;
	chip.pc = 0x200;
	chip.keyboard = 0;	
	for (int i=0; i<VIDEO_LENGTH; i++) chip.video[i]=0;
	for (int i=0; i<4096; i++) chip.memory[i] = 0;
	for (int i=0; i<16;i++) chip.stack[i] = 0;
	for (int i=0; i<16;i++) chip.registers[i] = 0;

	chip.sp = 0; // Convention adopted: sp points to the first free location on the stack. So last element pushed
	// is (sp-1)
	chip.index = 0;
	chip.timer = 0;
	chip.sound_timer = 0;
	chip.flag_draw_video = 0;	
	for (int i=0; i<FONTSET_SIZE; i++) {
		chip.memory[FONTS_START_ADDRESS + i] = fontset[i]; 
	}	
	return chip;
}

uint16_t Fetch(t_chip8 *chip) {
   uint16_t instruction = chip->memory[chip->pc] << 8u | chip->memory[chip->pc + 1];
   chip->pc += 2;
  return instruction;
} 

void CleanVideo(t_chip8 *chip) {
	for (uint32_t i = 0; i < VIDEO_LENGTH; i++) {
		chip->video[i] = 0;
	}
	chip->flag_draw_video = 1;
}

void DecodeAndExecute(t_chip8 *chip8, uint16_t instruction) {
	uint16_t opcode = instruction & 0xF000;
	uint16_t X = (instruction & 0x0F00) >> 8;
	uint16_t Y = (instruction & 0x00F0) >> 4;
	uint16_t N = instruction & 0x000F;
	uint16_t NN = instruction & 0x00FF;
	uint16_t NNN = instruction & 0x0FFF;

	switch (opcode) {
	    case 0x0000:
			switch (NN) {
				case 0x00E0:
					CleanVideo(chip8);	
				break;
				case 0x00EE:
					// Return from Subroutine 
					// Decrements stack pointer and return the PC (last element pushed)
					chip8->pc = chip8->stack[--chip8->sp];
				break;
            }
			break;
		case 0x1000: {
			// jump 
			chip8->pc = NNN;
			break;	
		}
		case 0x2000: {
			// CALL - push current PC to stack and jump to NNN
			// The SP is incremented after
			chip8->stack[chip8->sp++] = chip8->pc;
			chip8->pc = NNN;
			break;
		}
		case 0x3000:
		case 0x4000: {
			// 0x3XNN - Skip 1 instruction (increment PC by 2) if V[X] == NN
			// 0x4XNN - Skip 1 instruction (incremenet PC by 2) if V[X] != NN
			uint8_t vx = chip8->registers[X];
			if ((opcode == 0x3000 && vx == NN) || (opcode == 0x4000 && vx != NN)) {
				chip8->pc += 2;
			}
			break;
		}
		case 0x5000:
		case 0x9000: {
			// 0x5XY0 - skip if V[X] == V[y] 
			// 0x9XY0 - skip if V[X] != V[y]
			uint8_t vx = chip8->registers[X];
			uint8_t vy = chip8->registers[Y];
			if ((opcode == 0x5000 && vx == vy) || (opcode == 0x9000 && vx != vy)) {
				chip8->pc += 2;
			}
			break;
		}

		case 0x6000: {
			// Set VX register
			chip8->registers[X] = NN;
			break;
		}
		case 0x7000: {
			// ADD

			uint8_t vx = X;
			uint8_t val = NN;
			chip8->registers[vx] += val;
			break;
		}
		// Logical and Arithmetic instructions
		case 0x8000: {
			 
			uint8_t vx = chip8->registers[X];
			uint8_t vy = chip8->registers[Y];
			switch (N) { // read last nibble
				case 0x0000: chip8->registers[X] = vy; break;
				case 0x0001: chip8->registers[X] |= vy; break;
				case 0x0002: chip8->registers[X] &= vy; break;
				case 0x0003: chip8->registers[X] ^= vy; break;
				case 0x0004: {
					uint16_t sum = vx + vy;
 					// set carry flag to 1 if sum overflows
					chip8->registers[X] = sum;
					chip8->registers[0xF] = sum > 255 ? 1 : 0;
					break;
				}
				case 0x0005:
				case 0x0007: {

					chip8->registers[X] = (N == 0x0005 
										? chip8->registers[X] - chip8->registers[Y]
										: chip8->registers[Y] - chip8->registers[X]
					);
					// Set carry (borrow) to 1 if minuend > subtrahend
					if ((N == 0x0005 && vx >=  vy) || (N == 0x0007 && vy >= vx)) {
						chip8->registers[0xF] = 1; // no borrow
					} else {
						chip8->registers[0xF] = 0; // borrow
					}

					break;
				}
				case 0x0006:
				case 0x000E: { 
					// Shift
					// Set VX to VY (this may be configurable as newer CHIP8 And SuperCHIP don't do it
					vx = vy;
					// 0x8XY6 Shift VX >> 1
					// 0x8XYE Shift VX << 1
					// Then, set V[F]=the bit that has been shifted out.
					uint8_t new_vx = N == 0x0006 ? vx >> 1 : vx << 1;
					chip8->registers[X] = new_vx;	
					chip8->registers[0xF] = N == 0x0006 
						? vx & 1  // set 1 if the bit shifted out is 1
						: (vx & 0x80) ? 1 : 0;
					break;
				}
				
			}

		}	
		case 0xA000: {
			chip8->index = NNN;
			break;
		}
		case 0xB000: {
			// Again this can be configurable
			// Earlier chips jump to NNN + V[0] (eg B220 -> Jump to 220 + V[0])
			// Latest ones treat it as XNN -> Jump to XNN + V[X] (eg B220 -> Jump XNN + V[2])
			
			// Let's make the first one for now
			chip8->pc = NNN + chip8->registers[0];
			break;
			 
		}
		case 0xC000: {
			// Random number ANDed with NN and put into VX
						 
			chip8->registers[X] = rand() & NN;
			break;
		}
		case 0xD000: {
			// Draw DXYN
			// 1. fetch VX and VY pointers
			uint8_t vx = X;
			uint8_t vy = Y;
			uint8_t height = N; 
			// 2. Module the X and Y coordinates.
			uint8_t x = chip8->registers[vx] % (VIDEO_LENGTH - 1);
			uint8_t y = chip8->registers[vy] % (32 - 1);
			// 3. Set VF (V[15]) to 0
			chip8->registers[0xF] = 0;

			for (uint8_t i=0; i<height; i++) {
				// 4. get the sprite from index register
				uint8_t sprite_byte = chip8->memory[chip8->index + i];
				for (uint8_t bit = 0; bit < 8; bit++) {
					uint8_t pixel_x = x + bit;
					uint8_t pixel_y = y + i;
					// check bounds
					if (pixel_x >= 64 || pixel_y >= 32) continue;
					// get the bit from the sprite, from msb to lsb
					uint8_t sprite_pixel = (sprite_byte >> (7-bit)) & 1;
					// current bit from the video sprite
					uint8_t current_pixel = (chip8->video[pixel_x] >> pixel_y) & 1;

					uint8_t new_pixel = sprite_pixel ^ current_pixel;
					if (current_pixel && !new_pixel) chip8->registers[0xF] = 1; // collision!
					
					if (new_pixel) {
						chip8->video[pixel_x] |= (1U << pixel_y); // set bit 
					} else {
						chip8->video[pixel_x] &= ~(1U << pixel_y); // clear bit
					}
				}
									
			}	
			chip8->flag_draw_video = 1;	
			break;
		}

		case 0xE000: {
			uint8_t key = chip8->registers[X];
			// ex VX = 5 (maps to 'W' key in the classic KEYMAP)
			// chip8->keyboard has bit 5 set 0xb0000000000100000) (W pressed)
			// chip8->keyboard >> 5 & 1 returns 1
			uint8_t key_pressed = (chip8->keyboard >> key) & 1; 

			switch (NN) {
				case 0x009E: 
					if (key_pressed) {
						chip8->pc += 2;
					}
					
					break;
				case 0x00A1: 
					if (!key_pressed) {
						chip8->pc += 2;
					}
					break;

			}  
			break;
		}

		case 0xF000: {
			switch (NN) {
				case 0x0007: chip8->registers[X] = chip8->timer; break;
				case 0x0015: chip8->timer = chip8->registers[X]; break;
				case 0x0018: chip8->sound_timer = chip8->registers[X]; break;
			}
			if (NN == 0x001E) {
				// Amiga CHIP8 set the V[F]=1 if overflown and Spacefight 2091 relied on this.
				// Earlier CHIP8 did not do it. Let's do like Amiga here (again, may be Configurable TODO:) 
				uint16_t sum = chip8->index + chip8->registers[X]; 
				chip8->registers[0xF] = sum > 0xFFF ? 1 : 0;
				chip8->index = sum & 0x0FFF;
				
			} 
		
			//FX0A - Get key 	
			if (NN == 0x000A) {
				// Stops until a key is pressed // TODO:
				if (chip8->keyboard == 0) {
					chip8->pc -= 2; // repeat until key is pressed
				}
			}

			if (NN == 0x0029) {
				// Load font at X
				LOG_VERBOSE("Loading font sprite at %x\n", FONTS_START_ADDRESS + (chip8->registers[X] * 5)); 
				chip8->index = FONTS_START_ADDRESS + (chip8->registers[X] * 5); // fonts are 5 bytes;	
				
			}

			if (NN == 0x0033) {
				// Binary-coded decimal conversion
				// Convert X to decimal and place first digit in I (msb), second digit in I+1, third digit (lsb) in I+2
				// X can be between [0,255]
				uint16_t vx = chip8->registers[X];
				
				chip8->memory[chip8->index] = vx / 100; // hundreds digit
				chip8->memory[chip8->index+1] = (vx / 10) % 10; // tens digit
				chip8->memory[chip8->index+2] = (vx %  100) % 10; // ones digit
			}
			if (NN == 0x0055 || NN == 0x0065) {
				// Another ambiguous instruction among different versions
				// See: https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#fx55-and-fx65-store-and-load-memory
				uint16_t index = chip8->index; //earlier version modified index in-place, here we use a temp variable
				// TODO: make this configurable to be able to  run older ROMs
				for (int i=0; i<=X; i++) {
					if (NN == 0x0055) {
						chip8->memory[index+i] = chip8->registers[i];
					} else {
						chip8->registers[i] = chip8->memory[index+i];
					}
				} 
			}	
			break;

		}

		default:
			printf("\n\t %x - UNREGULAR INSTRUCTION!!!\n", instruction);
	} 
}

