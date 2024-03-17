#include "i80286.h"

// rol: FC<-b7...b0<-b7
unsigned char i286_rol8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p << 1) | ((p & 0x80) ? 1 : 0);
	if (p & 1) cpu->f |= I286_FC;
	if (!(cpu->f & I286_FC) != !(p & 0x80)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_rol16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p << 1) | ((p & 0x8000) ? 1 : 0);
	if (p & 1) cpu->f |= I286_FC;
	if (!(cpu->f & I286_FC) != !(p & 0x8000)) cpu->f |= I286_FO;
	return p;
}

// ror: b0->b7...b0->CF
unsigned char i286_ror8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p >> 1) | ((p & 1) ? 0x80 : 0);
	if (p & 0x80) cpu->f |= I286_FC;
	if (!(p & 0x80) != !(p & 0x40)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_ror16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p >> 1) | ((p & 1) ? 0x8000 : 0);
	if (p & 0x8000) cpu->f |= I286_FC;
	if (!(p & 0x8000) != !(p & 0x4000)) cpu->f |= I286_FO;
	return p;
}

// rcl: CF<-b7..b0<-CF
unsigned char i286_rcl8(CPU* cpu, unsigned char p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 0x80) cpu->f |= I286_FC;
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	if (!(cpu->f & I286_FC) != !(p & 0x80)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_rcl16(CPU* cpu, unsigned short p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 0x8000) cpu->f |= I286_FC;
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	if (!(cpu->f & I286_FC) != !(p & 0x8000)) cpu->f |= I286_FO;
	return p;
}

// rcr: CF->b7..b0->CF
unsigned char i286_rcr8(CPU* cpu, unsigned char p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 1) cpu->f |= I286_FC;
	p >>= 1;
	if (cpu->tmp) p |= 0x80;
	if (!(p & 0x80) != !(p & 0x40)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_rcr16(CPU* cpu, unsigned short p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 1) cpu->f |= I286_FC;
	p >>= 1;
	if (cpu->tmp) p |= 0x8000;
	if (!(p & 0x8000) != !(p & 0x4000)) cpu->f |= I286_FO;
	return p;
}

// sal: CF<-b7..b0<-0
unsigned char i286_sal8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 0x80) cpu->f |= I286_FC;
	p <<= 1;
	if (!(cpu->f & I286_FC) != !(p & 0x80)) cpu->f |= I286_FO;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x80) cpu->f |= I286_FS;
	return p;
}

unsigned short i286_sal16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 0x8000) cpu->f |= I286_FC;
	p <<= 1;
	if (!(cpu->f & I286_FC) != !(p & 0x8000)) cpu->f |= I286_FO;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x8000) cpu->f |= I286_FS;
	return p;
}

// shr 0->b7..b0->CF
unsigned char i286_shr8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	if (p & 0x80) cpu->f |= I286_FO;
	p >>= 1;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x80) cpu->f |= I286_FS;
	return p;
}

unsigned short i286_shr16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	if (p & 0x8000) cpu->f |= I286_FO;
	p >>= 1;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x8000) cpu->f |= I286_FS;
	return p;
}

// sar b7->b7..b0->CF
unsigned char i286_sar8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	p = (p >> 1) | (p & 0x80);
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x80) cpu->f |= I286_FS;
	return p;
}

unsigned short i286_sar16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	p = (p >> 1) | (p & 0x8000);
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x8000) cpu->f |= I286_FS;
	return p;
}
