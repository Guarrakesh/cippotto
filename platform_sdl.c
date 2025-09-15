#include "platform_sdl.h"
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#define container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type, member) ); })


#define DEFAULT_INSTRUCTIONS_PER_SECOND 700 
#define TIMER_HZ 60
#define MS_PER_INSTRUCTION(IPS) (1000.0/IPS)
#define MS_PER_TIMER 16.66 // 1000 / 60

void Init(Platform *self, t_chip8 *chip8, size_t screen_width, size_t screen_height) {

	SDLPlatform *platform = container_of(self, SDLPlatform, base);
	self->chip8 = chip8;
	self->screen_width = screen_width;
	self->screen_height = screen_height;
	
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Error: %s",SDL_GetError());
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return; 
	}

	

	if (!SDL_CreateWindowAndRenderer("CHIP-8", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &platform->window, &platform->renderer)) {

	
	printf("Error: %s",SDL_GetError());
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
	return;
	}

	platform->texture = SDL_CreateTexture(platform->renderer, 
			    SDL_PIXELFORMAT_ARGB8888,
                            SDL_TEXTUREACCESS_STREAMING,
                            platform->base.screen_width, platform->base.screen_height);
	SDL_SetRenderDrawColor(platform->renderer, 0, 0, 0, 255);
	SDL_SetTextureScaleMode(platform->texture, SDL_SCALEMODE_NEAREST);

	SDL_RenderClear(platform->renderer);
	
	SDL_SetRenderDrawColor(platform->renderer, 255, 255, 255, 255);
	

	SDL_RenderPresent(platform->renderer);


}
	
void Run(Platform *self) {
	SDLPlatform *platform = container_of(self, SDLPlatform, base);
	platform->running = 1;
	platform->paused = 1;	

	Draw(self);	

	uint32_t last_instruction_time = SDL_GetTicks();
	uint32_t last_timer_time = SDL_GetTicks();

	while (platform->running) {
		uint32_t now = SDL_GetTicks();
	
		// Process Inputs
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				platform->running = 0;
			}
			ProcessInputs(self, &event);
		}
		if (platform->paused & 1) {
			continue;
		}		
		// Execute instruction
		while (now - last_instruction_time >= MS_PER_INSTRUCTION(platform->instructions_per_second)) {
			uint16_t instr = Fetch(self->chip8);
			// Very sophisticated debugging technique:
			printf("Current Instruction: %x at position \t %x\n", instr, self->chip8->pc);
			DecodeAndExecute(self->chip8, instr);
			
			last_instruction_time += MS_PER_INSTRUCTION(platform->instructions_per_second);

			if (self->mode == PLATFORM_STEP_MODE) {
//				printf("In step mode: %x\n", self->mode);
				platform->paused = 1;
				break;	
			}
	
		}


		// Update timers at 60Hz
		while (now - last_timer_time >= MS_PER_TIMER) {
			if (self->chip8->timer > 0) self->chip8->timer--;
			if (self->chip8->sound_timer > 0) self->chip8->sound_timer--;
			last_timer_time += MS_PER_TIMER;
		}
		 		

		if (self->chip8->flag_draw_video & 1) {
			printf("Got a draw!!!\n");
			self->chip8->flag_draw_video = 0;
			Draw(self);
		}	
		
		// Since we handle timing manually, we give back control to OS
		// but it is not stricly 16ms anymore
		SDL_Delay(1);
	}
}

	
void ProcessInputs(Platform *self, SDL_Event *event) {
	SDLPlatform *platform = container_of(self, SDLPlatform, base);
	// TODO Implement this
	// TODO: A nice thing would be a "Pause" button (eg pressing the spacebar) that causes the chip to stop at current cycle.

	if (event->type == SDL_EVENT_KEY_DOWN) {
			switch (event->key.key) {
				case SDLK_SPACE:
					platform->paused = !platform->paused; 
					break;
				
				case SDLK_R: 
					platform->paused = 0;
					SDLPlatform_SetMode(platform, PLATFORM_STEP_MODE & 0);
					break;
				
				default:

					for ( uint8_t i=0; i<NUM_KEYS; i++) {
						if (KEYMAP[i] == event->key.key) {
							self->chip8->keyboard |= (1 << i); // set bit i
							break;
						}
					}
			}


			
	} else if (event->type == SDL_EVENT_KEY_UP) {
		for (uint8_t i = 0; i < NUM_KEYS; i++) {
			if (KEYMAP[i] == event->key.key) {
				self->chip8->keyboard &= ~(1 << i); // clear bit 
			}
		}
	}

	
}
void Draw(Platform *self) {
	SDLPlatform *platform = container_of(self, SDLPlatform, base);
	
	uint32_t *buffer = self->chip8->video;

	uint32_t* pixels = (uint32_t*) malloc(sizeof(uint32_t) * (platform->base.screen_width * platform->base.screen_height));
	for (int x=0; x<self->screen_width; x++) {
		uint32_t col = buffer[x];
		for(int y=0; y<self->screen_height; y++) {
			uint32_t bit = (col >> y) & 1;
			pixels[y * self->screen_width + x] = bit ? 0xFFFFFFFF : 0xFF000000;
		}
	}

	SDL_UpdateTexture(platform->texture, NULL, pixels, self->screen_width * sizeof(uint32_t));
	SDL_RenderClear(platform->renderer);
	
	// Rect for the textur
	SDL_FRect rect = { 0, 0, self->screen_width * 5, self->screen_height * 5};
	
	SDL_RenderTexture(platform->renderer, platform->texture, NULL, &rect);
	SDL_RenderPresent(platform->renderer);
	free(pixels);
	
}
void Clean(Platform *self) {
	SDLPlatform *platform = container_of(self, SDLPlatform, base);
	
	SDL_DestroyTexture(platform->texture);
	SDL_DestroyRenderer(platform->renderer);
	SDL_DestroyWindow(platform->window);
	SDL_Quit();

}

void InitSDLPlatform(SDLPlatform *platform) {
	platform->base.run = Run;
	platform->base.clean = Clean;
	platform->base.init = Init;
	platform->base.mode = PLATFORM_STEP_MODE & 0x0;
	platform->running = 0;
	
	SetInstructionsPerSecond(platform, DEFAULT_INSTRUCTIONS_PER_SECOND);
}

void SDLPlatform_SetMode(SDLPlatform *platform, uint8_t mode) {
	platform->base.mode = mode;
}

void SetInstructionsPerSecond(SDLPlatform *platform, int ips) {
	platform->instructions_per_second = ips;
}

void SDLPlatform_PrintDebug(SDLPlatform *platform) {
	printf("\n\n-------- DEBUG INFO --------\n\n");

	printf("\n PC: \t\t %x", platform->base.chip8->pc);
	printf("\n\n-------- END DEBUG INFO --------\n\n");

}
