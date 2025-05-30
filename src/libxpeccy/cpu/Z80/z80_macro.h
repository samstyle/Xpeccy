#pragma once

extern const unsigned char daaTab[0x1000];
// extern const unsigned char sz53pTab[0x100];
extern const unsigned char FHaddTab[8];
extern const unsigned char FHsubTab[8];
extern const unsigned char FVaddTab[8];
extern const unsigned char FVsubTab[8];

// mem i/o
//#define	MEMRD(adr,tk) cpu->mrd(adr,0,cpu->xptr);cpu->t+=tk;
//#define	MEMWR(adr,val,tk) cpu->mwr(adr,val,cpu->xptr);cpu->t+=tk;
// #define	IORD(port,tk) cpu->ird(port,cpu->data);cpu->t+=tk;

// ariphmetic
/*
#define INC(val) {\
	val++; \
	cpu->f = (cpu->f & Z80_FC) | (val ? 0 : Z80_FZ) | (val & (Z80_FS | Z80_F5 | Z80_F3)) | ((val == 0x80) ? Z80_FV : 0) | ((val & 0x0f) ? 0 : Z80_FH);\
}

#define DEC(val) {\
	cpu->f = (cpu->f & Z80_FC) | ((val & 0x0f) ? 0 : Z80_FH ) | Z80_FN; \
	val--; \
	cpu->f |= ((val == 0x7f) ? Z80_FV : 0 ) | (sz53pTab[val] & ~Z80_FP);\
}

#define ADD(val) {\
	cpu->tmpw = cpu->a + val;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | ((cpu->tmpw & 0x100) ? Z80_FC : 0) | FHaddTab[cpu->tmp & 7] | FVaddTab[cpu->tmp >> 4];\
}

#define ADC(val) {\
	cpu->tmpw = cpu->a + val + (cpu->f & Z80_FC);\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | ((cpu->tmpw & 0x100) ? Z80_FC : 0) | FHaddTab[cpu->tmp & 7] | FVaddTab[cpu->tmp >> 4];\
}

#define SUB(value) {\
	cpu->tmpw = cpu->a - value;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((value & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | ((cpu->tmpw & 0x100) ? Z80_FC : 0) | Z80_FN | FHsubTab[cpu->tmp & 0x07] | FVsubTab[cpu->tmp >> 4];\
}

#define SBC(value) {\
	cpu->tmpw = cpu->a - value - (cpu->f & Z80_FC);\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((value & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (Z80_FS | Z80_F5 | Z80_F3)) | (cpu->a ? 0 : Z80_FZ) | ((cpu->tmpw & 0x100) ? Z80_FC : 0) | Z80_FN | FHsubTab[cpu->tmp & 0x07] | FVsubTab[cpu->tmp >> 4];\
}

#define CP(val) {\
	cpu->tmpw = cpu->a - val;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->f = (cpu->tmpw & Z80_FS) | (val & (Z80_F5 | Z80_F3)) | ((cpu->tmpw & 0x100) ? Z80_FC : 0) | (cpu->tmpw ? 0 : Z80_FZ)) | Z80_FN | FHsubTab[cpu->tmp & 0x07] | FVsubTab[cpu->tmp >> 4];\
}
*/

/*
#define ADD16(val1,val2) {\
	cpu->tmpi = val1 + val2;\
	cpu->tmp = ((val1 & 0x800) >> 11) | ((val2 & 0x800) >> 10) | ((cpu->tmpi & 0x800) >> 9);\
	cpu->mptr = val1 + 1;\
	val1 = cpu->tmpi;\
	cpu->f = (cpu->f & (Z80_FS | Z80_FZ | Z80_FV)) | ((cpu->tmpi & 0x10000) ? Z80_FC : 0) | ((val1 >> 8) & (Z80_F5 | Z80_F3)) | FHaddTab[cpu->tmp];\
}

#define ADC16(val) {\
	cpu->tmpi = cpu->hl + val + (cpu->f & Z80_FC);\
	cpu->tmp = ((cpu->hl & 0x8800) >> 11) | ((val & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);\
	cpu->mptr = cpu->hl + 1;\
	cpu->hl = cpu->tmpi;\
	cpu->f = (cpu->tmpi & 0x10000 ? Z80_FC : 0) | FVaddTab[cpu->tmp >> 4] | (cpu->h & (Z80_FS | Z80_F5 | Z80_F3)) | FHaddTab[cpu->tmp & 0x07] | (cpu->hl ? 0 : Z80_FZ);\
}

#define SBC16(val) {\
	cpu->tmpi = cpu->hl - val - (cpu->f & Z80_FC);\
	cpu->tmp = ((cpu->hl & 0x8800) >> 11) | ((val & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);\
	cpu->mptr = cpu->hl + 1;\
	cpu->hl = cpu->tmpi;\
	cpu->f = (cpu->tmpi & 0x10000 ? Z80_FC : 0) | Z80_FN | FVsubTab[cpu->tmp >> 4] | (cpu->h & (Z80_FS | Z80_F5 | Z80_F3 )) | FHsubTab[cpu->tmp & 0x07] | (cpu->hl ? 0 : Z80_FZ);\
}
*/

// misc

// #define JR(offset) {cpu->pc += (signed char)offset; cpu->mptr = cpu->pc; cpu->t += 5;}

//#define POP(rh,rl) {rl = MEMRD(cpu->sp++,3); rh = MEMRD(cpu->sp++,3);}
//#define PUSH(rh,rl) {MEMWR(--cpu->sp,rh,3); MEMWR(--cpu->sp,rl,3);}

//#define RST(adr) {PUSH(cpu->hpc,cpu->lpc); cpu->mptr = adr; cpu->pc = cpu->mptr;}
//#define RET {POP(cpu->hpc,cpu->lpc); cpu->mptr = cpu->pc;}

// shift

/*
#define RL(val) {\
	cpu->tmp = val;\
	val = (val << 1) | cpu->fz.c;\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
	cpu->fz.c = !!(cpu->tmp & 0x80);\
}

#define RLC(val) {\
	val = (val << 1) | (val >> 7);\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
	cpu->fz.c = (val & 1);\
}

#define RR(val) {\
	cpu->tmp = val;\
	val = (val >> 1) | (cpu->fz.c << 7);\
	cpu->fz.c = cpu->tmp & 1;\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
}

#define RRC(val) {\
	cpu->fz.c = val & 1;\
	val = (val >> 1) | (val << 7);\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
}

#define SLA(val) {\
	cpu->fz.c = !!(val & 0x80);\
	val <<= 1;\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
}

#define SLL(val) {\
	cpu->fz.c = !!(val & 0x80);\
	val = (val << 1) | 0x01;\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
}

#define SRA(val) {\
	cpu->fz.c = val & 1;\
	val = (val & 0x80) | (val >> 1);\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
}

// cpu->f = val & 1; val >>= 1; cpu->f |= sz53pTab[val];}
#define SRL(val) {\
	cpu->fz.c = val & 1;\
	val >>= 1;\
	cpu->fz.s = !!(val & 0x80);\
	cpu->fz.z = !val;\
	cpu->fz.f5 = !!(val & 0x20);\
	cpu->fz.h = 0;\
	cpu->fz.f3 = !!(val & 0x08);\
	cpu->fz.pv = parity(val);\
	cpu->fz.n = 0;\
}
*/

// bit
// TODO:z = bit value, p/v = z, s = (val & (1 << bit) & 0x80)

// cpu->f = (cpu->f & Z80_FC) | Z80_FH | (sz53pTab[val & (0x01 << bit)] & ~(Z80_F5 | Z80_F3)) | ((val & (1 << bit)) & (Z80_F5 | Z80_F3));}
#define BIT(bit,val) {\
	cpu->tmp = val & (1 << bit);\
	cpu->fz.s = !!(cpu->tmp & 0x80);\
	cpu->fz.z = !cpu->tmp;\
	cpu->fz.f5 = !!(cpu->tmp & 0x20);\
	cpu->fz.h = 1;\
	cpu->fz.f3 = !!(cpu->tmp & 0x08);\
	cpu->fz.pv = cpu->fz.z;\
	cpu->fz.n = 0;\
}

// cpu->f = (cpu->f & Z80_FC) | Z80_FH | (sz53pTab[val & (1 << bit)] & ~(Z80_F5 | Z80_F3)) | (cpu->hptr & (Z80_F5 | Z80_F3));}
#define BITM(bit,val) {\
	cpu->tmp = val & (1 << bit);\
	cpu->fz.s = !!(cpu->tmp & 0x80);\
	cpu->fz.z = !cpu->tmp;\
	cpu->fz.f5 = !!(cpu->regWZh & 0x20);\
	cpu->fz.h = 1;\
	cpu->fz.f3 = !!(cpu->regWZh & 0x08);\
	cpu->fz.pv = cpu->fz.z;\
	cpu->fz.n = 0;\
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
