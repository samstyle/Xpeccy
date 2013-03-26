#include <stdlib.h>

#include "z80.h"
#include "z80macros.h"
#include "z80tables.c"

extern opCode npTab[256];
extern opCode ddTab[256];
extern opCode fdTab[256];
extern opCode cbTab[256];
extern opCode edTab[256];

#include "z80nop.c"
#include "z80ed.c"
#include "z80ddcb.c"
#include "z80fdcb.c"
#include "z80dd.c"
#include "z80fd.c"
#include "z80cb.c"

Z80CPU* cpuCreate(cbmr fmr, cbmw fmw, cbir fir, cbiw fiw, cbirq frq, void* dt) {
	Z80CPU* cpu = (Z80CPU*)malloc(sizeof(Z80CPU));
	cpu->data = dt;
	cpu->mrd = fmr;
	cpu->mwr = fmw;
	cpu->ird = fir;
	cpu->iwr = fiw;
	cpu->irq = frq;
	cpu->halt = 0;
	cpu->resPV = 0;
	cpu->noint = 0;
	cpu->imode = 0;
	return cpu;
}

void cpuDestroy(Z80CPU* cpu) {
	free(cpu);
}

void cpuReset(Z80CPU* cpu) {
	cpu->pc = 0;
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	cpu->imode = 0;
	cpu->af = cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->af_ = cpu->bc_ = cpu->de_ = cpu->hl_ = 0xffff;
	cpu->ix = cpu->iy = 0xffff;
	cpu->sp = 0xffff;
	cpu->i = cpu->r = cpu->r7 = 0;
}

int cpuExec(Z80CPU* cpu) {
	cpu->t = 0;
	cpu->noint = 0;
	cpu->opTab = npTab;
	do {
		cpu->op = &cpu->opTab[cpu->mrd(cpu->pc++,1,cpu->data)];
		cpu->r++;
		cpu->t += cpu->op->t;
		cpu->op->exec(cpu);
	} while (cpu->op->flag & 1);
	return cpu->t;
}

int cpuINT(Z80CPU* cpu) {
	if (!cpu->iff1 || cpu->noint) return 0;
	cpu->iff1 = 0;
	cpu->iff2 = 0;
	if (cpu->halt) {
		cpu->pc++;
		cpu->halt = 0;
	}
	if (cpu->resPV) {
		cpu->f &= ~FP;
		cpu->resPV = 0;
	}
	cpu->opTab = npTab;
	switch(cpu->imode) {
		case 0:
			cpu->t = 2;
			cpu->op = &cpu->opTab[cpu->irq(cpu->data)];
			cpu->r++;
			cpu->t += cpu->op->t;		// +5 (RST38 fetch)
			cpu->op->exec(cpu);		// +3 +3 execution. 13 total
			while (cpu->op->flag & 1) {
				cpu->op = &cpu->opTab[cpu->mrd(cpu->pc++,1,cpu->data)];
				cpu->r++;
				cpu->t += cpu->op->t;
				cpu->op->exec(cpu);
			}
			break;
		case 1:
			cpu->r++;
			cpu->t = 2 + 5;	// 2 extra + 5 on RST38 fetch
			nprFF(cpu);	// +3 +3 execution. 13 total
			break;
		case 2:
			cpu->r++;
			cpu->t = 7;
			PUSH(cpu->hpc,cpu->lpc);	// +3 (10) +3 (13)
			cpu->lptr = cpu->irq(cpu->data);	// int vector (FF)
			cpu->hptr = cpu->i;
			cpu->lpc = MEMRD(cpu->mptr++,3);	// +3 (16)
			cpu->hpc = MEMRD(cpu->mptr,3);		// +3 (19)
			cpu->mptr = cpu->pc;
			break;
	}
	return cpu->t;
}

int cpuNMI(Z80CPU* cpu) {
	if (cpu->noint) return 0;
	cpu->r++;
	cpu->iff1 = 0;
	cpu->t = 5;
	PUSH(cpu->hpc,cpu->lpc);
	cpu->pc = 0x0066;
	cpu->mptr = cpu->pc;
	return cpu->t;		// always 11
}

// disasm

const char halfByte[] = "0123456789ABCDEF";

int cpuDisasm(unsigned short adr,char* buf, cbdmr mrd, void* data) {
	unsigned char op;
	unsigned char tmp = 0;
	unsigned char dtl;
	unsigned char dth;
	unsigned short dtw;
	int res = 0;
	opCode* opc;
	opCode* opt = npTab;
	do {
		op = mrd(adr++,data);
		res++;
		opc = &opt[op];
		if (opc->flag & 1) {
			opt = opc->tab;
			if ((opt == ddcbTab) || (opt == fdcbTab)) {
				tmp = mrd(adr++,data);
				res++;
			}
		}
	} while (opc->flag & 1);
	const char* src = opc->mnem;
	while (*src != 0) {
		if (*src == ':') {
			src++;
			op = *(src++);
			switch(op) {
				case '1':		// byte = (adr)
					dtl = mrd(adr++,data);
					res++;
					*(buf++) = halfByte[dtl >> 4];
					*(buf++) = halfByte[dtl & 0x0f];
					break;
				case '2':		// word = (adr,adr+1)
					dtl = mrd(adr++,data);
					dth = mrd(adr++,data);
					res += 2;
					*(buf++) = halfByte[dth >> 4];
					*(buf++) = halfByte[dth & 0x0f];
					*(buf++) = halfByte[dtl >> 4];
					*(buf++) = halfByte[dtl & 0x0f];
					break;
				case '3':		// word = adr + [e = (adr)]
					dtl = mrd(adr++,data);
					res++;
					dtw = adr + (signed char)dtl;
					*(buf++) = halfByte[(dtw >> 12) & 0x0f];
					*(buf++) = halfByte[(dtw >> 8) & 0x0f];
					*(buf++) = halfByte[(dtw >> 4) & 0x0f];
					*(buf++) = halfByte[dtw & 0x0f];
					break;
				case '4':		// signed byte e = (adr)
					tmp = mrd(adr++,data);
					res++;
				case '5':		// signed byte e = tmp
					if (tmp < 0x80) {
						*(buf++) = '+';
					} else {
						*(buf++) = '-';
						tmp = (0xff - tmp) + 1;
					}
					*(buf++) = halfByte[tmp >> 4];
					*(buf++) = halfByte[tmp & 0x0f];
					break;
			}
		} else {
			*(buf++) = *(src++);
		}
	}
	*buf = 0;
	return res;
}
