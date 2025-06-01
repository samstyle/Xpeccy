#include "z80.h"

// ALU
unsigned char z80_inc8(CPU* cpu, unsigned char v) {
	v++;
	cpu->f.s = !!(v & 0x80);
	cpu->f.z = !v;
	cpu->f.f5 = !!(v & 0x20);
	cpu->f.h = !(v & 0x0f);
	cpu->f.f3 = !!(v & 0x08);
	cpu->f.n = 0;
	cpu->f.pv = !!(v == 0x80);
	return v;
}

unsigned char z80_dec8(CPU* cpu, unsigned char v) {
	cpu->f.h = !(v & 0x0f);
	v--;
	cpu->f.s = !!(v & 0x80);
	cpu->f.z = !v;
	cpu->f.f5 = !!(v & 0x20);
	cpu->f.f3 = !!(v & 0x08);
	cpu->f.n = 1;
	cpu->f.pv = !!(v == 0x7f);
	return v;
}

unsigned char z80_add8(CPU* cpu, unsigned char v, unsigned char c) {
	cpu->tmpw = cpu->regA + v + c;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->f.s = !!(cpu->ltw & 0x80);
	cpu->f.z = !cpu->ltw;
	cpu->f.f5 = !!(cpu->ltw & 0x20);
	cpu->f.h = !!FHaddTab[cpu->tmp & 7];
	cpu->f.f3 = !!(cpu->ltw & 0x08);
	cpu->f.pv = !!FVaddTab[(cpu->tmp >> 4) & 7];
	cpu->f.n = 0;
	cpu->f.c = !!cpu->htw;
	return cpu->ltw;
}

// F3,F5 taked from result
unsigned char z80_sub8(CPU* cpu, unsigned char v, unsigned char c) {
	cpu->tmpw = cpu->regA - v - c;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->f.s = !!(cpu->ltw & 0x80);
	cpu->f.z = !cpu->ltw;
	cpu->f.f5 = !!(cpu->ltw & 0x20);
	cpu->f.h = !!FHsubTab[cpu->tmp & 7];
	cpu->f.f3 = !!(cpu->ltw & 0x08);
	cpu->f.pv = !!FVsubTab[(cpu->tmp >> 4) & 7];
	cpu->f.n = 1;
	cpu->f.c = !!cpu->htw;
	return cpu->ltw;
}

// F3,F5 taked from operand
void z80_cp8(CPU* cpu, unsigned char v) {
	cpu->tmpw = cpu->regA - v;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->f.s = !!(cpu->ltw & 0x80);
	cpu->f.z = !cpu->ltw;
	cpu->f.f5 = !!(v & 0x20);
	cpu->f.h = !!FHsubTab[cpu->tmp & 7];
	cpu->f.f3 = !!(v & 0x08);
	cpu->f.pv = !!FVsubTab[(cpu->tmp >> 4) & 7];
	cpu->f.n = 1;
	cpu->f.c = !!cpu->htw;
}

// S,Z,PV flags affected only by ADC
unsigned short z80_add16(CPU* cpu, unsigned short v1, unsigned short v2) {
	cpu->regWZ = v1;
	cpu->tmpi = v1 + v2;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	v1 = cpu->tmpi & 0xffff;
	cpu->f.f5 = !!(v1 & 0x2000);
	cpu->f.h = !!FHaddTab[cpu->tmp & 7];
	cpu->f.f3 = !!(v1 & 0x0800);
	cpu->f.n = 0;
	cpu->f.c = !!(cpu->tmpi & ~0xffff);
	cpu->regWZ++;
	return v1;
}

unsigned short z80_adc16(CPU* cpu, unsigned short v1, unsigned short v2, unsigned char c) {
	cpu->regWZ = v1;
	cpu->tmpi = v1 + v2 + c;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	v1 = cpu->tmpi & 0xffff;
	cpu->f.s = !!(v1 & 0x8000);
	cpu->f.z = !v1;
	cpu->f.f5 = !!(v1 & 0x2000);
	cpu->f.h = !!FHaddTab[cpu->tmp & 7];
	cpu->f.f3 = !!(v1 & 0x0800);
	cpu->f.pv = !!FVaddTab[cpu->tmp >> 4];
	cpu->f.n = 0;
	cpu->f.c = !!(cpu->tmpi & ~0xffff);
	cpu->regWZ++;
	return v1;
}

unsigned short z80_sub16(CPU* cpu, unsigned short v1, unsigned short v2, unsigned char c) {
	cpu->tmpi = v1 - v2 - c;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	cpu->regWZ = v1 + 1;
	v1 = cpu->tmpi & 0xffff;
	cpu->f.s = !!(v1 & 0x8000);
	cpu->f.z = !v1;
	cpu->f.f5 = !!(v1 & 0x2000);
	cpu->f.h = !!FHsubTab[cpu->tmp & 7];
	cpu->f.f3 = !!(v1 & 0x0800);
	cpu->f.pv = !!FVsubTab[cpu->tmp >> 4];
	cpu->f.n = 1;
	cpu->f.c = !!(cpu->tmpi & ~0xffff);
	return v1;
}

// logic

void z80_and8(CPU* cpu, unsigned char v) {
	cpu->regA &= v;
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 1;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = parity(cpu->regA);
	cpu->f.n = 0;
	cpu->f.c = 0;
}

void z80_or8(CPU* cpu, unsigned char v) {
	cpu->regA |= v;
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = parity(cpu->regA);
	cpu->f.n = 0;
	cpu->f.c = 0;
}

void z80_xor8(CPU* cpu, unsigned char v) {
	cpu->regA ^= v;
	cpu->f.s = !!(cpu->regA & 0x80);
	cpu->f.z = !cpu->regA;
	cpu->f.f5 = !!(cpu->regA & 0x20);
	cpu->f.h = 0;
	cpu->f.f3 = !!(cpu->regA & 0x08);
	cpu->f.pv = parity(cpu->regA);
	cpu->f.n = 0;
	cpu->f.c = 0;
}

