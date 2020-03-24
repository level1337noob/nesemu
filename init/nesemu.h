#ifndef NES_EMU_H
#define NES_EMU_H

#include <SDL2/SDL.h>
#include <types.h>
#include <bus/bus.h>
#include <mapper.h>
#include <cpu/cpu.h>

class NesEmu {
private:
	// Headers main
	const char *file {};
	unsigned char *rom {};
	const char *current_file {};
	int size {};

	// Bus connector etc
	iNesMapper *mapper;
	Bus bus;
	Cpu cpu;
	Ppu ppu;

	SDL_Rect scale {};
	SDL_Window *window {};
	SDL_Renderer *render {};
	SDL_Event ev;
	SDL_Texture *tex {};
	u8 *framebuffer {};
public:
	int w{}, h{};
	bool failed {};
	bool fullscreen_enabled {};
	bool realtime {};
	bool debug {};
	bool main_menu_bar {true};
	float s {1.5f};
	bool reset {};
	NesEmu();
	bool load_rom(const char *file);
	void run();
	void render_display();
	void copy_pixels(u32 *pixels, u32 *display);
	SDL_Texture *pattern_tex0 {};
	SDL_Texture *pattern_tex1 {};

	void render_pattern_texture(SDL_Texture *texture, u8 nn = 0);

	// Cycling + debugging cycles
	void cycle_frame();
	void cycle_line();
	void cycle_pixel();

	bool draw_ui();

	// Updates etc
	void update_audio();
	void update_interrupts();
	void update_timers();
	void update_video();
	~NesEmu();
};

#endif
