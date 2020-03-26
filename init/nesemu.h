#ifndef NES_EMU_H
#define NES_EMU_H

#include <SDL2/SDL.h>
#include <types.h>
#include <bus/bus.h>
#include <mapper.h>
#include <cpu/cpu.h>

class NesEmu {
private:
	const char *file {};
	unsigned char *rom {};
	const char *current_file {};
	int size {};

	iNesMapper *mapper;
	Bus bus;
	Cpu cpu;
	Ppu ppu;

	SDL_Window *window {};
	SDL_Renderer *render {};
	SDL_Event ev;
	SDL_Texture *tex {};
	SDL_Rect scale {};

public:
	int w { 256 }, h { 240 };
	bool failed {};
	bool fullscreen_enabled {};
	bool realtime {};
	bool broken_emulator_state { };
	bool main_menu_bar {true};

	// Update the frame time from the signal generator

	void clock(unsigned int machine_cycles);
	void emit_interrupts(void);


	// ROM Loader
	bool load_rom(const char *file);

	// Reset the ROM and all the emulator state
	bool reset_state();

	// U/I stuff
	bool draw_menu_bar();
	bool draw_memory_editor();

	// Debug tools for UI
	bool show_memedit {};

	// Updates the display from the video renderer
	void update_video_renderer();

	// Runs the main loop
	void run();
	NesEmu();
	~NesEmu();
};

typedef NesEmu NES_System;

#endif
