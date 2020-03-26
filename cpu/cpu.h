#ifndef CPU_H
#define CPU_H

#include <types.h>
#include <bus/bus.h>
#include <ppu/2c02.h>

#include <SDL2/SDL.h>
#include <imgui.h>
#include <log.h>

extern Log cpu_log;

enum {
	C = 0x01, Z = 0x02, I = 0x04, D = 0x08, B = 0x10, V = 0x40, N = 0x80
};

struct Instruction {
public:
	u8 opcode {};
	u16 old_pc {};
	u8 bytes {};
	u32 old_cycles {};
	const char *fmt {};
	void (*exec)(class Cpu *cpu);
};

class Cpu {
private:
	Bus *bus;
	Ppu *ppu;
	Instruction ins {};
public:
	void attach_ppu(Ppu *ppu);
	void attach_bus(Bus *bus);
	bool failed_opcode {};

	// r/w ops
	u8 read(u16);
	inline void write(u16, u8);
	inline u16 read16(u16);
	inline void write16(u16, u8);



	void power_on(u16 addr = 0xC000); // sets all the registers back in place when starting up
	u32 cycles {};
	u8 ticks_per_instruction {};


	// GPRs
	u8 A {}, X {}, Y {}, P {}, S {};
	u16 pc {};
	// status flag changes
	inline void stf(u8 flag);
	inline void clf(u8 flag);
	void tick(u8 v);
	// stack ops
	inline void push(u8);
	inline u8 pop();
	inline void push16(u16);
	inline u16 pop16();

	// interrupt vectors
	void reset(void);
	void rst(void);
	void irq(void);
	void brk(void);

	void nmi(void);
	unsigned char clock(); // only get the clocks of per instruction
};

#endif
