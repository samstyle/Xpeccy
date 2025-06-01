#include "i80286.h"

static const int i286_add_FO[8] = {0, 0, 0, 1, 1, 0, 0, 0};

unsigned char i286_add8(CPU* cpu, unsigned char p1, unsigned char p2, int cf) {
//	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xff;
	int r2 = (p2 & 0xff) + (cf ? 1 : 0);
	int res = r1 + r2;
	cpu->f.o = !!(((p1 ^ p2 ^ 0x80) & (res ^ p2)) & 0x80);
	cpu->f.s = !!(res & 0x80);
	cpu->f.z = !(res & 0xff);
	cpu->f.a = !!((p1 & 15) + (p2 & 15) > 15);
	cpu->f.p = parity(res & 0xff);
	cpu->f.c = !!(res & ~0xff);
	return res & 0xff;
}

unsigned short i286_add16(CPU* cpu, unsigned short p1, unsigned short p2, int cf) {
//	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xffff;
	int r2 = (p2 & 0xffff) + (cf ? 1 : 0);
	int res = r1 + r2;
	cpu->f.o = !!(((p1 ^ p2 ^ 0x8000) & (res ^ p2)) & 0x8000);
	cpu->f.s = !!(res & 0x8000);
	cpu->f.z = !(res & 0xffff);
	cpu->f.a = ((p1 & 0xfff) + (p2 & 0xfff) > 0xfff);
	cpu->f.p = parity(res & 0xff);
	cpu->f.c = !!(res & ~0xffff);
	return res & 0xffff;
}

// static const int i286_sub_FO[8] = {0, 1, 0, 0, 0, 0, 1, 0};

unsigned char i286_sub8(CPU* cpu, unsigned char p1, unsigned char p2, int cf) {
//	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xff;
	int r2 = (p2 & 0xff) + (cf ? 1 : 0);
	int res = r1 - r2;
	cpu->f.o = !!(((p1 ^ p2) & (p1 ^ res)) & 0x80);
	cpu->f.s = !!(res & 0x80);
	cpu->f.z = !(res & 0xff);
	cpu->f.p = parity(res & 0xff);
	cpu->f.c = !!(res & ~0xff);
	cpu->f.a = !!((p1 & 0x0f) < (p2 & 0x0f));
	return res & 0xff;
}

unsigned short i286_sub16(CPU* cpu, unsigned short p1, unsigned short p2, int cf) {
//	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xffff;
	int r2 = (p2 & 0xffff) + (cf ? 1 : 0);
	int res = r1 - r2;
	cpu->f.o = !!(((p1 ^ p2) & (p1 ^ res)) & 0x8000);
	cpu->f.s = !!(res & 0x8000);
	cpu->f.z = !(res & 0xffff);
	cpu->f.p = parity(res & 0xff);
	cpu->f.c = !!(res & ~0xffff);
	cpu->f.a = !!((p1 & 0x0fff) < (p2 & 0x0fff));
	return res & 0xffff;
}

// mul
int i286_smul(CPU* cpu, signed short p1, signed short p2) {
	int res = p1 * p2;
//	cpu->f &= ~(I286_FO | I286_FC);
	cpu->f.c = !!((p1 & 0x7fff) * (p2 & 0x7fff) > 0xffff);
	cpu->tmp = ((p1 & 0x8000) >> 15) | ((p2 & 0x8000) >> 14) | ((res & 0x8000) >> 13);
	cpu->f.o = !!i286_add_FO[cpu->tmp & 7];
	return res;
}

// logic
unsigned char i286_and8(CPU* cpu, unsigned char p1, unsigned char p2) {
//	cpu->f &= ~(I286_FO | I286_FS | I286_FP | I286_FZ | I286_FC);
	p1 &= p2;
	cpu->f.s = !!(p1 & 0x80);
	cpu->f.z = !p1;
	cpu->f.p = parity(p1 & 0xff);
	cpu->f.o = 0;
	cpu->f.c = 0;
	return p1;
}

unsigned short i286_and16(CPU* cpu, unsigned short p1, unsigned short p2) {
//	cpu->f &= ~(I286_FO | I286_FS | I286_FP | I286_FZ | I286_FC);
	p1 &= p2;
	cpu->f.s = !!(p1 & 0x8000);
	cpu->f.z = !p1;
	cpu->f.p = parity(p1 & 0xff);
	cpu->f.o = 0;
	cpu->f.c = 0;
	return p1;
}

unsigned char i286_or8(CPU* cpu, unsigned char p1, unsigned char p2) {
	unsigned char res =  p1 | p2;
//	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	cpu->f.s = !!(res & 0x80);
	cpu->f.z = !res;
	cpu->f.p = parity(res & 0xff);
	cpu->f.o = 0;
	cpu->f.c = 0;
	return res;
}

unsigned short i286_or16(CPU* cpu, unsigned short p1, unsigned short p2) {
	unsigned short res =  p1 | p2;
	//cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	cpu->f.s = !!(res & 0x8000);
	cpu->f.z = !res;
	cpu->f.p = parity(res & 0xff);
	cpu->f.o = 0;
	cpu->f.c = 0;
	return res;
}

unsigned char i286_xor8(CPU* cpu, unsigned char p1, unsigned char p2) {
	p1 ^= p2;
//	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	cpu->f.s = !!(p1 & 0x80);
	cpu->f.z = !p1;
	cpu->f.p = parity(p1 & 0xff);
	cpu->f.o = 0;
	cpu->f.c = 0;
	return p1;
}

unsigned short i286_xor16(CPU* cpu, unsigned short p1, unsigned short p2) {
	p1 ^= p2;
	// cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	cpu->f.s = !!(p1 & 0x8000);
	cpu->f.z = !p1;
	cpu->f.p = parity(p1 & 0xff);
	cpu->f.o = 0;
	cpu->f.c = 0;
	return p1;
}
