#include "z80.h"

// ALU
unsigned char z80_inc8(CPU* cpu, unsigned char v) {
	v++;
	cpu->fz.s = !!(v & 0x80);
	cpu->fz.z = !v;
	cpu->fz.f5 = !!(v & 0x20);
	cpu->fz.h = !(v & 0x0f);
	cpu->fz.f3 = !!(v & 0x08);
	cpu->fz.n = 0;
	cpu->fz.pv = !!(v == 0x80);
	return v;
}

unsigned char z80_dec8(CPU* cpu, unsigned char v) {
	cpu->fz.h = !(v & 0x0f);
	v--;
	cpu->fz.s = !!(v & 0x80);
	cpu->fz.z = !v;
	cpu->fz.f5 = !!(v & 0x20);
	cpu->fz.f3 = !!(v & 0x08);
	cpu->fz.n = 1;
	cpu->fz.pv = !!(v == 0x7f);
	return v;
}

unsigned char z80_add8(CPU* cpu, unsigned char v, unsigned char c) {
	cpu->tmpw = cpu->regA + v + c;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->fz.s = !!(cpu->ltw & 0x80);
	cpu->fz.z = !cpu->ltw;
	cpu->fz.f5 = !!(cpu->ltw & 0x20);
	cpu->fz.h = !!FHaddTab[cpu->tmp & 7];
	cpu->fz.f3 = !!(cpu->ltw & 0x08);
	cpu->fz.pv = !!FVaddTab[(cpu->tmp >> 4) & 7];
	cpu->fz.n = 0;
	cpu->fz.c = !!cpu->htw;
	return cpu->ltw;
}

// F3,F5 taked from result
unsigned char z80_sub8(CPU* cpu, unsigned char v, unsigned char c) {
	cpu->tmpw = cpu->regA - v - c;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->fz.s = !!(cpu->ltw & 0x80);
	cpu->fz.z = !cpu->ltw;
	cpu->fz.f5 = !!(cpu->ltw & 0x20);
	cpu->fz.h = !!FHsubTab[cpu->tmp & 7];
	cpu->fz.f3 = !!(cpu->ltw & 0x08);
	cpu->fz.pv = !!FVsubTab[(cpu->tmp >> 4) & 7];
	cpu->fz.n = 1;
	cpu->fz.c = !!cpu->htw;
	return cpu->ltw;
}

// F3,F5 taked from operand
void z80_cp8(CPU* cpu, unsigned char v) {
	cpu->tmpw = cpu->regA - v;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->fz.s = !!(cpu->ltw & 0x80);
	cpu->fz.z = !cpu->ltw;
	cpu->fz.f5 = !!(v & 0x20);
	cpu->fz.h = !!FHsubTab[cpu->tmp & 7];
	cpu->fz.f3 = !!(v & 0x08);
	cpu->fz.pv = !!FVsubTab[(cpu->tmp >> 4) & 7];
	cpu->fz.n = 1;
	cpu->fz.c = !!cpu->htw;
}

// S,Z,PV flags affected only by ADC
unsigned short z80_add16(CPU* cpu, unsigned short v1, unsigned short v2) {
	cpu->regWZ = v1;
	cpu->tmpi = v1 + v2;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	v1 = cpu->tmpi & 0xffff;
	cpu->fz.f5 = !!(v1 & 0x2000);
	cpu->fz.h = !!FHaddTab[cpu->tmp & 7];
	cpu->fz.f3 = !!(v1 & 0x0800);
	cpu->fz.n = 0;
	cpu->fz.c = !!(cpu->tmpi & ~0xffff);
	cpu->regWZ++;
	return v1;
}

unsigned short z80_adc16(CPU* cpu, unsigned short v1, unsigned short v2, unsigned char c) {
	cpu->regWZ = v1;
	cpu->tmpi = v1 + v2 + c;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	v1 = cpu->tmpi & 0xffff;
	cpu->fz.s = !!(v1 & 0x8000);
	cpu->fz.z = !v1;
	cpu->fz.f5 = !!(v1 & 0x2000);
	cpu->fz.h = !!FHaddTab[cpu->tmp & 7];
	cpu->fz.f3 = !!(v1 & 0x0800);
	cpu->fz.pv = !!FVaddTab[cpu->tmp >> 4];
	cpu->fz.n = 0;
	cpu->fz.c = !!(cpu->tmpi & ~0xffff);
	cpu->regWZ++;
	return v1;
}

unsigned short z80_sub16(CPU* cpu, unsigned short v1, unsigned short v2, unsigned char c) {
	cpu->tmpi = v1 - v2 - c;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	cpu->regWZ = v1 + 1;
	v1 = cpu->tmpi & 0xffff;
	cpu->fz.s = !!(v1 & 0x8000);
	cpu->fz.z = !v1;
	cpu->fz.f5 = !!(v1 & 0x2000);
	cpu->fz.h = !!FHsubTab[cpu->tmp & 7];
	cpu->fz.f3 = !!(v1 & 0x0800);
	cpu->fz.pv = !!FVsubTab[cpu->tmp >> 4];
	cpu->fz.n = 1;
	cpu->fz.c = !!(cpu->tmpi & ~0xffff);
	return v1;
}

// logic

void z80_and8(CPU* cpu, unsigned char v) {
	cpu->regA &= v;
	cpu->fz.s = !!(cpu->regA & 0x80);
	cpu->fz.z = !cpu->regA;
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 1;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.pv = parity(cpu->regA);
	cpu->fz.n = 0;
	cpu->fz.c = 0;
}

void z80_or8(CPU* cpu, unsigned char v) {
	cpu->regA |= v;
	cpu->fz.s = !!(cpu->regA & 0x80);
	cpu->fz.z = !cpu->regA;
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.pv = parity(cpu->regA);
	cpu->fz.n = 0;
	cpu->fz.c = 0;
}

void z80_xor8(CPU* cpu, unsigned char v) {
	cpu->regA ^= v;
	cpu->fz.s = !!(cpu->regA & 0x80);
	cpu->fz.z = !cpu->regA;
	cpu->fz.f5 = !!(cpu->regA & 0x20);
	cpu->fz.h = 0;
	cpu->fz.f3 = !!(cpu->regA & 0x08);
	cpu->fz.pv = parity(cpu->regA);
	cpu->fz.n = 0;
	cpu->fz.c = 0;
}

