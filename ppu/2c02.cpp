#include <stdio.h>
#include <types.h>
#include <bus/bus.h>
#include <string.h>
#include <ppu/2c02.h>
#include <stdlib.h>

void Ppu::attach_bus(Bus *bus) { this->bus = bus; }
void Ppu::plot_pixel(s32 x, s32 y, u32 csel)
{
	if (x >= 256 || y >= 240 || x < 0 || y < 0) { return; }
	display[y * 256 + x] = csel;
}


void Ppu::power_up()
{
	ppu_ctrl.v = 0;
	ppu_mask.v = 0;
	ppu_status.v = 0;
	// oamaddr.v = 0;
	latch = 0;
	taddr.r = 0;
	vaddr.r = 0;
	ppu_data = 0;
	// put random crap in the palette table color
	for (int i = 0; i < 0x20; i++) {
		palette_ram[i] = rand();
	}
	palette_ram[0] = 0x00;	
}


#define o(i, j) color_table[i] = j;
#define MASK(r, g, b) (((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF))
void Ppu::init_palette_table()
{
	o(0x00, MASK( 84,  84,  84))
	o(0x01, MASK(  0,  30, 116))
	o(0x02, MASK(  8,  16, 144))
	o(0x03, MASK( 48,   0, 136))
	o(0x04, MASK( 68,   0, 100))
	o(0x05, MASK( 92,   0,  48))
	o(0x06, MASK( 84,   4,   0))
	o(0x07, MASK( 60,  24,   0))
	o(0x08, MASK( 32,  42,   0))
	o(0x09, MASK( 32,  42,   0))
	o(0x0A, MASK(  0,  64,   0))
	o(0x0B, MASK(  0,  60,   0))
	o(0x0C, MASK(  0,  50,  60))
	o(0x0D, MASK(  0,   0,   0))
	o(0x0E, MASK(  0,   0,   0))
	o(0x0F, MASK(  0,   0,   0))
	o(0x10, MASK(150, 150, 152))
	o(0x11, MASK(  8,  76, 196))
	o(0x12, MASK( 48,  50, 236))
	o(0x13, MASK( 92,  30, 228))
	o(0x14, MASK(136,  20, 176))
	o(0x15, MASK(160,  20, 100))
	o(0x16, MASK(152,  34,  32))
	o(0x17, MASK(120,  60,   0))
	o(0x18, MASK( 84,  90,   0))
	o(0x19, MASK( 40, 114,   0))
	o(0x1A, MASK(  8, 124,   0))
	o(0x1B, MASK(  0, 118,  40))
	o(0x1C, MASK(  0, 102, 120))
	o(0x1D, MASK(  0,   0,   0))
	o(0x1E, MASK(  0,   0,   0))
	o(0x1F, MASK(  0,   0,   0))
	o(0x20, MASK(236, 238, 236))
	o(0x21, MASK( 76, 154, 236))
	o(0x22, MASK(120, 124, 236))
	o(0x23, MASK(176,  98, 236))
	o(0x24, MASK(228,  84, 236))
	o(0x25, MASK(236,  88, 180))
	o(0x26, MASK(236, 106, 100))
	o(0x27, MASK(212, 136,  32))
	o(0x28, MASK(160, 170,   0))
	o(0x29, MASK(116, 196,   0))
	o(0x2A, MASK( 76, 208,  32))
	o(0x2B, MASK( 56, 204, 108))
	o(0x2C, MASK( 56, 180, 204))
	o(0x2D, MASK( 60,  60,  60))
	o(0x2E, MASK(  0,   0,   0))
	o(0x2F, MASK(  0,   0,   0))
	o(0x30, MASK(236, 238, 236))
	o(0x31, MASK(168, 204, 236))
	o(0x32, MASK(188, 188, 236))
	o(0x33, MASK(212, 178, 236))
	o(0x34, MASK(236, 174, 236))
	o(0x35, MASK(236, 174, 212))
	o(0x36, MASK(236, 180, 176))
	o(0x37, MASK(228, 196, 144))
	o(0x38, MASK(204, 210, 120))
	o(0x39, MASK(180, 222, 120))
	o(0x3A, MASK(168, 226, 144))
	o(0x3B, MASK(152, 226, 180))
	o(0x3C, MASK(160, 214, 228))
	o(0x3D, MASK(160, 162, 160))
	o(0x3E, MASK(  0,   0,   0))
	o(0x3F, MASK(  0,   0,   0))
}
#undef MASK
#undef o

u8 Ppu::register_load(u16 addr, bool rd)
{
	ppu_log.Report("Unhandled read to ppu register in address $%04x\n", addr);
	return 0;
}


void Ppu::register_store(u16 addr, u8 v)
{
	ppu_log.Report("Unhandled write to ppu register in address $%04x with $%02x\n", addr, v);
}


// Selects a specific palette
u32 Ppu::select_palette(u8 idx)
{
	return color_table[idx&0x3F];
}


void Ppu::copy_pattern_table()
{
	if (bus->mapper->chr_rom_size) {
		for (int i = 0; i < 0x2000; i++) {
			pattern_table[i] = bus->chr_rom[i];
		}
		ppu_log.Report("Copied pattern table from Bus ROM\n");
	} else {
		ppu_log.Report("Using CHR-RAM\n");
	}
}


const u8 Ppu::access_nametable(const u16 addr,
				const u8 mirrored)
{
	u8 rv;
	if (mirrored) {
		if ((addr >= 0x000 && addr <= 0x3FF)
			|| (addr >= 0x800 && addr <= 0xBFF)) {
			rv = 0;
		} else {
			rv = 1;
		}
	} else {
		if ((addr >= 0x000 && addr <= 0x3FF)
			|| (addr >= 0x400 && addr <= 0x7FF)) {
			rv = 0;
		} else {
			rv = 1;
		}
	}
	return rv;
}

void Ppu::write(u16 addr, u8 v)
{
	u8 r;addr &= 0x3FFF;
	switch (addr & 0x3000) {
	case 0x0000: case 0x1000: pattern_table[addr] = v; return;
	case 0x2000:e: addr &= 0xFFF; r = access_nametable( addr, mirrored_nametable ); vram[r][addr & 0x3FF] = v; return;
	case 0x3000: if (addr <= 0x3EFF) { addr &= 0xEFF; goto e; }
	if ((addr & 0x13) == 0x10) addr &= ~0x10;
	palette_ram[addr & 0x1F] = v & (ppu_mask.grayscale ? 0x30 : 0xFF);
	}
}


u16 Ppu::read(u16 addr)
{
	u8 v;addr &= 0x3FFF;
	switch (addr & 0x3000) {
	case 0x0000: case 0x1000: return pattern_table[addr];
	case 0x2000:e: addr &= 0xFFF; v = access_nametable( addr, mirrored_nametable ); return vram[v][addr & 0x3FF];
	case 0x3000: if (addr <= 0x3EFF) { addr &= 0xEFF; goto e; }
	if ((addr & 0x13) == 0x10) addr &= ~0x10;
	return palette_ram[addr & 0x1F] & (ppu_mask.grayscale ? 0x30 : 0xFF);
	}

	return 0;
}

void Ppu::clock()
{
	//	...
	ccyc = true;
	if (dot++ >= 341) {
		cline = true;
		if (scanline++ == 261) {
			ppu_status.vblank = 0;
			vblank = false;
			frameOdd ^= 1;
			cframe = true;
			frames++;
			scanline = -1;
		}
		dot -= 341;
	}
}
