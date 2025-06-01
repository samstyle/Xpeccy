#include "z80.h"

// NOTE: don't use this for rla/rra/rlca/rrca - different flags affection

unsigned char z80_rl(CPU* cpu, unsigned char val) {
	cpu->tmp = val;
	val = (val << 1) | cpu->f.c;
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	cpu->f.c = !!(cpu->tmp & 0x80);
	return val;
}

unsigned char z80_rlc(CPU* cpu, unsigned char val) {
	val = (val << 1) | (val >> 7);
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	cpu->f.c = (val & 1);
	return val;
}

unsigned char z80_rr(CPU* cpu, unsigned char val) {
	cpu->tmp = val;
	val = (val >> 1) | (cpu->f.c << 7);
	cpu->f.c = cpu->tmp & 1;
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	return val;
}

unsigned char z80_rrc(CPU* cpu, unsigned char val) {
	cpu->f.c = val & 1;
	val = (val >> 1) | (val << 7);
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	return val;
}

unsigned char z80_sla(CPU* cpu, unsigned char val) {
	cpu->f.c = !!(val & 0x80);
	val <<= 1;
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	return val;
}

unsigned char z80_sra(CPU* cpu, unsigned char val) {
	cpu->f.c = val & 1;
	val = (val & 0x80) | (val >> 1);
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	return val;
}

unsigned char z80_sll(CPU* cpu, unsigned char val) {
	cpu->f.c = !!(val & 0x80);
	val = (val << 1) | 0x01;
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	return val;
}

unsigned char z80_srl(CPU* cpu, unsigned char val) {
	cpu->f.c = val & 1;
	val >>= 1;
	cpu->f.s = !!(val & 0x80);
	cpu->f.z = !val;
	cpu->f.f5 = !!(val & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(val & 0x08);
	cpu->f.pv = parity(val);
	cpu->f.n = 0;
	return val;
}
