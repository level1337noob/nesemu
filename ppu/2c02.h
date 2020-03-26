#ifndef _2C02_PPU_H
#define _2C02_PPU_H

#include <types.h>
#include <imgui.h>
#include <log.h>
#include <bus/bus.h>

extern Log ppu_log;

enum {
	PPUCTRL = 0x2000,
	PPUMASK = 0x2001,
	PPUSTATUS = 0x2002,
	OAMADDR = 0x2003,
	OAMDATA = 0x2004,
	PPUSCROLL = 0x2005,
	PPUADDR = 0x2006,
	PPUDATA = 0x2007,
	OAMDMA = 0x4014,
};

class Ppu {
private:
	Bus *bus;
	unsigned int color_table[0x40];
public:
	void attach_bus(Bus *bus);
	u8 load(u16 addr);
	void store(u16 addr, u8 data);
	void power_up();
	void init_palette_table();
	void clock(u8 cycles);
};

#endif
