#pragma once

#include "z80.h"

extern const unsigned char daaTab[0x1000];
extern const unsigned char FHaddTab[8];
extern const unsigned char FHsubTab[8];
extern const unsigned char FVaddTab[8];
extern const unsigned char FVsubTab[8];

// bit
// z = bit value, p/v = z, s = (val & (1 << bit) & 0x80)

// cpu->f = (cpu->f & Z80_FC) | Z80_FH | (sz53pTab[val & (0x01 << bit)] & ~(Z80_F5 | Z80_F3)) | ((val & (1 << bit)) & (Z80_F5 | Z80_F3));}
#define BIT(bit,val) {\
	cpu->tmp = val & (1 << bit);\
	cpu->flgS = !!(cpu->tmp & 0x80);\
	cpu->flgZ = !cpu->tmp;\
	cpu->flgF5 = !!(cpu->tmp & 0x20);\
	cpu->flgH = 1;\
	cpu->flgF3 = !!(cpu->tmp & 0x08);\
	cpu->flgPV = cpu->flgZ;\
	cpu->flgN = 0;\
}

// cpu->f = (cpu->f & Z80_FC) | Z80_FH | (sz53pTab[val & (1 << bit)] & ~(Z80_F5 | Z80_F3)) | (cpu->hptr & (Z80_F5 | Z80_F3));}
#define BITM(bit,val) {\
	cpu->tmp = val & (1 << bit);\
	cpu->flgS = !!(cpu->tmp & 0x80);\
	cpu->flgZ = !cpu->tmp;\
	cpu->flgF5 = !!(cpu->regWZh & 0x20);\
	cpu->flgH = 1;\
	cpu->flgF3 = !!(cpu->regWZh & 0x08);\
	cpu->flgPV = cpu->flgZ;\
	cpu->flgN = 0;\
}

//#define SET(bit,val) {val |= (1 << bit);}
//#define RES(bit,val) {val &= ~(1 << bit);}

// extend

#define SWAP(rp1,rp2) {cpu->tmpw = rp1; rp1 = rp2; rp2 = cpu->tmpw;}		// swap 16bit regs

#define	RDSHIFT(base) {\
	cpu->tmp = z80_mrd(cpu, cpu->regPC++);\
	cpu->regWZ = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
}

#define XDCB(base,_name) {\
	cpu->regWZ = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->t++;\
	cpu->tmpb = _name(cpu, cpu->tmpb);\
	z80_mwr(cpu, cpu->regWZ, cpu->tmpb);\
}

#define XDCBR(base,name,reg) {\
	XDCB(base,name);\
	reg = cpu->tmpb;\
}

#define	BITX(base,bit) {\
	cpu->regWZ = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->t++;\
	BITM(bit,cpu->tmpb);\
}

#define RESX(base,bit) {\
	cpu->regWZ = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->t++;\
	cpu->tmpb &= ~(0x01 << bit);\
	z80_mwr(cpu, cpu->regWZ, cpu->tmpb);\
}

#define RESXR(base,bit,reg) {\
	RESX(base,bit);\
	reg = cpu->tmpb;\
}

#define SETX(base,bit) {\
	cpu->regWZ = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = z80_mrd(cpu, cpu->regWZ); cpu->t++;\
	cpu->tmpb |= (0x01 << bit);\
	z80_mwr(cpu, cpu->regWZ, cpu->tmpb);\
}

#define SETXR(base,bit,reg) {\
	SETX(base,bit);\
	reg = cpu->tmpb;\
}
