#include "z80.h"

// ALU
unsigned char z80_inc8(CPU* cpu, unsigned char v) {
	v++;
	cpu->flgS = !!(v & 0x80);
	cpu->flgZ = !v;
	cpu->flgF5 = !!(v & 0x20);
	cpu->flgH = !(v & 0x0f);
	cpu->flgF3 = !!(v & 0x08);
	cpu->flgN = 0;
	cpu->flgPV = !!(v == 0x80);
	return v;
}

unsigned char z80_dec8(CPU* cpu, unsigned char v) {
	cpu->flgH = !(v & 0x0f);
	v--;
	cpu->flgS = !!(v & 0x80);
	cpu->flgZ = !v;
	cpu->flgF5 = !!(v & 0x20);
	cpu->flgF3 = !!(v & 0x08);
	cpu->flgN = 1;
	cpu->flgPV = !!(v == 0x7f);
	return v;
}

unsigned char z80_add8(CPU* cpu, unsigned char v, unsigned char c) {
	cpu->tmpw = cpu->regA + v + c;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->flgS = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	cpu->flgF5 = !!(cpu->ltw & 0x20);
	cpu->flgH = !!FHaddTab[cpu->tmp & 7];
	cpu->flgF3 = !!(cpu->ltw & 0x08);
	cpu->flgPV = !!FVaddTab[(cpu->tmp >> 4) & 7];
	cpu->flgN = 0;
	cpu->flgC = !!cpu->htw;
	return cpu->ltw;
}

// F3,F5 taked from result
unsigned char z80_sub8(CPU* cpu, unsigned char v, unsigned char c) {
	cpu->tmpw = cpu->regA - v - c;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->flgS = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	cpu->flgF5 = !!(cpu->ltw & 0x20);
	cpu->flgH = !!FHsubTab[cpu->tmp & 7];
	cpu->flgF3 = !!(cpu->ltw & 0x08);
	cpu->flgPV = !!FVsubTab[(cpu->tmp >> 4) & 7];
	cpu->flgN = 1;
	cpu->flgC = !!cpu->htw;
	return cpu->ltw;
}

// F3,F5 taked from operand
void z80_cp8(CPU* cpu, unsigned char v) {
	cpu->tmpw = cpu->regA - v;
	cpu->tmp = ((cpu->regA & 0x88) >> 3) | ((v & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);
	cpu->flgS = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	cpu->flgF5 = !!(v & 0x20);
	cpu->flgH = !!FHsubTab[cpu->tmp & 7];
	cpu->flgF3 = !!(v & 0x08);
	cpu->flgPV = !!FVsubTab[(cpu->tmp >> 4) & 7];
	cpu->flgN = 1;
	cpu->flgC = !!cpu->htw;
}

// S,Z,PV flags affected only by ADC
unsigned short z80_add16(CPU* cpu, unsigned short v1, unsigned short v2) {
	cpu->regWZ = v1;
	cpu->tmpi = v1 + v2;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	v1 = cpu->tmpi & 0xffff;
	cpu->flgF5 = !!(v1 & 0x2000);
	cpu->flgH = !!FHaddTab[cpu->tmp & 7];
	cpu->flgF3 = !!(v1 & 0x0800);
	cpu->flgN = 0;
	cpu->flgC = !!(cpu->tmpi & ~0xffff);
	cpu->regWZ++;
	return v1;
}

unsigned short z80_adc16(CPU* cpu, unsigned short v1, unsigned short v2, unsigned char c) {
	cpu->regWZ = v1;
	cpu->tmpi = v1 + v2 + c;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	v1 = cpu->tmpi & 0xffff;
	cpu->flgS = !!(v1 & 0x8000);
	cpu->flgZ = !v1;
	cpu->flgF5 = !!(v1 & 0x2000);
	cpu->flgH = !!FHaddTab[cpu->tmp & 7];
	cpu->flgF3 = !!(v1 & 0x0800);
	cpu->flgPV = !!FVaddTab[cpu->tmp >> 4];
	cpu->flgN = 0;
	cpu->flgC = !!(cpu->tmpi & ~0xffff);
	cpu->regWZ++;
	return v1;
}

unsigned short z80_sub16(CPU* cpu, unsigned short v1, unsigned short v2, unsigned char c) {
	cpu->tmpi = v1 - v2 - c;
	cpu->tmp = ((v1 & 0x8800) >> 11) | ((v2 & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);
	cpu->regWZ = v1 + 1;
	v1 = cpu->tmpi & 0xffff;
	cpu->flgS = !!(v1 & 0x8000);
	cpu->flgZ = !v1;
	cpu->flgF5 = !!(v1 & 0x2000);
	cpu->flgH = !!FHsubTab[cpu->tmp & 7];
	cpu->flgF3 = !!(v1 & 0x0800);
	cpu->flgPV = !!FVsubTab[cpu->tmp >> 4];
	cpu->flgN = 1;
	cpu->flgC = !!(cpu->tmpi & ~0xffff);
	return v1;
}

// logic

void z80_and8(CPU* cpu, unsigned char v) {
	cpu->regA &= v;
	cpu->flgS = !!(cpu->regA & 0x80);
	cpu->flgZ = !cpu->regA;
	cpu->flgF5 = !!(cpu->regA & 0x20);
	cpu->flgH = 1;
	cpu->flgF3 = !!(cpu->regA & 0x08);
	cpu->flgPV = parity(cpu->regA);
	cpu->flgN = 0;
	cpu->flgC = 0;
}

void z80_or8(CPU* cpu, unsigned char v) {
	cpu->regA |= v;
	cpu->flgS = !!(cpu->regA & 0x80);
	cpu->flgZ = !cpu->regA;
	cpu->flgF5 = !!(cpu->regA & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(cpu->regA & 0x08);
	cpu->flgPV = parity(cpu->regA);
	cpu->flgN = 0;
	cpu->flgC = 0;
}

void z80_xor8(CPU* cpu, unsigned char v) {
	cpu->regA ^= v;
	cpu->flgS = !!(cpu->regA & 0x80);
	cpu->flgZ = !cpu->regA;
	cpu->flgF5 = !!(cpu->regA & 0x20);
	cpu->flgH = 0;
	cpu->flgF3 = !!(cpu->regA & 0x08);
	cpu->flgPV = parity(cpu->regA);
	cpu->flgN = 0;
	cpu->flgC = 0;
}

