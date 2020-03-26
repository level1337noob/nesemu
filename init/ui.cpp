#include <init/nesemu.h>
#include <imgui.h>
#include <memedit.h>
#include <log.h>

bool NesEmu::draw_menu_bar(void)
{
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
				ImGui::Checkbox("Realtime Mode", &realtime);				
				ImGui::Checkbox("Show Memory Editor", &show_memedit);
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

			ImGui::EndMainMenuBar();
		}
	}
	return rv;
}


bool NesEmu::draw_memory_editor(void)
{
	if (broken_emulator_state)
		return false;
	static MemoryEditor memedit_prg, memedit_ram, memedit_chr1, memedit_chr2;
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
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
	}

	return true;
}
