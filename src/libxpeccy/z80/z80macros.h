// mem i/o
#define	MEMRD(adr,tk) cpu->mrd(adr,0,cpu->data);cpu->t+=tk;
#define	MEMWR(adr,val,tk) cpu->mwr(adr,val,cpu->data);cpu->t+=tk;
#define	IORD(port,tk) cpu->ird(port,cpu->data);cpu->t+=tk;
#define IOWR(port,val,tk) cpu->iwr(port,val,cpu->data);cpu->t+=tk;

// ariphmetic
#define INC(val) {val++; cpu->f = (cpu->f & FC) | (val ? 0 : FZ) | (val & (FS | F5 | F3)) | ((val == 0x80) ? FV : 0) | ((val & 0x0f) ? 0 : FH);}
#define DEC(val) {cpu->f = (cpu->f & FC) | ((val & 0x0f) ? 0 : FH ) | FN; val--; cpu->f |= ((val == 0x7f) ? FV : 0 ) | (val & (FS | F5 | F3));}

#define ADD(val) {\
	cpu->tmpw = cpu->a + val;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (FS | F5 | F3)) | (cpu->a ? 0 : FZ) | ((cpu->tmpw & 0x100) ? FC : 0) | FHaddTab[cpu->tmp & 7] | FVaddTab[cpu->tmp >> 4];\
}

#define ADC(val) {\
	cpu->tmpw = cpu->a + val + (cpu->f & FC);\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (FS | F5 | F3)) | (cpu->a ? 0 : FZ) | ((cpu->tmpw & 0x100) ? FC : 0) | FHaddTab[cpu->tmp & 7] | FVaddTab[cpu->tmp >> 4];\
}

#define SUB(value) {\
	cpu->tmpw = cpu->a - value;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((value & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (FS | F5 | F3)) | (cpu->a ? 0 : FZ) | ((cpu->tmpw & 0x100) ? FC : 0) | FN | FHsubTab[cpu->tmp & 0x07] | FVsubTab[cpu->tmp >> 4];\
}

#define SBC(value) {\
	cpu->tmpw = cpu->a - value - (cpu->f & FC);\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((value & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->a = cpu->tmpw & 0xff;\
	cpu->f = (cpu->a & (FS | F5 | F3)) | (cpu->a ? 0 : FZ) | ((cpu->tmpw & 0x100) ? FC : 0) | FN | FHsubTab[cpu->tmp & 0x07] | FVsubTab[cpu->tmp >> 4];\
}

#define CP(val) {\
	cpu->tmpw = cpu->a - val;\
	cpu->tmp = ((cpu->a & 0x88) >> 3) | ((val & 0x88) >> 2) | ((cpu->tmpw & 0x88) >> 1);\
	cpu->f = (cpu->tmpw & FS) | (val & (F5 | F3)) | ((cpu->tmpw & 0x100) ? FC : (cpu->tmpw ? 0 : FZ)) | FN | FHsubTab[cpu->tmp & 0x07] | FVsubTab[cpu->tmp >> 4];\
}

#define ADD16(val1,val2) {\
	cpu->tmpi = val1 + val2;\
	cpu->tmp = ((val1 & 0x800) >> 11) | ((val2 & 0x800) >> 10) | ((cpu->tmpi & 0x800) >> 9);\
	cpu->mptr = val1 + 1;\
	val1 = cpu->tmpi;\
	cpu->f = (cpu->f & (FS | FZ | FV)) | ((cpu->tmpi & 0x10000) ? FC : 0) | ((val1 >> 8) & (F5 | F3)) | FHaddTab[cpu->tmp];\
}

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
	cpu->f = (cpu->tmpi & 0x10000 ? FC : 0) | FN | FVsubTab[cpu->tmp >> 4] | (cpu->h & (FS | F5 | F3 )) | FHsubTab[cpu->tmp & 0x07] | (cpu->h ? 0 : FZ);\
}

// misc

#define JR(offset) {cpu->pc += (signed char)offset; cpu->mptr = cpu->pc; cpu->t += 5;}

#define POP(rh,rl) {rh = MEMRD(cpu->sp++,3); rl = MEMRD(cpu->sp++,3);}
#define PUSH(rh,rl) {MEMWR(--cpu->sp,rl,3); MEMWR(--cpu->sp,rh,3);}

#define RST(adr) {PUSH(cpu->hpc,cpu->lpc);cpu->mptr = adr;cpu->pc = cpu->mptr;}
#define RET {POP(cpu->hpc,cpu->lpc);cpu->mptr = cpu->pc;}

// shift

#define RL(val) {cpu->tmp = val; val = (val << 1) | (cpu->f & FC); cpu->f = (cpu->tmp >> 7) | sz53pTab[val];}
#define RLC(val) {val = (val << 1) | (val >> 7); cpu->f = (val & FC) | sz53pTab[val];}
#define RR(val) {cpu->tmp = val; val = (val >> 1) | (cpu->f << 7); cpu->f = (cpu->tmp & FC) | sz53pTab[val];}
#define RRC(val) {cpu->f = val & FC; val = (val >> 1) | (val << 7); cpu->f |= sz53pTab[val];}

#define SLA(val) {cpu->f = (val >> 7); val <<= 1; cpu->f |= sz53pTab[val];}
#define SLL(val) {cpu->f = (val >> 7); val = (val << 1) | 0x01; cpu->f |= sz53pTab[val];}
#define SRA(val) {cpu->f = val & FC; val = (val & 0x80) | (val >> 1); cpu->f |= sz53pTab[val];}
#define SRL(val) {cpu->f = val & FC; val >>= 1; cpu->f |= sz53pTab[val];}

// bit

#define BIT(bit,val) {cpu->f = (cpu->f & FC ) | FH | sz53pTab[val & (0x01 << bit)] | (val & (F5 | F3));}

// extend

#define SWAP(rp1,rp2) {cpu->tmpw = rp1; rp1 = rp2; rp2 = cpu->tmpw;}

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

#define	BITX(base,num) {\
	cpu->mptr = base + (signed char)cpu->tmp;\
	cpu->t += 5;\
	cpu->tmpb = MEMRD(cpu->mptr,4);\
	BIT(num,cpu->tmpb);\
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
