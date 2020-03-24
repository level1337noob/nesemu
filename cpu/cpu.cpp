// cpu made in 10+ hours already tested on some nes binaries
#include <stdio.h>
#include <stdlib.h>
#include <cpu/cpu.h>
static Instruction ins;
void Cpu::attach_ppu(Ppu *ppu) { this->ppu = ppu; }
void Cpu::attach_bus(Bus *bus) { this->bus = bus; }
inline void Cpu::stf(u8 flag) { P |= flag; } // set the flag
inline void Cpu::clf(u8 flag) { P &=~flag; } // clear the flag
inline void Cpu::push(u8 v) { bus->cpu_store(0x100+S--, v); } // stores on the bus
inline u8 Cpu::pop() { return bus->cpu_load(++S+0x100); } // loads on the bus
inline void Cpu::push16(u16 v) { push(v >> 8); push(v & 0xFF); }
inline u16 Cpu::pop16() { u8 g = pop(), t = pop(); return g | t << 8; }
inline u8 Cpu::read(u16 a) { return bus->cpu_load(a); }	//	...
inline void Cpu::write(u16 a, u8 v) { return bus->cpu_store(a, v); }	//	...
inline u16 Cpu::read16(u16 a) { u8 n = read(a), x = read(a + 1); return x << 8 | n; }	//	...
inline void Cpu::write16(u16 a, u8 v) { write(a, v >> 8), write(a, v & 0xFF); }	//	...
void Cpu::power_on(u16 addr) { A = X = Y = 0, S = 0xFD, P = 0x34, cycles = 7; pc = addr; }
#define n(i) push16(pc), push(P), pc = read16(i)
void Cpu::rst(void) { n(0xFFFC);          tick(7); }	//	reset vec
void Cpu::nmi(void) { P &=~B; n(0xFFFA);  tick(7); }	//	nmi on the chip itself used soon
void Cpu::irq(void) { P &=~B; n(0xFFFE);  tick(7); }	//	irq
void Cpu::brk(void) { if (P & I) { irq(); } else { P |= I, P |= B; n(0xFFFE), tick(7); } } // brk swappable with irq
#undef n
void Cpu::reset(void) { S -= 3, P |= I; } // resets the state
#define nz(i) ((i) & 0x80) ? cpu->stf(N) : cpu->clf(N), !(i) ? cpu->stf(Z) : cpu->clf(Z)
#define imm() (cpu->read(cpu->pc++))
#define imm16() (cpu->read(cpu->pc) | (cpu->read(cpu->pc + 1) << 8) )
void lda(Cpu *cpu, u8 v) { cpu->A = v; nz(cpu->A); } // load/store
void ldx(Cpu *cpu, u8 v) { cpu->X = v; nz(cpu->X); }
void ldy(Cpu *cpu, u8 v) { cpu->Y = v; nz(cpu->Y); }
void sta(Cpu *cpu, u16 a) { cpu->write(a, cpu->A); }
void stx(Cpu *cpu, u16 a) { cpu->write(a, cpu->X); }
void sty(Cpu *cpu, u16 a) { cpu->write(a, cpu->Y); }
void tax(Cpu *cpu) { cpu->X = cpu->A; nz(cpu->X); } // reg to
void tay(Cpu *cpu) { cpu->Y = cpu->A; nz(cpu->Y); }
void txa(Cpu *cpu) { cpu->A = cpu->X; nz(cpu->A); }
void tya(Cpu *cpu) { cpu->A = cpu->Y; nz(cpu->A); }
void tsx(Cpu *cpu) { cpu->X = cpu->S; nz(cpu->X); } // stack ops
void txs(Cpu *cpu) { cpu->S = cpu->X; }
void pha(Cpu *cpu) { cpu->push(cpu->A); }
void php(Cpu *cpu) { cpu->push(cpu->P | B); }
void pla(Cpu *cpu) { cpu->A = cpu->pop(); nz(cpu->A); }
void plp(Cpu *cpu) { cpu->P = cpu->pop(); if (cpu->P & B) { cpu->clf(B); } else { cpu->stf(0x20); } }
void _and(Cpu *cpu, u8 v) { cpu->A &= v; nz(cpu->A); } // logical
void eor(Cpu *cpu, u8 v) { cpu->A ^= v; nz(cpu->A); }
void ora(Cpu *cpu, u8 v) { cpu->A |= v; nz(cpu->A); }
void bit(Cpu *cpu, u8 v) { u8 result = cpu->A & v; !result? cpu->stf(Z) : cpu->clf(Z);
						   v & 0x80 ? cpu->stf(N) : cpu->clf(N);
						   v & 0x40 ? cpu->stf(V) : cpu->clf(V); }
void inc(Cpu *cpu, u16 a, u8 v) { cpu->write(a, ++v); nz(v); } // incs/decs
void inx(Cpu *cpu) { cpu->X++; nz(cpu->X); }
void iny(Cpu *cpu) { cpu->Y++; nz(cpu->Y); }
void dec(Cpu *cpu, u16 a, u8 v) { cpu->write(a, --v); nz(v); }
void dex(Cpu *cpu) { cpu->X--; nz(cpu->X); }
void dey(Cpu *cpu) { cpu->Y--; nz(cpu->Y); }
void jmp(Cpu *cpu, u16 addr) { cpu->pc = addr; } // jmp/ret
void jsr(Cpu *cpu) { cpu->push16(cpu->pc + 1); cpu->pc = cpu->read16(cpu->pc); }
void rts(Cpu *cpu) { cpu->pc = cpu->pop16() + 1; }
#define n(br, c)		\
	void br(Cpu *cpu) { s8 i = imm();  if (c) { if ((((u8)cpu->pc)+(s8)i)>=0x100) cpu->tick(1);;  cpu->pc += (s8) i; cpu->tick(1); } else {  } }
n(bcc, !(cpu->P & C) )n(bcs,  (cpu->P & C) )n(bpl, !(cpu->P & N) )n(bmi,  (cpu->P & N) ) // branches
n(bvc, !(cpu->P & V) )n(bvs,  (cpu->P & V) )n(bne, !(cpu->P & Z) )n(beq,  (cpu->P & Z) )
#undef n
void clc(Cpu *cpu) { cpu->clf(C); } // sfc
void cld(Cpu *cpu) { cpu->clf(D); }
void cli(Cpu *cpu) { cpu->clf(I); }
void clv(Cpu *cpu) { cpu->clf(V); }
void sec(Cpu *cpu) { cpu->stf(C); }
void sed(Cpu *cpu) { cpu->stf(D); }
void sei(Cpu *cpu) { cpu->stf(I); }
void nop(Cpu *cpu) { } // sys funcs
void rti(Cpu *cpu) { cpu->P = cpu->pop(), cpu->pc = cpu->pop16(); if (cpu->P & B) { cpu->clf(B); } else { cpu->stf(0x20); } }
void adc(Cpu *cpu, u8 v) { // https://stackoverflow.com/questions/29193303/6502-emulation-proper-way-to-implement-adc-and-sbc answer by Ulfalizer, Arithmetic
	const u16 result = cpu->A + v + (cpu->P & C);
	bool carry = result>0xFF;
	bool overflow = ~(cpu->A ^ v) & (cpu->A ^ result) & 0x80;
	cpu->A = (u8)result&0xFF;
	overflow?cpu->stf(V):cpu->clf(V);
	carry?cpu->stf(C):cpu->clf(C);
	nz(cpu->A);
}
void sbc(Cpu *cpu, u8 v) { adc(cpu, ~(v)); } // just complement or A xor 0xFF it
#define n(c, reg)	\
	void c(Cpu *cpu, u8 v) {u8 res = cpu->reg - v;						\
							cpu->reg >= v ? cpu->stf(C) : cpu->clf(C);	\
							cpu->reg == v ? cpu->stf(Z) : cpu->clf(Z);	\
							res & 0x80 ? cpu->stf(N) : cpu->clf(N); }
n(cmp, A)n(cpx, X)n(cpy, Y)
#undef n
// shifts
void asl_a(Cpu *cpu) { cpu->A & 0x80 ? cpu->stf(C) : cpu->clf(C), cpu->A <<= 1, nz(cpu->A); } // accumulator shifts
void lsr_a(Cpu *cpu) { cpu->A & 0x01 ? cpu->stf(C) : cpu->clf(C), cpu->A >>= 1, nz(cpu->A); }
void ror_a(Cpu *cpu) { bool t = (cpu->A & 0x1) != 0; cpu->A = (cpu->A >> 1) | ((cpu->P & C) << 7); nz(cpu->A); t ? cpu->stf(C) : cpu->clf(C); }
void rol_a(Cpu *cpu) { bool t = (cpu->A & 0x80) != 0; cpu->A = (cpu->A << 1) | ((cpu->P & C)); nz(cpu->A); t ? cpu->stf(C) : cpu->clf(C); }
void asl(Cpu *cpu, u16 a, u8 v) { v & 0x80 ? cpu->stf(C) : cpu->clf(C), v <<= 1, nz(cpu->A); cpu->write(a, v); }
void lsr(Cpu *cpu, u16 a, u8 v) { v & 0x01 ? cpu->stf(C) : cpu->clf(C), v >>= 1, nz(cpu->A); cpu->write(a, v); }
void ror(Cpu *cpu, u16 a, u8 v) { bool t = (v & 0x01) != 0; v = (v >> 1) | ((cpu->P & C) << 7); nz(v); t ? cpu->stf(C) : cpu->clf(C); cpu->write(a, v); }
void rol(Cpu *cpu, u16 a, u8 v) { bool t = (v & 0x80) != 0; v = (v << 1) | ((cpu->P & C)); nz(v); t ? cpu->stf(C) : cpu->clf(C); cpu->write(a, v); }
// unofficial opcodes
void alr(Cpu *cpu) { } // combined ops
void anc(Cpu *cpu) { }
void arr(Cpu *cpu) { }
void axs(Cpu *cpu) { }
void lax(Cpu *cpu, u8 v) { cpu->X = cpu->A = v; nz(cpu->X); }
void sax(Cpu *cpu, u16 a) { cpu->write(a, cpu->A & cpu->X); }
void dcp(Cpu *cpu, u16 a, u8 v) { cpu->write(a, --v); cpu->A>=v?cpu->stf(C):cpu->clf(C); u8 i = cpu->A - v; nz(i); } // rmw ops
void isc(Cpu *cpu, u16 a, u8 v) { cpu->write(a, ++v); sbc(cpu, v); }
void rla(Cpu *cpu, u16 a, u8 v) { bool t = (v & 0x80) != 0; v = (v << 1) | ((cpu->P & C)); t ? cpu->stf(C) : cpu->clf(C); cpu->write(a, v); cpu->A &= v; nz(cpu->A); }
void rra(Cpu *cpu, u16 a, u8 v) { bool t = (v & 0x01) != 0; v = (v >> 1) | ((cpu->P & C) << 7); t ? cpu->stf(C) : cpu->clf(C); cpu->write(a, v); cpu->A &= v; nz(cpu->A); }
void slo(Cpu *cpu, u16 a, u8 v) { v&0x80?cpu->stf(C):cpu->clf(C); v<<=1; cpu->write(a, v); cpu->A |= v; nz(cpu->A); }
void sre(Cpu *cpu, u16 a, u8 v) { v&0x01?cpu->stf(C):cpu->clf(C);cpu->A^=(v>>1);nz(cpu->A);cpu->write(a, v>>1); }
void Cpu::tick(u8 v) { _cycle += v; } // the addressing modes
bool Cpu::clock(Instruction& i) // decode pla
{
	#define zpg_x() ((read(pc)+X)&0xFF)
	#define zpg_y() ((read(pc)+Y)&0xFF)
	#define ind_x() (read((read(pc)+X)&0xFF) + read((read(pc)+X+1)&0xFF)*256)
	#define ind_y() (read(read(pc))+read((read(pc)+1)&0xFF)*256)
	#define misfire_y() if ((ind_y()+Y)&0x10000) tick(1); else if (((ind_y()&0xFF)+Y)&0x100) tick(1)
	#define l(i) (read16(pc)+i)
	#define misfire_a(g) if ((read16(pc)+g)&0x10000) tick(1); else if (((read16(pc)&0xFF)+g)&0x100) tick(1)
	#define I u16 n = ind_y()+Y
	#define o(_i, b, c, ...) case (_i): {i.bytes = b; this->tick(c); __VA_ARGS__} break;

	_cycle = 0;
	i.old_cycles = cycles;
	switch (i.opcode = read(i.old_pc = pc++)) {
	//	...
	o(0x4C, 3, 3, jmp(this, read16(pc)); ) o(0xA2, 2, 2, ldx(this, read(pc++)); )
	o(0x86, 2, 3, stx(this, read(pc++)); ) o(0x20, 3, 6, jsr(this); )
	// unofficial mirrors
	case 0x1A:case 0x3A:case 0x5A:case 0x7A:case 0xDA:case 0xFA:
	o(0xEA, 1, 2, nop(this); )o(0x38, 1, 2, sec(this); )o(0xB0, 2, 2, bcs(this); ) o(0x18, 1, 2, clc(this); )o(0x90, 2, 2, bcc(this); ) o(0xA9, 2, 2, lda(this, read(pc++)); )
	o(0xF0, 2, 2, beq(this); ) o(0xD0, 2, 2, bne(this); )o(0x85, 2, 3, sta(this, read(pc++)); )o(0x24, 2, 3, bit(this, read(read(pc++))); )o(0x70, 2, 2, bvs(this); ) o(0x50, 2, 2, bvc(this); )
	o(0x10, 2, 2, bpl(this); ) o(0x60, 1, 6, rts(this); )o(0x78, 1, 2, sei(this); ) o(0xF8, 1, 2, sed(this); )o(0x08, 1, 3, php(this); ) o(0x68, 1, 4, pla(this); )o(0x29, 2, 2, _and(this, read(pc++)); ) o(0xC9, 2, 2, cmp(this, read(pc++)); )
	o(0xD8, 1, 2, cld(this); ) o(0x48, 1, 3, pha(this); )o(0x28, 1, 4, plp(this); ) o(0x30, 2, 2, bmi(this); )o(0x09, 2, 2, ora(this, read(pc++)); ) o(0xB8, 1, 2, clv(this); )o(0x49, 2, 2, eor(this, read(pc++)); )
	// unofficial mirrors
	o(0x69, 2, 2, adc(this, read(pc++)); )o(0xA0, 2, 2, ldy(this, read(pc++)); ) o(0xC0, 2, 2, cpy(this, read(pc++)); )o(0xE0, 2, 2, cpx(this, read(pc++)); )case 0xEB:o(0xE9, 2, 2, sbc(this, read(pc++)); )
	o(0xC8, 1, 2, iny(this); ) o(0xE8, 1, 2, inx(this); )o(0x88, 1, 2, dey(this); ) o(0xCA, 1, 2, dex(this); )o(0xA8, 1, 2, tay(this); )o(0xAA, 1, 2, tax(this); )o(0x98, 1, 2, tya(this); )  o(0x8A, 1, 2, txa(this); )o(0xBA, 1, 2, tsx(this); ) o(0x8E, 3, 4, stx(this, read16(pc)); pc += 2; )
	o(0x9A, 1, 2, txs(this); ) o(0xAE, 3, 4, ldx(this, read(read16(pc))); pc += 2; )o(0xAD, 3, 4, lda(this, read(read16(pc))); pc += 2; )o(0x40, 1, 6, rti(this);   ) o(0x4A, 1, 2, lsr_a(this); )o(0x0A, 1, 2, asl_a(this); ) o(0x6A, 1, 2, ror_a(this); )o(0x2A, 1, 2, rol_a(this); ) o(0xA5, 2, 3, lda(this, read(read(pc++))); )
	o(0x8D, 3, 4, sta(this, read16(pc)); pc += 2; ) o(0xA1, 2, 6, lda(this, read(ind_x())); pc += 1; )o(0x81, 2, 6, sta(this, ind_x()); pc += 1; ) o(0x01, 2, 6, ora(this, read(ind_x())); pc += 1; )o(0x21, 2, 6, _and(this, read(ind_x())); pc += 1; ) o(0x41, 2, 6, eor(this, read(ind_x())); pc += 1; )o(0x61, 2, 6, adc(this, read(ind_x())); pc += 1; ) o(0xC1, 2, 6, cmp(this, read(ind_x())); pc += 1; )
	o(0xE1, 2, 6, sbc(this, read(ind_x())); pc += 1; ) o(0xA4, 2, 3, ldy(this, read(read(pc++))); )o(0x84, 2, 3, sty(this, read(pc++)); ) o(0xA6, 2, 3, ldx(this, read(read(pc++))); )o(0x05, 2, 3, ora(this, read(read(pc++))); ) o(0x25, 2, 3, _and(this, read(read(pc++))); )
	o(0x45, 2, 3, eor(this, read(read(pc++))); ) o(0x65, 2, 3, adc(this, read(read(pc++))); )o(0xC5, 2, 3, cmp(this, read(read(pc++))); ) o(0xE5, 2, 3, sbc(this, read(read(pc++))); )o(0xE4, 2, 3, cpx(this, read(read(pc++))); ) o(0xC4, 2, 3, cpy(this, read(read(pc++))); )
	o(0x46, 2, 5, lsr(this, read(pc), read(read( pc))); pc += 1; ) o(0x06, 2, 5, asl(this, read(pc), read(read(pc))); pc += 1; )o(0x66, 2, 5, ror(this, read(pc), read(read(pc))); pc += 1; ) o(0x26, 2, 5, rol(this, read(pc), read(read(pc))); pc += 1; )o(0xE6, 2, 5, inc(this, read(pc), read(read(pc))); pc += 1; ) o(0xC6, 2, 5, dec(this, read(pc), read(read(pc))); pc += 1; )
	o(0xAC, 3, 4, ldy(this, read(read16(pc))); pc += 2; ) o(0x8C, 3, 4, sty(this, read16(pc)); pc += 2; )
	o(0x2C, 3, 4, bit(this, read(read16(pc))); pc += 2; ) o(0x0D, 3, 4, ora(this, read(read16(pc))); pc += 2; )
	o(0x2D, 3, 4, _and(this, read(read16(pc))); pc += 2; ) o(0x4D, 3, 4, eor(this, read(read16(pc))); pc += 2; )
	o(0x6D, 3, 4, adc(this, read(read16(pc))); pc += 2; ) o(0xCD, 3, 4, cmp(this, read(read16(pc))); pc += 2; )
	o(0xED, 3, 4, sbc(this, read(read16(pc))); pc += 2; ) o(0xEC, 3, 4, cpx(this, read(read16(pc))); pc += 2; )
	o(0xCC, 3, 4, cpy(this, read(read16(pc))); pc += 2; ) o(0x4E, 3, 6, lsr(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0x0E, 3, 6, asl(this, read16(pc), read(read16(pc))); pc += 2; ) o(0x6E, 3, 6, ror(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0x2E, 3, 6, rol(this, read16(pc), read(read16(pc))); pc += 2; ) o(0xEE, 3, 6, inc(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0xCE, 3, 6, dec(this, read16(pc), read(read16(pc))); pc += 2; ) o(0xB1, 2, 5, I; lda(this, read(n)); misfire_y(); pc++; )
	o(0x11, 2, 5, I; ora(this, read(n)); misfire_y(); pc++; ) o(0x31, 2, 5, I; _and(this, read(n)); misfire_y(); pc++; )
	o(0x51, 2, 5, I; eor(this, read(n)); misfire_y(); pc++; ) o(0x71, 2, 5, I; adc(this, read(n)); misfire_y(); pc++; )
	o(0xD1, 2, 5, I; cmp(this, read(n)); misfire_y(); pc++; ) o(0xF1, 2, 5, I; sbc(this, read(n)); misfire_y(); pc++; )o(0x91, 2, 6, I; sta(this, n); pc++; )
	o(0x6C, 3, 5, u16 addr = (read16(pc)); if ((addr&0xFF) == 0xFF) { addr = read(addr) + (read(addr & 0xFF00) << 8); jmp(this, addr); } else { jmp(this, read16(addr)); } )
	o(0xB9, 3, 4, lda(this, read(l(Y))); misfire_a(Y); pc += 2; )o(0x19, 3, 4, ora(this, read(l(Y))); misfire_a(Y); pc += 2; )
	o(0x39, 3, 4, _and(this, read(l(Y))); misfire_a(Y); pc += 2; )o(0x59, 3, 4, eor(this, read(l(Y))); misfire_a(Y); pc += 2; )
	o(0x79, 3, 4, adc(this, read(l(Y))); misfire_a(Y); pc += 2; )o(0xD9, 3, 4, cmp(this, read(l(Y))); misfire_a(Y); pc += 2; )
	o(0xF9, 3, 4, sbc(this, read(l(Y))); misfire_a(Y); pc += 2; )o(0x99, 3, 5, sta(this, l(Y)); pc += 2; )
	o(0xB4, 2, 4, ldy(this, read(zpg_x())); pc++; )o(0x94, 2, 4, sty(this, zpg_x()); pc++; )
	o(0x15, 2, 4, ora(this, read(zpg_x())); pc++; )o(0x35, 2, 4, _and(this, read(zpg_x())); pc++; )
	o(0x55, 2, 4, eor(this, read(zpg_x())); pc++; )o(0x75, 2, 4, adc(this, read(zpg_x())); pc++; )
	o(0xD5, 2, 4, cmp(this, read(zpg_x())); pc++; )o(0xF5, 2, 4, sbc(this, read(zpg_x())); pc++; )
	o(0xB5, 2, 4, lda(this, read(zpg_x())); pc++; )o(0x95, 2, 4, sta(this, zpg_x()); pc++; )
	o(0x56, 2, 6, lsr(this, zpg_x(), read(zpg_x())); pc++; )o(0x16, 2, 6, asl(this, zpg_x(), read(zpg_x())); pc++; )
	o(0x76, 2, 6, ror(this, zpg_x(), read(zpg_x())); pc++; )o(0x36, 2, 6, rol(this, zpg_x(), read(zpg_x())); pc++; )
	o(0xF6, 2, 6, inc(this, zpg_x(), read(zpg_x())); pc++; )o(0xD6, 2, 6, dec(this, zpg_x(), read(zpg_x())); pc++; )
	o(0xB6, 2, 4, ldx(this, read(zpg_y())); pc++; )o(0x96, 2, 4, stx(this, zpg_y()); pc++; )
	o(0xBC, 3, 4, ldy(this, read(l(X))); misfire_a(X); pc += 2; )o(0x1D, 3, 4, ora(this, read(l(X))); misfire_a(X); pc += 2; )
	o(0x3D, 3, 4, _and(this, read(l(X))); misfire_a(X); pc += 2; )o(0x5D, 3, 4, eor(this, read(l(X))); misfire_a(X); pc += 2; )
	o(0x7D, 3, 4, adc(this, read(l(X))); misfire_a(X); pc += 2; )o(0xDD, 3, 4, cmp(this, read(l(X))); misfire_a(X); pc += 2; )
	o(0xFD, 3, 4, sbc(this, read(l(X))); misfire_a(X); pc += 2; )o(0xBD, 3, 4, lda(this, read(l(X))); misfire_a(X); pc += 2; )
	o(0x9D, 3, 5, sta(this, l(X)); pc += 2; )o(0x5E, 3, 7, lsr(this, l(X), read(l(X))); pc += 2; )
	o(0x1E, 3, 7, asl(this, l(X), read(l(X))); pc += 2; )o(0x7E, 3, 7, ror(this, l(X), read(l(X))); pc += 2; )
	o(0x3E, 3, 7, rol(this, l(X), read(l(X))); pc += 2; )o(0xFE, 3, 7, inc(this, l(X), read(l(X))); pc += 2; )
	o(0xDE, 3, 7, dec(this, l(X), read(l(X))); pc += 2; )o(0xBE, 3, 4, ldx(this, read(l(Y))); misfire_a(Y); pc += 2; )
	// unofficial nops
	o(0x0C, 3, 4, pc += 2; )case 0x44:case 0x64:o(0x04, 2, 3, pc++; )
	case 0xF4:case 0x34:case 0x54:case 0x74:case 0xD4:o(0x14, 2, 4, pc++; )
	case 0x82:case 0x89:case 0xC2:case 0xE2:o(0x80, 1, 2, pc++; )
	case 0x3C:case 0x5C:case 0x7C:case 0xDC:case 0xFC:o(0x1C, 3, 4, misfire_a(X); pc += 2; )
	// combined ops
	o(0xA3, 2, 6, lax(this, read(ind_x())); pc += 1; )o(0xA7, 2, 3, lax(this, read(read(pc++))); )
	o(0xAF, 3, 4, lax(this, read(read16(pc))); pc += 2; )o(0xB3, 2, 5, I; lax(this, read(n)); misfire_y(); pc++; )
	o(0xB7, 2, 4, lax(this, read(zpg_y())); pc++; )o(0xBF, 3, 4, lax(this, read(l(Y))); misfire_a(Y); pc += 2; )
	o(0x83, 2, 6, sax(this, ind_x()); pc++; )o(0x87, 2, 3, sax(this, read(pc++)); )
	o(0x8F, 3, 4, sax(this, read16(pc)); pc += 2; )o(0x97, 2, 4, sax(this, (read(pc++)+Y)&0xFF); )
	// rmw ops
	o(0xC3, 2, 8, dcp(this, ind_x(), read(ind_x())); pc += 1; )
	o(0xC7, 2, 5, dcp(this, read(pc), read(read(pc))); pc += 1; )
	o(0xCF, 2, 6, dcp(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0xD3, 2, 8, I; dcp(this, n, read(n)); pc++; )
	o(0xD7, 2, 6, dcp(this, (read(pc)+X)&0xFF, read((read(pc)+X)&0xFF)); pc += 1; )
	o(0xDB, 3, 7, dcp(this, l(Y), read(l(Y))); pc += 2; ) // no misfires
	o(0xDF, 3, 7, dcp(this, l(X), read(l(X))); pc += 2; ) // no misfires
	o(0xE3, 2, 8, isc(this, ind_x(), read(ind_x())); pc += 1; )
	o(0xE7, 2, 5, isc(this, read(pc), read(read(pc))); pc += 1; )
	o(0xEF, 2, 6, isc(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0xF3, 2, 8, I; isc(this, n, read(n)); pc++; )
	o(0xF7, 2, 6, isc(this, (read(pc)+X)&0xFF, read((read(pc)+X)&0xFF)); pc += 1; )
	o(0xFB, 3, 7, isc(this, l(Y), read(l(Y))); pc += 2; ) // no misfires
	o(0xFF, 3, 7, isc(this, l(X), read(l(X))); pc += 2; ) // no misfires
	o(0x03, 2, 8, slo(this, ind_x(), read(ind_x())); pc += 1; )
	o(0x07, 2, 5, slo(this, read(pc), read(read(pc))); pc += 1; )
	o(0x0F, 2, 6, slo(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0x13, 2, 8, I; slo(this, n, read(n)); pc++; )
	o(0x17, 2, 6, slo(this, (read(pc)+X)&0xFF, read((read(pc)+X)&0xFF)); pc += 1; )
	o(0x1B, 3, 7, slo(this, l(Y), read(l(Y))); pc += 2; ) // no misfires
	o(0x1F, 3, 7, slo(this, l(X), read(l(X))); pc += 2; ) // no misfires

	o(0x23, 2, 8, rla(this, ind_x(), read(ind_x())); pc += 1; )
	o(0x27, 2, 5, rla(this, read(pc), read(read(pc))); pc += 1; )
	o(0x2F, 2, 6, rla(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0x33, 2, 8, I; rla(this, n, read(n)); pc++; )
	o(0x37, 2, 6, rla(this, (read(pc)+X)&0xFF, read((read(pc)+X)&0xFF)); pc += 1; )
	o(0x3B, 3, 7, rla(this, l(Y), read(l(Y))); pc += 2; ) // no misfires
	o(0x3F, 3, 7, rla(this, l(X), read(l(X))); pc += 2; ) // no misfires

	o(0x43, 2, 8, sre(this, ind_x(), read(ind_x())); pc += 1; )
	o(0x47, 2, 5, sre(this, read(pc), read(read(pc))); pc += 1; )
	o(0x4F, 2, 6, sre(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0x53, 2, 8, I; sre(this, n, read(n)); pc++; )
	o(0x57, 2, 6, sre(this, (read(pc)+X)&0xFF, read((read(pc)+X)&0xFF)); pc += 1; )
	o(0x5B, 3, 7, sre(this, l(Y), read(l(Y))); pc += 2; ) // no misfires
	o(0x5F, 3, 7, sre(this, l(X), read(l(X))); pc += 2; ) // no misfires

	o(0x63, 2, 8, rra(this, ind_x(), read(ind_x())); pc += 1; )
	o(0x67, 2, 5, rra(this, read(pc), read(read(pc))); pc += 1; )
	o(0x6F, 2, 6, rra(this, read16(pc), read(read16(pc))); pc += 2; )
	o(0x73, 2, 8, I; rra(this, n, read(n)); pc++; )
	o(0x77, 2, 6, rra(this, (read(pc)+X)&0xFF, read((read(pc)+X)&0xFF)); pc += 1; )
	o(0x7B, 3, 7, rra(this, l(Y), read(l(Y))); pc += 2; ) // no misfires
	o(0x7F, 3, 7, rra(this, l(X), read(l(X))); pc += 2; ) // no misfires
	#undef I
	case 0x00: brk(); break;
	default:
		cpu_log.Report("[ERROR] Failed opcode 0x%02X\n", i.opcode);
		break;
	}

	cycles += _cycle;
	i.old_cycles = cycles - (cycles - i.old_cycles);
	return true;
}
// cycle each instruction at a time with the ppu 3x the speed
bool Cpu::cycle(void)
{
	// Clock the CPU
	clock(ins);

	// Clock the PPU 2x times the CPU
	for (int i = 0; i < _cycle * 3; i++) {
		ppu->clock();
		if (ppu->nmi) { ppu->nmi = 0, nmi(); }
	}
	return true;
}
