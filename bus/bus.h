#ifndef BUS_H
#define BUS_H

#include <types.h>
#include <mapper.h>
#include <imgui.h>
#include <log.h>

#define DEFINE_CARTRIDGE(nr) 					\
	u8 mapper##nr##_load(u16);		 			\
	void mapper##nr##_store(u16, u8);

extern Log general_log;
class Bus {
private:
	bool broken_bus {};
	class Ppu *ppu;
public:
	unsigned char mapper_no {};
	iNesMapper *mapper;
	u8 *memory {};
	u8 *prg_rom {};
	u8 *chr_rom {};
	u8 ram[0x800] {};

	void attach_ppu(class Ppu *ppu) { this->ppu = ppu; };

	void set_memory(u8 *memory, u32 = 0);
	void clear_memory();
	void plugin_mapper(iNesMapper *mapper);

	bool select_cartridge(iNesMapper *mapper);

	// This will be used for connecting to the CPU and PPU bus
	u8 cpu_load(u16);
	void cpu_store(u16, u8);


	u8 io_register_load(u16 addr);
	void io_register_store(u16 addr, u8 value);

	// The NES has different mappers related to Marat Fayzulin's mapper of the iNes
	u8 (Bus::*cart_load)(u16);
	void (Bus::*cart_store)(u16, u8);

	// The GamePak Cartridges
	DEFINE_CARTRIDGE(000)
};


#endif
