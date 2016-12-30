#ifndef _LR_MACRO_H
#define _LR_MACRO_H

extern unsigned char FLHaddTab[8];
extern unsigned char FLHsubTab[8];

/*
// mem i/o
#define	MEMRD(adr,tk) cpu->mrd(adr,0,cpu->data);cpu->t+=tk;
#define	MEMWR(adr,val,tk) cpu->mwr(adr,val,cpu->data);cpu->t+=tk;
#define	IORD(port,tk) cpu->ird(port,cpu->data);cpu->t+=tk;
#define IOWR(port,val,tk) cpu->iwr(port,val,cpu->data);cpu->t+=tk;
*/

// ariphmetic
#define INCL(val) {\
	val++; \
	cpu->f = (cpu->f & FLC) | (val ? 0 : FLZ) | ((val & 0x0f) ? 0 : FLH);\
}

#define DECL(val) {\
	val--; \
	cpu->f = (cpu->f & FLC) | FLN | (val ? 0 : FLZ) | (((val & 0x0f) == 0x0f) ? FLH : 0);\
}

#define ADDL(val) {\
	cpu->tmpw = cpu->a + val;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a ? 0 : FLZ) | ((cpu->tmpw & 0x100) ? FLC : 0) | FLHaddTab[cpu->tmp & 7];\
}

#define ADCL(val) {\
	cpu->tmpw = cpu->a + val + (cpu->f & FC);\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a ? 0 : FLZ) | ((cpu->tmpw & 0x100) ? FLC : 0) | FLHaddTab[cpu->tmp & 7];\
}

#define SUBL(value) {\
	cpu->tmpw = cpu->a - value;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((value & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a ? 0 : FLZ) | ((cpu->tmpw & 0x100) ? FLC : 0) | FLN | FLHsubTab[cpu->tmp & 7];\
}

#define SBCL(value) {\
	cpu->tmpw = cpu->a - value - (cpu->f & FC);\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((value & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a ? 0 : FLZ) | ((cpu->tmpw & 0x100) ? FLC : 0) | FLN | FLHsubTab[cpu->tmp & 7];\
}

#define CMP(val) {\
	cpu->tmpw = cpu->a - val;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->f = (cpu->tmpw & FS) | (val & (F5 | F3)) | ((cpu->tmpw & 0x100) ? FLC : (cpu->tmpw ? 0 : FLZ)) | FLN | FLHsubTab[cpu->tmp & 7];\
}

#define ADDL16(val1,val2) {\
	cpu->tmpi = val1 + val2;\
	cpu->tmp = ((val1 & 0x800) >> 11) | ((val2 & 0x800) >> 10) | ((cpu->tmpi & 0x800) >> 9);\
	cpu->mptr = val1 + 1;\
	val1 = cpu->tmpi;\
	cpu->f = (cpu->f & FLZ) | ((cpu->tmpi & 0x10000) ? FLC : 0) | FLHaddTab[cpu->tmp & 7];\
}

#define SWAPH(rp) {rp = ((rp & 0xf0) >> 4) | ((rp & 0x0f) << 4); cpu->f = (rp ? 0 : FLZ);}		// swap hi/lo halfbyte

/*
#define ADC16(val) {\
	cpu->tmpi = cpu->hl + val + (cpu->f & FC);\
	cpu->tmp = ((cpu->hl & 0x8800) >> 11) | ((val & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);\
	cpu->mptr = cpu->hl + 1;\
	cpu->hl = cpu->tmpi;\
	cpu->f = (cpu->tmpi & 0x10000 ? FC : 0) | FVaddTab[cpu->tmp >> 4] | (cpu->h & (FS | F5 | F3)) | FHaddTab[cpu->tmp & 0x07] | (cpu->hl ? 0 : FZ);\
}

#define SBC16(val) {\
	cpu->tmpi = cpu->hl - val - (cpu->f & FC);\
	cpu->tmp = ((cpu->hl & 0x8800) >> 11) | ((val & 0x8800) >> 10) | ((cpu->tmpi & 0x8800) >> 9);\
	cpu->mptr = cpu->hl + 1;\
	cpu->hl = cpu->tmpi;\
	cpu->f = (cpu->tmpi & 0x10000 ? FC : 0) | FN | FVsubTab[cpu->tmp >> 4] | (cpu->h & (FS | F5 | F3 )) | FHsubTab[cpu->tmp & 0x07] | (cpu->hl ? 0 : FZ);\
}

// misc

#define JR(offset) {cpu->pc += (signed char)offset; cpu->mptr = cpu->pc; cpu->t += 5;}

#define POP(rh,rl) {rl = MEMRD(cpu->sp++,3); rh = MEMRD(cpu->sp++,3);}
#define PUSH(rh,rl) {MEMWR(--cpu->sp,rh,3); MEMWR(--cpu->sp,rl,3);}

#define RST(adr) {PUSH(cpu->hpc,cpu->lpc); cpu->mptr = adr; cpu->pc = cpu->mptr;}
#define RET {POP(cpu->hpc,cpu->lpc); cpu->mptr = cpu->pc;}
*/

// shift

#define RLX(val) {\
	cpu->tmp = val;\
	val = (val << 1) | ((cpu->f & FLC) ? 1 : 0);\
	cpu->f = ((cpu->tmp & 0x80) ? FLC : 0) | (val ? 0 : FLZ);\
}

#define RLCX(val) {\
	val = (val << 1) | (val >> 7);\
	cpu->f = ((val & 1) ? FLC : 0) | (val ? 0 : FLZ);\
}

#define RRX(val) {\
	cpu->tmp = val;\
	val = (val >> 1) | ((cpu->f & FLC) ? 0x80 : 0);\
	cpu->f = ((cpu->tmp & 1) ? FLC : 0) | (val ? 0 : FLZ);\
}

#define RRCX(val) {\
	cpu->f = val & FLC;\
	val = (val >> 1) | (val << 7);\
	cpu->f |= (val ? 0 : FLZ);\
}

#define SLAX(val) {\
	cpu->f = (val & 0x80) ? FLC : 0;\
	val <<= 1;\
	cpu->f |= (val ? 0 : FLZ);\
}

#define SLLX(val) {\
	cpu->f = (val & 0x80) ? FLC : 0;\
	val = (val << 1) | 0x01;\
	cpu->f |= (val ? 0 : FLZ);\
}

#define SRAX(val) {\
	cpu->f = (val & 1) ? FLC : 0;\
	val = (val & 0x80) | (val >> 1);\
	cpu->f |= (val ? 0 : FLZ);\
}

#define SRLX(val) {\
	cpu->f = (val & 1) ? FLC : 0;\
	val >>= 1;\
	cpu->f |= (val ? 0 : FLZ);\
}

// bit

#define BITL(bit,val) {cpu->f = (cpu->f & FLC) | FLH | ((val & (0x01 << bit)) ? 0 : FLZ);}
// #define BITML(bit,val) {cpu->f = (cpu->f & FLC) | FLH | (sz53pTab[val & (1 << bit)] & ~(F5 | F3)) | (cpu->hptr & (F5 | F3));}

/*
#define SET(bit,val) {val |= (1 << bit);}
#define RES(bit,val) {val &= ~(1 << bit);}

// extend

#define SWAP(rp1,rp2) {cpu->tmpw = rp1; rp1 = rp2; rp2 = cpu->tmpw;}		// swap 16bit regs

#define	RDSHIFT(base) {\
	cpu->tmp = MEMRD(cpu->pc++,3);\
	cpu->mptr = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
}

#define XDCB(base,name) {\
	cpu->mptr = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = MEMRD(cpu->mptr,4);\
	name(cpu->tmpb);\
	MEMWR(cpu->mptr,cpu->tmpb,3);\
}

#define XDCBR(base,name,reg) {\
	XDCB(base,name);\
	reg = cpu->tmpb;\
}

#define	BITX(base,bit) {\
	cpu->mptr = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = MEMRD(cpu->mptr,4);\
	BITM(bit,cpu->tmpb);\
}

#define RESX(base,bit) {\
	cpu->mptr = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = MEMRD(cpu->mptr,4);\
	cpu->tmpb &= ~(0x01 << bit);\
	MEMWR(cpu->mptr,cpu->tmpb,3);\
}

#define RESXR(base,bit,reg) {\
	RESX(base,bit);\
	reg = cpu->tmpb;\
}

#define SETX(base,bit) {\
	cpu->mptr = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = MEMRD(cpu->mptr,4);\
	cpu->tmpb |= (0x01 << bit);\
	MEMWR(cpu->mptr,cpu->tmpb,3);\
}

#define SETXR(base,bit,reg) {\
	SETX(base,bit);\
	reg = cpu->tmpb;\
}
*/

#endif
