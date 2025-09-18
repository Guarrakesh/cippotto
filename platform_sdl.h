#include "platform.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

#define WINDOW_WIDTH  1274
#define WINDOW_HEIGHT 720

// Mimic the classic Keyboard layout
// todo: Configurable?
// Maps the Key on the keyboard to the corresponding CHIP8 Key: see comment below for mapping visualisation.
const static uint8_t KEYMAP[NUM_KEYS] = {
	SDLK_X, SDLK_1, SDLK_2, SDLK_3, // 0, 1, 2, 3
	SDLK_Q, SDLK_W, SDLK_E, SDLK_A, // 4, 5, 6, 7,
	SDLK_S, SDLK_D, SDLK_Z, SDLK_C, // 8, 9, A, B,
	SDLK_4, SDLK_R, SDLK_F, SDLK_V, // C, D, E, F
};
// Look at your these keys on your keyboard: this is your CHIP-8 keypad 
// 1, 2, 3, 4    			1, 2, 3, C
// Q, W, E, R	  maps to 	4, 5, 6, D
// A, S, D, F	   ---> 	7, 8, 9, E,
// Z, X, C, V				A, 0, B, F
typedef struct SDLPlatform {
	struct Platform base;
	uint8_t running;
	uint8_t paused; 
	int instructions_per_second;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;	

	
} SDLPlatform;


// Generic Platform functions
void Init(Platform *self, t_chip8 *, size_t, size_t);
void Clean(Platform *self);
void Run(Platform *self);
// SDL Specific Functions
void ProcessInputs(Platform *self, SDL_Event *);
void Draw(Platform *self);
void SetInstructionsPerSecond(SDLPlatform *, int);
void InitSDLPlatform(SDLPlatform *);


void SDLPlatform_SetMode(SDLPlatform *, uint8_t);
void SDLPlatform_PrintDebug(SDLPlatform *);
#endif
