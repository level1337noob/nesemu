#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <init/nesemu.h>
#include <noobinterface.h>
#include <memedit.h>
#include <log.h>

using namespace std;
#if (defined __linux__) || (defined __APPLE__)
	#include <dirent.h>
#endif

NesEmu::NesEmu()
{

}

NesEmu::~NesEmu()
{
	free(rom);
	rom = NULL;
}


bool NesEmu::load_rom(const char *file)
{
	//file = "tests/ppu/full_palette/full_palette.nes";
	//file = "roms/donkeykong.nes";
	FILE *t = fopen(file, "rb");
	if (!t) {
		nes_printf("Invalid file\n");
		return false;
	}
	
	fseek(t, 0, SEEK_END);
	size = ftell(t);
	rewind(t);

	if (size > 0x100000) {
		nes_printf("Size is greater than 1MB\n");
		fclose(t);
		return false;
	}

	rom = (unsigned char *) malloc(size + 0x20);
	memset(rom, 0, size + 0x20);
	fread(rom, size, 1, t);
	current_file = file;
error:
	fclose(t);
	return true;
}


void NesEmu::cycle_frame(void)
{
	do {
		if (!cpu.cycle())
			failed = true;
	} while (!ppu.cframe);
}


void NesEmu::cycle_line(void)
{
	do {
		if (!cpu.cycle())
			failed = true;
	} while (!ppu.cline);
}


void NesEmu::cycle_pixel(void)
{
	if (!cpu.cycle())
		failed = true;
}


void NesEmu::copy_pixels(u32 *pixels, u32 *display)
{
	memcpy((u8 *) pixels, (u8 *) display, 256 * 240 * 4);
}


void NesEmu::render_display() {
	void *pixels;
	int pitch;
	if (SDL_LockTexture(tex, NULL, &pixels, &pitch)) {
		nes_printf("Couldn't lock texture %s\n", SDL_GetError());
		failed = true;
		return;
	}

	copy_pixels((u32 *) pixels, ppu.display);

	SDL_UnlockTexture(tex);
}


void NesEmu::update_video(void)
{
	if (!realtime) {
		if (ppu.ccyc) {
			ppu.ccyc = false;
			goto g;
		} else if (ppu.cline) {
			ppu.cline = false;
			goto g;
		}
	}

	if (ppu.cframe) {
	g:
		ppu.cframe = false;
		render_display();
	}

	SDL_RenderClear(render);
	SDL_RenderCopy(render, tex, NULL, &scale);
	NoobInterface::RenderAll();
}

Log cpu_log {}, ppu_log {}, general_log {};

class fs {
private:
public:
	std::vector<std::string> path {"noobdir/"};
#if (defined __linux__) || (defined __APPLE__)
	DIR *dp;
	struct dirent *d;
#endif


	std::string get_path() {
		std::string result;
		for (auto& x : path)
			result += x;
		return result;
	}

	const char *root_directory() { return path.at(0).c_str(); }
	void prev(void) { if (path.size() == 1) { return; } path.pop_back(); }
	void walk(std::string path) { this->path.push_back(path + "/"); }
	bool set_path(std::string path) {
		const char *s = path.c_str();
		if (!memcmp(s, root_directory(), strlen(root_directory()))) {
			s = path.c_str() + strlen(root_directory());
			path = ".";
			path += s;
		}

#if (defined __linux__) || (defined __APPLE__)
		dp = opendir(path.c_str());
		if (!dp) { return false; }
#endif
		return true;
	}

	const char *read_current_file() {
#if (defined __linux__) || (defined __APPLE__)
		d = readdir(dp);
		if (!d) return NULL;
		if (d->d_name[0] == '.') {
			return NULL;
		}
		return d->d_name;
#else
		return NULL;
#endif
	}
};


void NesEmu::render_pattern_texture(SDL_Texture *tex, u8 nn)
{
	void *pixels;
	int pitch;
	u8 x;
	u8 y;
}


bool NesEmu::draw_ui(void)
{
	static MemoryEditor memedit_prg, memedit_ram, memedit_chr1, memedit_chr2;
	static bool show_log {}, show_memedit {};
	ImGui::NewFrame();

	bool rv {};
	if (main_menu_bar) {
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::Button("Open ROM")) {
					ImGui::OpenPopup("Open File");
				}
				
				if (ImGui::BeginPopupModal("Open File", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
					ImGui::Text("Open the NES binary file (mapper 000 supported for now)");

					static char buffer[1024] = "...";
					ImGui::Separator();
					ImGui::InputText("File Input", buffer, IM_ARRAYSIZE(buffer));
					ImGui::Separator();
					
					if (ImGui::Button("Select File")) {
						if (!load_rom(buffer)) {
							ImGui::OpenPopup("Check File Integrity");
						} else {
							reset = 1;
							rv = true;
						}
					}
					
					if (ImGui::BeginPopupModal("Check File Integrity", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
						ImGui::Text("Invalid ROM-File Selected");
						if (ImGui::Button("Ok")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					if (ImGui::Button("Cancel")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				
				if (ImGui::BeginMenu("Save State")) {
					if (ImGui::MenuItem("State 1")) {

					}
					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Load State")) {
					if (ImGui::MenuItem("State 1")) {

					}
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Exit", "ALT+F4")) {
					rv = true;
				}

				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("Debug")) {

				ImGui::Text("General Info");
				ImGui::Checkbox("Realtime Mode", &realtime);
				if (ImGui::Button("Reset Emulator")) {
					reset = 1;
					rv = true;
				}
				
				static char buf_scale[4] {"1.5"};
				static char x_scale[4] {"0"};
				static char y_scale[4] {"0"};
				
				ImGui::PushItemWidth(30.0f);
				ImGui::InputText("X Position", x_scale, 4);
				ImGui::SameLine();

				ImGui::InputText("Y Position", y_scale, 4);
				ImGui::SameLine();
				ImGui::PopItemWidth();

				if (ImGui::Button("Resize")) {

				}


				ImGui::PushItemWidth(30.0f);
				ImGui::InputText("Texture Scale", buf_scale, 4);
				ImGui::SameLine();
				ImGui::PopItemWidth();

				if (ImGui::Button("Rescale")) {
					float i = strtof(buf_scale, 0);
					if (i >= 10.0f) {
						printf("Invalid scale\n");
					} else {
						s = i;
						scale.w = 256 * s;
						scale.h = 240 * s;
						printf("Set the scale to %.2f\n", i);
					}
				}

				ImGui::Text("CPU/PPU Debuggers");
				ImGui::Checkbox("Debugging Tools", &debug);
				ImGui::Checkbox("Show Log", &show_log);
				ImGui::Checkbox("Show Memory Editor", &show_memedit);

				ImGui::Text("NTSC TV Steps");
				if (ImGui::Button("Step current")) { cycle_pixel(); }
				if (ImGui::Button("Step scanline")) { cycle_line(); }
				if (ImGui::Button("Step frame")) { cycle_frame(); }
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("About")) {
				ImGui::Text("nesemu - NES Emulator/Debugger");
				ImGui::Separator();
				ImGui::Text("File Info: %s", current_file);
				ImGui::Separator();
				ImGui::Text("2020 level1337noob @ Github");
				ImGui::EndMenu();
			}

			ImGui::Text("TexW: %.0f", scale.w * s);
			ImGui::Text("TexH: %.0f", scale.h * s);
			ImGui::Text("Frames: %d", ppu.frames);
			
			if (ImGui::Button("Fullscreen")) {
				fullscreen_enabled = !fullscreen_enabled;
				if (fullscreen_enabled) {
					SDL_SetWindowFullscreen(window, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
				} else {
					SDL_SetWindowFullscreen(window, SDL_WINDOW_SHOWN);
				}
				SDL_GetWindowSize(window, &scale.w, &scale.h);
			}
			ImGui::EndMainMenuBar();
		}
	}

	if (show_log) {
		cpu_log.Draw("CPU Log");
		ppu_log.Draw("PPU Log");
		general_log.Draw("General Log");
	}

	if (show_memedit) {
		if (ImGui::Begin("Memory Viewer/Editor")) {
			if (ImGui::BeginTabBar("ROM Dump")) {
				if (ImGui::BeginTabItem("RAM")) {
					memedit_ram.DrawContents(bus.ram, 0x800, 0x0000);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("PRG-ROM")) {
					memedit_prg.DrawContents(bus.prg_rom, 0x4000 * bus.mapper->prg_rom_size, 0x0000);
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Pattern Table Memory")) {
					memedit_chr1.DrawContents(ppu.pattern_table, 0x2000, 0x0000);
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
	}

	if (debug) {
		if (ImGui::Begin("Debugger", NULL, ImGuiWindowFlags_NoCollapse)) {
			if (ImGui::TreeNode("CPU Info")) {
				ImGui::Text("Register Dump");
				ImGui::Text("A: $%02X X: $%02X Y: $%02X", cpu.A, cpu.X, cpu.Y);
				ImGui::Text("SP: %02X P: %02X Cycles: %d", cpu.S, cpu.P, cpu.cycles);
				ImGui::Text("PC: $%04X Opcode $%02X", cpu.pc, cpu.read(cpu.pc));
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("PPU Info")) {
				ImGui::Text("Dot %d", ppu.dot);
				ImGui::Text("Mirroring: %s", bus.mapper->flags6 & 1 ? "Horizontal" : "Vertical");
				ImGui::Text("ppu.cframe: %d\nppu.ccyc: %d\nppu.cline: %d", ppu.cframe, ppu.ccyc, ppu.cline);
				ImGui::Text("Scanline %d", ppu.scanline);
				ImGui::Text("Pattern Table 0");
				render_pattern_texture(pattern_tex0, 0);
				ImGui::Image(pattern_tex0, ImVec2(128, 128));
				ImGui::Text("Pattern Table 1");
				render_pattern_texture(pattern_tex1, 1);
				ImGui::Image(pattern_tex1, ImVec2(128, 128));
				ImGui::TreePop();
			}
			ImGui::End();
		}

		ImGui::ShowDemoWindow();
	}

	ImGui::EndFrame();

	return rv;
}


void NesEmu::run(void)
{
rep:
	memset((void *) &bus, 0, sizeof bus);
	memset((void *) &cpu, 0, sizeof ppu);
	memset((void *) &ppu, 0, sizeof ppu);
	mapper = (iNesMapper *) rom;
	bus.plugin_mapper(mapper);
	bus.set_memory(rom);
	if (!bus.select_cartridge(mapper)) {
		goto fail;
	}
	cpu.power_on(0xC000);
	cpu.attach_bus(&bus);
	cpu.attach_ppu(&ppu);
	ppu.init_palette_table();
	ppu.attach_bus(&bus);
	ppu.set_mirroring();
	ppu.copy_pattern_table();
	ppu.power_up();
	bus.attach_ppu(&ppu);
	w = 256 * 1.5f;
	h = 240 * 1.5f;
	window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
											  w * s, h * s, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	render = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	ImGui::CreateContext();
	NoobInterface::Init(window);
	NoobInterface::SetRenderingMode(render, _SDL_RENDERING_MODE);
	ImGui::StyleColorsDark();
	SDL_GetWindowSize(window, &scale.w, &scale.h);
	tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240);
	if (!pattern_tex0) { pattern_tex0 = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 128); }
	if (!pattern_tex1) { pattern_tex1 = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 128); }
	ImGui::PushStyleColor(ImGuiCol_Text, 0xFFBABABA);
	ImGui::PushStyleColor(ImGuiCol_Button, 0xFF000000);
	ImGui::PushStyleColor(ImGuiCol_CheckMark, 0xFF900000);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, 0xF11F1F1F);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, 0xFF101010);
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, 0xFF101010);
	while (NoobInterface::isOpened()) {
		while (SDL_PollEvent(&ev)) {
			NoobInterface::ProcessEvents(&ev);
			switch (ev.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				switch (ev.type == SDL_KEYDOWN ? ev.key.keysym.scancode & 0xFF : 0xDEAD) {
				case SDL_SCANCODE_ESCAPE: failed = true; break;
				case SDL_SCANCODE_V: if (!realtime) cycle_frame(); break;
				case SDL_SCANCODE_X: if (!realtime) cycle_line(); break;
				case SDL_SCANCODE_Z: if (!realtime) cycle_pixel(); break;
				case SDL_SCANCODE_P:
					realtime = !realtime;
					break;
				case SDL_SCANCODE_J:
					main_menu_bar = !main_menu_bar;
					break;
				}
				break;
			case SDL_WINDOWEVENT:
				switch (ev.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					{
						SDL_GetWindowSize(window, &scale.w, &scale.h);
					}
					break;
				}
				break;
			}
		}

		if (realtime) { cycle_frame(); }
		if (draw_ui()) { break; }
		update_video();
		SDL_Delay(10);
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	NoobInterface::Destroy();
	ImGui::DestroyContext();
	SDL_DestroyTexture(pattern_tex0);
	SDL_DestroyTexture(pattern_tex1);
	SDL_DestroyTexture(tex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(render);
	pattern_tex0 = pattern_tex1 = tex = NULL;
	window = NULL;
	render = NULL;
fail:
	bus.clear_memory();
	if (reset) {
		reset = 0;
		load_rom(current_file);
		SDL_Delay(1000);
		goto rep;
	}
}
