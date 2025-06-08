#include "z80.h"

// NOTE: don't use this for rla/rra/rlca/rrca - different flags affection

unsigned char z80_rl(CPU* cpu, unsigned char val) {
	cpu->tmp = val;
	val = (val << 1) | cpu->flgC;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	cpu->flgC = !!(cpu->tmp & 0x80);
	return val;
}

unsigned char z80_rlc(CPU* cpu, unsigned char val) {
	val = (val << 1) | (val >> 7);
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	cpu->flgC = (val & 1);
	return val;
}

unsigned char z80_rr(CPU* cpu, unsigned char val) {
	cpu->tmp = val;
	val = (val >> 1) | (cpu->flgC << 7);
	cpu->flgC = cpu->tmp & 1;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	return val;
}

unsigned char z80_rrc(CPU* cpu, unsigned char val) {
	cpu->flgC = val & 1;
	val = (val >> 1) | (val << 7);
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	return val;
}

unsigned char z80_sla(CPU* cpu, unsigned char val) {
	cpu->flgC = !!(val & 0x80);
	val <<= 1;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	return val;
}

unsigned char z80_sra(CPU* cpu, unsigned char val) {
	cpu->flgC = val & 1;
	val = (val & 0x80) | (val >> 1);
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	return val;
}

unsigned char z80_sll(CPU* cpu, unsigned char val) {
	cpu->flgC = !!(val & 0x80);
	val = (val << 1) | 0x01;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	return val;
}

unsigned char z80_srl(CPU* cpu, unsigned char val) {
	cpu->flgC = val & 1;
	val >>= 1;
	cpu->flgS = !!(val & 0x80);
	cpu->flgZ = !val;
	cpu->flgF5 = !!(val & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(val & 0x08);
	cpu->flgPV = parity(val);
	cpu->flgN = 0;
	return val;
}
