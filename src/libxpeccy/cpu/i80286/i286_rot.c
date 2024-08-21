#include "i80286.h"

// rol: CF<-b7...b0<-b7
unsigned char i286_rol8(CPU* cpu, unsigned char p) {
	//cpu->f &= ~(I286_FC | I286_FO);
	p = (p << 1) | ((p & 0x80) ? 1 : 0);
	cpu->fx.c = p & 1;
	cpu->fx.o = !!(!cpu->fx.c != !(p & 0x80));
	return p;
}

unsigned short i286_rol16(CPU* cpu, unsigned short p) {
	//cpu->f &= ~(I286_FC | I286_FO);
	p = (p << 1) | ((p & 0x8000) ? 1 : 0);
	cpu->fx.c = p & 1;
	cpu->fx.o = (!cpu->fx.c != !(p & 0x8000));
	return p;
}

// ror: b0->b7...b0->CF
unsigned char i286_ror8(CPU* cpu, unsigned char p) {
	//cpu->f &= ~(I286_FC | I286_FO);
	p = (p >> 1) | ((p & 1) ? 0x80 : 0);
	cpu->fx.c = !!(p & 0x80);
	cpu->fx.o = !!(!(p & 0x80) != !(p & 0x40));
	return p;
}

unsigned short i286_ror16(CPU* cpu, unsigned short p) {
	//cpu->f &= ~(I286_FC | I286_FO);
	p = (p >> 1) | ((p & 1) ? 0x8000 : 0);
	cpu->fx.c = !!(p & 0x8000);
	cpu->fx.o = !!(!(p & 0x8000) != !(p & 0x4000));
	return p;
}

// rcl: CF<-b7..b0<-CF
unsigned char i286_rcl8(CPU* cpu, unsigned char p) {
	cpu->tmp = cpu->fx.c;
	//cpu->f &= ~(I286_FC | I286_FO);
	cpu->fx.c = !!(p & 0x80);
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	cpu->fx.o = !!(!cpu->fx.c != !(p & 0x80));
	return p;
}

unsigned short i286_rcl16(CPU* cpu, unsigned short p) {
	cpu->tmp = cpu->fx.c;
	//cpu->f &= ~(I286_FC | I286_FO);
	cpu->fx.c = !!(p & 0x8000);
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	cpu->fx.o = !!(!cpu->fx.c != !(p & 0x8000));
	return p;
}

// rcr: CF->b7..b0->CF
unsigned char i286_rcr8(CPU* cpu, unsigned char p) {
	cpu->tmp = cpu->fx.c;
	//cpu->f &= ~(I286_FC | I286_FO);
	cpu->fx.c = p & 1;
	p >>= 1;
	if (cpu->tmp) p |= 0x80;
	cpu->fx.o = !!(!(p & 0x80) != !(p & 0x40));
	return p;
}

unsigned short i286_rcr16(CPU* cpu, unsigned short p) {
	cpu->tmp = cpu->fx.c;
	//cpu->f &= ~(I286_FC | I286_FO);
	cpu->fx.c = p & 1;
	p >>= 1;
	if (cpu->tmp) p |= 0x8000;
	cpu->fx.o = !!(!(p & 0x8000) != !(p & 0x4000));
	return p;
}

// sal: CF<-b7..b0<-0
unsigned char i286_sal8(CPU* cpu, unsigned char p) {
	//cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	cpu->fx.c = !!(p & 0x80);
	p <<= 1;
	cpu->fx.o = (!cpu->fx.c != !(p & 0x80));
	cpu->fx.z = !p;
	cpu->fx.p = parity(p & 0xff);
	cpu->fx.s = !!(p & 0x80);
	return p;
}

unsigned short i286_sal16(CPU* cpu, unsigned short p) {
	//cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	cpu->fx.c = !!(p & 0x8000);
	p <<= 1;
	cpu->fx.o = !!(!cpu->fx.c != !(p & 0x8000));
	cpu->fx.z = !p;
	cpu->fx.p = parity(p & 0xff);
	cpu->fx.s = !!(p & 0x8000);
	return p;
}

// shr 0->b7..b0->CF
unsigned char i286_shr8(CPU* cpu, unsigned char p) {
	//cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	cpu->fx.c = p & 1;
	cpu->fx.o = !!(p & 0x80);
	p >>= 1;
	cpu->fx.z = !p;
	cpu->fx.p = parity(p & 0xff);
	cpu->fx.s = !!(p & 0x80);
	return p;
}

unsigned short i286_shr16(CPU* cpu, unsigned short p) {
	//cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	cpu->fx.c = p & 1;
	cpu->fx.o = !!(p & 0x8000);
	p >>= 1;
	cpu->fx.z = !p;
	cpu->fx.p = parity(p & 0xff);
	cpu->fx.s = !!(p & 0x8000);
	return p;
}

// sar b7->b7..b0->CF
unsigned char i286_sar8(CPU* cpu, unsigned char p) {
	//cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	cpu->fx.c = p & 1;
	p = (p >> 1) | (p & 0x80);
	cpu->fx.z = !p;
	cpu->fx.p = parity(p & 0xff);
	cpu->fx.s = !!(p & 0x80);
	cpu->fx.o = 0;
	return p;
}

unsigned short i286_sar16(CPU* cpu, unsigned short p) {
	//cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	cpu->fx.c = p & 1;
	p = (p >> 1) | (p & 0x8000);
	cpu->fx.z = !p;
	cpu->fx.p = parity(p & 0xff);
	cpu->fx.s = !!(p & 0x8000);
	cpu->fx.o = 0;
	return p;
}
