#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <init/nesemu.h>
#include <noobinterface.h>

Log cpu_log {}, ppu_log {}, general_log {};

using namespace std;
NesEmu::NesEmu() { }
NesEmu::~NesEmu() { free(rom), rom = NULL; }

// main internals of rendering the display
void NesEmu::update_video_renderer(void)
{
	SDL_RenderClear(render);
	SDL_RenderCopy(render, tex, NULL, &scale);
	NoobInterface::RenderAll();
}

bool NesEmu::reset_state()
{
	memset((void *) &bus, 0, sizeof bus);
	memset((void *) &cpu, 0, sizeof ppu);
	memset((void *) &ppu, 0, sizeof ppu);
	mapper = (iNesMapper *) rom;
	bus.plugin_mapper(mapper);
	bus.set_memory(rom);
	if (!bus.select_cartridge(mapper)) {
		broken_emulator_state = true;
		return false;
	}
	cpu.power_on(0xC000);
	cpu.attach_bus(&bus);
	cpu.attach_ppu(&ppu);
	ppu.init_palette_table();
	ppu.attach_bus(&bus);
	ppu.power_up();
	bus.attach_ppu(&ppu);
	return true;
}


void NesEmu::run(void)
{
	reset_state();
	window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
											  w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	SDL_GetWindowSize(window, &scale.w, &scale.h);

	// Initialise ImGui and my interface
	ImGui::CreateContext();
	NoobInterface::Init(window);
	NoobInterface::SetRenderingMode(render, _SDL_RENDERING_MODE);
	NoobInterface::SetDarkTheme();
	while (NoobInterface::isOpened()) {
		while (SDL_PollEvent(&ev)) {
			NoobInterface::ProcessEvents(&ev);
			switch (ev.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				switch (ev.type == SDL_KEYDOWN ? ev.key.keysym.scancode & 0xFF : 0xDEAD) {
				case SDL_SCANCODE_J:
					main_menu_bar = !main_menu_bar;
					break;
				}
				break;
			case SDL_WINDOWEVENT:
				switch (ev.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					SDL_GetWindowSize(window, &scale.w, &scale.h);
					break;
				}
				break;
			}
		}

		// Clock the NES emulator in realtime
		if (!broken_emulator_state) {
			clock(0);
		}

		ImGui::NewFrame();
		draw_menu_bar();
		draw_memory_editor();
		ImGui::EndFrame();

		update_video_renderer();
		SDL_Delay(10);
	}


	// Destroy contents
	bus.clear_memory();
	NoobInterface::ClearDarkTheme();
	NoobInterface::Destroy();
	ImGui::DestroyContext();
	SDL_DestroyTexture(tex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(render);
	window = NULL;
	render = NULL;
}
