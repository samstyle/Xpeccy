#ifndef _LR_MACRO_H
#define _LR_MACRO_H

extern unsigned char FLHaddTab[8];
extern unsigned char FLHsubTab[8];

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
	cpu->tmpw = cpu->a + val + ((cpu->f & FLC) ? 1 : 0);\
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
	cpu->tmpw = cpu->a - value - ((cpu->f & FLC) ? 1 : 0);\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((value & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a ? 0 : FLZ) | ((cpu->tmpw & 0x100) ? FLC : 0) | FLN | FLHsubTab[cpu->tmp & 7];\
}

#define CMP(val) {\
	cpu->tmpw = cpu->a - val;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->f = ((cpu->tmpw & 0x100) ? FLC : (cpu->tmpw ? 0 : FLZ)) | FLN | FLHsubTab[cpu->tmp & 7];\
}

#define ADDL16(val1,val2) {\
	cpu->tmpi = val1 + val2;\
	cpu->tmp = ((val1 & 0x800) >> 11) | ((val2 & 0x800) >> 10) | ((cpu->tmpi & 0x800) >> 9);\
	cpu->mptr = val1 + 1;\
	val1 = cpu->tmpi;\
	cpu->f = (cpu->f & FLZ) | ((cpu->tmpi & 0x10000) ? FLC : 0) | FLHaddTab[cpu->tmp & 7];\
}

// misc

#define SWAPH(rp) {rp = ((rp & 0xf0) >> 4) | ((rp & 0x0f) << 4); cpu->f = (rp ? 0 : FLZ);}		// swap hi/lo halfbyte

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
	cpu->f = (val & 1) ? FLC : 0;\
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

#endif
