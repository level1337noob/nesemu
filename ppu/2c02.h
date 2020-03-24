#ifndef _2C02_PPU_H
#define _2C02_PPU_H

#include <types.h>
#include <imgui.h>
#include <log.h>

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
public:

	void init_palette_table();
	void copy_pattern_table();

	void power_up();

	// rendering
	u16 dot {};
	s16 scanline {};
	bool frameOdd {};

	bool nmi {};
	bool vblank {};

	void VisibleScanline();
	void VblankScanline();
	void PostrenderScanline();

	u8 pattern_table[0x2000] {};
	u8 palette_ram[0x20] {};
	u32 color_table[0x40] {};
	u8 vram[2][0x400] {};

	u32 display[256*240] {};

	bool mirrored_nametable {};
	void set_mirroring() { mirrored_nametable = bus->mapper->flags6 & 1; } // 1 = Vertical 0 = Horizontal


	//	...
	int frames {};


	// debugging tools
	bool ccyc {};
	bool cline {};
	bool cframe {};
	void clock();

	u32 select_palette(u8 idx); // selects the color
	void plot_pixel(s32 x, s32 y, u32 csel);
	void attach_bus(Bus *bus);

	// the registers
	union { // $2000 > write
		struct {
			u8 basename_table_address_x : 1;
			u8 basename_table_address_y : 1;
			u8 vram_address : 1; // 0 : increment 1 : inc 32
			u8 spr_pattern_table_addr : 1; // 0: $0000 1: $1000 ignored in 8x16 mode
			u8 bg_pattern_table_addr : 1; // 0: 8x8 pixels 1: 8x16 pixels
			u8 spr_size : 1; // 0: 8x8 pixels 1: 8x16 pixels
			u8 master_slave_select : 1; // 0: read backdrop from ext pins 1: output color on ext pins
			u8 vblank_nmi_interval : 1; // generate an nmi at the start of the beginning vblank interval 0: off 1: on
		};

		u8 v;
	} ppu_ctrl;

	union { // $2001 > write
		struct {
			u8 grayscale : 1; // 0: normal color 1: produce a greyscale display
			u8 show_bg_l8pix : 1; // show the left most 8 pixels of the background to the screen 0: hide
			u8 show_spr_l8pix : 1; // show the left most 8 pixels of the background to the screen 0: hide
			u8 render_background : 1; // show background
			u8 render_sprite : 1; // show sprite
			u8 emph_red : 1; // emphasize red
			u8 emph_green : 1; // emphasize green
			u8 emph_blue : 1; // emphasize blue
		};

		u8 v;
	} ppu_mask;

	union { // $2002 > read
		struct {
			u8 reason : 4; // lsb for the ppu register not being updated for this address
			u8 : 1; // padding
			u8 spr_overflow : 1;
			u8 spr_hit : 1;
			u8 vblank : 1; // vblank has started 0: not in vblank 1: in vblank at line 241 at cycle 1
		};

		u8 v;
	} ppu_status;
	// TODO oamaddr oamdata
	u8 ppu_scroll {}; // $2005 > write x2

	u8 addr_latch {}; // For the PPU address

	u8 ppu_data {}; // $2007 > read/write

/*
	Background
	VRAM address, temporary VRAM address, fine X scroll, and first/second write toggle - This controls the addresses that the PPU reads during background rendering. See PPU scrolling.
	2 16-bit shift registers - These contain the pattern table data for two tiles. Every 8 cycles, the data for the next tile is loaded into the upper 8 bits of this shift register. Meanwhile, the pixel to render is fetched from one of the lower 8 bits.
	2 8-bit shift registers - These contain the palette attributes for the lower 8 pixels of the 16-bit shift register. These registers are fed by a latch which contains the palette attribute for the next tile. Every 8 cycles, the latch is loaded with the palette attribute for the next tile.
*/
	// Preface of the background registers
	u16 tileShiftLower { 1 }, tileShiftUpper { 1 };
	u8 bg_next_tile_msb {};
	u8 bg_next_tile_lsb {};
	u8 attribShiftLower { 1 }, attribShiftUpper { 1 };
	u8 attrib_next_tile_msb {};
	u8 attrib_next_tile_lsb {};
	u8 bg_next_tile_id {};
	u8 bg_next_tile_attrib {};

	/* Loopy's VRAM address */
	union Addr {
		struct {
		    u8 cX : 5;  // Coarse X.
		    u8 cY : 5;  // Coarse Y.
		    u8 nX : 1;  // Nametable X.
		    u8 nY : 1;  // Nametable Y.
		    u8 fY : 3;  // Fine Y.
			u8 : 1;
		};
		
		u16 r;
	};

	u8 fX {};
	Addr taddr;
	Addr vaddr;


	bool latch {};

	u8 register_load(u16 addr, bool rd = 0);
	void register_store(u16 addr, u8 v);
	u16 read(u16 addr);
	void write(u16 addr, u8 v);

	void select_palette_ram(u8 palette, u8 pixel);

	u32 GetColorFromPaletteRam(u8 palette, u8 pixel);
	const u8 access_nametable(const u16 addr,
							  const u8 mirrored);
};

#endif
