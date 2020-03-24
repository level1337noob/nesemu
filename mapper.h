#ifndef MAPPER_H
#define MAPPER_H

// Mapper format by Marat Fayzullin
struct iNesMapper {
	unsigned char ines_magic[4];
	unsigned char prg_rom_size;
	unsigned char chr_rom_size;
	// yea i got lazy, skipped
	unsigned char flags6;
	unsigned char flags7;
	unsigned char prg_ram_size;
	unsigned char tv_system;
	unsigned char unused[6];
};

#endif
