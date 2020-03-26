#include <stdio.h>
#include <stdlib.h>
#include <init/nesemu.h>


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
	fclose(t);
	return true;
}
