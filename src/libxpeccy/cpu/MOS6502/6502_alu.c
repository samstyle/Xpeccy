#include "6502.h"

// logic

unsigned char mos_ora(CPU* cpu, unsigned char va, unsigned char vb) {
	va |= vb;
	cpu->flgN = !!(va & 0x80);
	cpu->flgZ = !va;
	return va;
}

unsigned char mos_and(CPU* cpu, unsigned char va, unsigned char vb) {
	va &= vb;
	cpu->flgN = !!(va & 0x80);
	cpu->flgZ = !va;
	return va;
}

unsigned char mos_eor(CPU* cpu, unsigned char va, unsigned char vb) {
	va ^= vb;
	cpu->flgN = !!(va & 0x80);
	cpu->flgZ = !va;
	return va;
}

// math
// NOTE: ADD set C flag if overflow occured, clears if not
// NOTE: SUB & CMP clears C flag if overflow occured, set if not
// NOTE: BCD mode (flag MFD) doesn't affect NES
unsigned char mos_adc(CPU* cpu, unsigned char ac, unsigned char op) {
	cpu->flgTMP = cpu->flgC;
	cpu->tmpw = ac + op + cpu->flgTMP;
	cpu->flgZ = !(cpu->tmpw & 0xff);
	if (cpu->flgD && !cpu->flgND) {
		if ((ac & 0x0f) + (op & 0x0f) + cpu->flgTMP > 9) cpu->tmpw += 6;
		cpu->flgN = !!(cpu->tmpw & 0x80);
		cpu->flgV = !((ac ^ op) & 0x80) && ((ac ^ cpu->tmpw) & 0x80);
		if (cpu->tmpw > 0x99) cpu->tmpw += 0x60;
		cpu->flgC = !!(cpu->tmpw > 0x99);
	} else {
		cpu->flgN = !!(cpu->tmpw & 0x80);
		cpu->flgV = !((ac ^ op) & 0x80) && ((ac ^ cpu->tmpw) & 0x80);
		cpu->flgC = !!(cpu->tmpw & 0xff00);
	}
	return cpu->tmpw & 0xff;
}

unsigned char mos_sbc(CPU* cpu, unsigned char ac, unsigned char op) {
	cpu->flgTMP = !cpu->flgC;
	cpu->tmpw = ac - op - cpu->flgTMP;
	cpu->flgN = !!(cpu->tmpw & 0x80);
	cpu->flgZ = !(cpu->tmpw & 0xff);
	cpu->flgV = ((cpu->tmpw ^ ac) & 0x80) && ((ac ^ op) & 0x80);
	if (cpu->flgD && !cpu->flgND) {
		if (((ac & 0x0f) - cpu->flgTMP) < (op & 0x0f)) cpu->tmpw -= 6;
		if (cpu->tmpw > 0x99) cpu->tmpw -= 0x60;
	}
	cpu->flgC = !!(cpu->tmpw < 0x100);
	return cpu->tmpw & 0xff;
}

void mos_cmp(CPU* cpu, unsigned char ac, unsigned char op) {
	cpu->tmpw = ac - op;
	cpu->flgN = !!(cpu->tmpw & 0x80);
	cpu->flgC = !(cpu->tmpw & 0x100);
	cpu->flgZ = !cpu->ltw;
}

// rotation

// shift

// asl: fC <- V <- 0
unsigned char mos_asl(CPU* cpu, unsigned char op) {
	cpu->flgC = !!(op & 0x80);
	op = (op << 1) & 0xff;
	cpu->flgN = !!(op & 0x80);
	cpu->flgZ = !op;
	return op;
}

// lsr: 0 -> V -> fC
unsigned char mos_lsr(CPU* cpu, unsigned char op) {
	cpu->flgC = !!(op & 0x01);
	op = op >> 1;
	cpu->flgN = 0;
	cpu->flgZ = !op;
	return op;
}

// rol: fC <- V <- fC
unsigned char mos_rol(CPU* cpu, unsigned char op) {
	cpu->tmpw = op & 0xff;
	cpu->tmpw <<= 1;
	if (cpu->flgC) cpu->tmpw |= 1;
	cpu->flgC = !!(cpu->tmpw & 0x100);
	cpu->flgN = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	return cpu->ltw;
}

// ror: fC -> V -> fC
unsigned char mos_ror(CPU* cpu, unsigned char op) {
	cpu->tmpw = op & 0xff;
	if (cpu->flgC) cpu->tmpw |= 0x100;
	cpu->flgC = !!(cpu->tmpw & 1);
	cpu->tmpw >>= 1;
	cpu->flgN = !!(cpu->ltw & 0x80);
	cpu->flgZ = !cpu->ltw;
	return cpu->ltw;
}
