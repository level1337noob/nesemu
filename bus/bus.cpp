#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <bus/bus.h>
#include <ppu/2c02.h>

#define o(g, ...) case (g): __VA_ARGS__ break;

#define SET_CARTRIDGE(o)				\
	cart_load = &Bus::mapper##o##_load;	\
	cart_store = &Bus::mapper##o##_store;


void Bus::plugin_mapper(iNesMapper *mapper)
{
	this->mapper = mapper;
}


void Bus::set_memory(unsigned char *memory, unsigned int size)
{
report_back:
	if (!this->memory) {
		this->memory = (unsigned char *) malloc(size);
		memcpy(this->memory + 0x10, memory, size);
		prg_rom = (memory + 0x10);
		chr_rom = ((memory + 0x10) + (0x4000 * mapper->prg_rom_size));
		return;
	} else {
		clear_memory();
		goto report_back;
	}
}


void Bus::clear_memory(void)
{
	free(this->memory);
	this->memory = NULL;
}


bool Bus::select_cartridge(iNesMapper *mapper)
{
	mapper_no = (mapper->flags7 & 0xF0) | ((mapper->flags6 & 0xF0) >> 4);
	bool ret {true};

	general_log.Report("Selected mapper %d\n", mapper_no);

	switch (mapper_no) {
	o(0x00, SET_CARTRIDGE(000); )

	default:
		general_log.Report("Unsupported mapper %03d\n", mapper_no);
		ret = false;
		break;
	}
	return ret;
}


u8 Bus::cpu_load(u16 addr)
{
	if (addr >= 0x0000 && addr <= 0x1FFF) {
		return ram[addr & 0x7FF];
	}

	if (addr >= 0x2000 && addr <= 0x3FFF) {
		return ppu->load(addr & 0x2007);
	}

	if (addr >= 0x4000 && addr <= 0x4017) {
		if (addr == OAMDMA) {
			return ppu->load(addr);
		}

		general_log.Report("Unhandled read to IO $%04x\n", addr);
		return io_register_load(addr & 0x1F);
	}

	if (addr >= 0x4018 && addr <= 0x401F) {
		general_log.Report("Unused IO Functionality at 0x%04X\n", addr);
		return 0;
	}

	if (addr >= 0x4020 && addr <= 0x7FFF) {
		//general_log.Report("Unused address at 0x%04X\n", addr);
		return 0;
	}

	return (this->*cart_load)(addr);
}


void Bus::cpu_store(u16 addr, u8 value)
{
	if (addr >= 0x0000 && addr <= 0x1FFF) {
		ram[addr & 0x7FF] = value;
		return;
	}

	if (addr >= 0x2000 && addr <= 0x3FFF) {
		return ppu->store(addr & 0x2007, value);
	}

	if (addr >= 0x4000 && addr <= 0x4017) {
		if (addr == OAMDMA) {
			return ppu->store(addr, value);
		}
		
		general_log.Report("Unhandled write to IO at $%04x with $%02X\n", addr, value);
		return io_register_store(addr & 0x1F, value);
	}

	if (addr >= 0x4018 && addr <= 0x401F) {
		general_log.Report("Disabled IO Functionality at 0x%04x\n", addr);
		return;
	}

	if (addr >= 0x4020 && addr <= 0x7FFF) {
		return;
	}
	return (this->*cart_store)(addr, value);
}


u8 Bus::io_register_load(u16)
{
	return 0;
}


void Bus::io_register_store(u16, u8 value)
{

}


// Cartridges
u8 Bus::mapper000_load(u16 addr)
{
	if (!mapper_no) {
		return prg_rom[addr & 0x3FFF];
	} else {
		return prg_rom[addr & 0x7FFF];
	}
}


void Bus::mapper000_store(u16 addr, u8 value)
{
	if (!mapper_no) {
		prg_rom[addr & 0x3FFF] = value;
	} else {
		prg_rom[addr & 0x7FFF] = value;
	}
}
