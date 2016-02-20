#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

CPU* cpuCreate(cbmr fmr, cbmw fmw, cbir fir, cbiw fiw, cbirq frq, void* dt) {
	CPU* cpu = (CPU*)malloc(sizeof(CPU));
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

void cpuDestroy(CPU* cpu) {
	free(cpu);
}

void cpuReset(CPU* cpu) {
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

int cpuExec(CPU* cpu) {
	cpu->t = 0;
	cpu->noint = 0;
	cpu->opTab = npTab;
	do {
		cpu->op = &cpu->opTab[cpu->mrd(cpu->pc++,1,cpu->data)];
		cpu->r++;
		cpu->t += cpu->op->t;
		cpu->op->exec(cpu);
	} while (cpu->op->prefix);
	return cpu->t;
}

int cpuINT(CPU* cpu) {
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
			while (cpu->op->prefix) {
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

int cpuNMI(CPU* cpu) {
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
		if (opc->prefix) {
			opt = opc->tab;
			if ((opt == ddcbTab) || (opt == fdcbTab)) {
				tmp = mrd(adr++,data);
				res++;
			}
		}
	} while (opc->prefix);
	const char* src = opc->mnem;
	while (*src != 0) {
		if (*src == ':') {
			src++;
			op = *(src++);
			switch(op) {
				case '1':		// byte = (adr)
					dtl = mrd(adr++,data);
					res++;
					*buf++ = halfByte[dtl >> 4];
					*buf++ = halfByte[dtl & 0x0f];
					break;
				case '2':		// word = (adr,adr+1)
					dtl = mrd(adr++,data);
					dth = mrd(adr++,data);
					res += 2;
					*buf++ = halfByte[dth >> 4];
					*buf++ = halfByte[dth & 0x0f];
					*buf++ = halfByte[dtl >> 4];
					*buf++ = halfByte[dtl & 0x0f];
					break;
				case '3':		// word = adr + [e = (adr)]
					dtl = mrd(adr++,data);
					res++;
					dtw = adr + (signed char)dtl;
					*buf++ = halfByte[(dtw >> 12) & 0x0f];
					*buf++ = halfByte[(dtw >> 8) & 0x0f];
					*buf++ = halfByte[(dtw >> 4) & 0x0f];
					*buf++ = halfByte[dtw & 0x0f];
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
					*buf++ = halfByte[tmp >> 4];
					*buf++ = halfByte[tmp & 0x0f];
					break;
			}
		} else {
			*(buf++) = *(src++);
		}
	}
	*buf = 0;
	return res;
}

// asm

typedef struct {
	unsigned match:1;
	int idx;
	opCode* op;
	char arg[8][256];
} xAsmScan;

const char letrz[] = "+-0123456789";

xAsmScan scanAsmTab(const char* com, opCode* tab) {
	xAsmScan res;
	res.match = 0;
	int i;
	for (i = 0; i < 8; i++)
		memset(res.arg[i], 0, 256);
	res.idx = -1;
	i = 0;
	int par = 0;
	int work;
	const char* cptr;
	const char* zptr;
	const char* tptr;
	do {
		cptr = com;
		tptr = tab[i].mnem;
		do {
			work = 0;
			if (*tptr == ':') {						// need argument
				if (strchr(letrz, *cptr)) {				// if there is a number (maybe signed)
					tptr += 2;
					zptr = strchr(cptr, *tptr);
					if (zptr || (*tptr == 0x00)) {
						if (par < 8) {
							strncpy(res.arg[par], cptr, zptr - cptr);
							if (!strchr(res.arg[par], ',')) {	// check if argument doesn't have ',' - wrong matching
								work = 1;
								cptr = zptr;
								par++;
							}
						}
					}
				}
			} else {							// check mnemonic char by char
				if (*cptr == *tptr) {					// compare current char
					if (*cptr == 0x00) {				// if both strings reach end
						res.match = 1;
						res.idx = i;
						res.op = &tab[i];
					} else {					// else go to next char
						work = 1;
						cptr++;
						tptr++;
					}
				}
			}
		} while (work);
		i++;
	} while (!res.match && (i < 256));
	return res;
}

int cpuAsm(const char* com, char* buf, unsigned short adr) {
	if (strlen(com) > 255) return 0;
	int par = 0;
	int num;
	int sign;
	char cbuf[256];
	const char* zptr;
	char* ptr = cbuf;
	strcpy(cbuf, com);
	while (*ptr) {			// toLower;
		if ((*ptr >= 'A') && (*ptr <= 'Z')) {
			*ptr ^= 0x20;
		}
		ptr++;
	}

	xAsmScan res = scanAsmTab(cbuf, npTab);
	ptr = buf;
	if (!res.match) {
		ptr = buf;
		*ptr++ = 0xdd;
		res = scanAsmTab(cbuf, ddTab);
	}
	if (!res.match) {
		ptr = buf;
		*ptr++ = 0xfd;
		res = scanAsmTab(cbuf, fdTab);
	}
	if (!res.match) {
		ptr = buf;
		*ptr++ = 0xcb;
		res = scanAsmTab(cbuf, cbTab);
	}
	if (!res.match) {
		ptr = buf;
		*ptr++ = 0xed;
		res = scanAsmTab(cbuf, edTab);
	}
	if (!res.match) {
		ptr = buf;
		*ptr++ = 0xdd;
		*ptr++ = 0xcb;
		res = scanAsmTab(cbuf, ddcbTab);
	}
	if (!res.match) {
		ptr = buf;
		*ptr++ = 0xfd;
		*ptr++ = 0xcb;
		res = scanAsmTab(cbuf, fdcbTab);
	}
	if (res.match) {
		*ptr++ = res.idx;
		zptr = res.op->mnem;

		while ((par < 7) && (strlen(res.arg[par]) > 0)) {
			zptr = strchr(zptr, ':');
			if (!zptr) break;
			zptr++;
			num = strtol(res.arg[par], NULL, 0);
			sign = ((res.arg[par][0] == '+') || (res.arg[par][0] == '-')) ? 1 : 0;
			par++;
			switch(*zptr) {
				case '1':
					*ptr++ = num & 0xff;
					break;
				case '2':
					*ptr++ = num & 0xff;
					*ptr++ = (num >> 8) & 0xff;
					break;
				case '3':
					if (!sign) {
						num -= (adr + (ptr - buf + 1));
					}
					*ptr++ = num & 0xff;
					break;
				case '4':
					if (sign) {
						*ptr++ = num & 0xff;
					} else {
						ptr = buf;
						par = 100;
					}
					break;
				case '5':
					if (sign) {
						*ptr = *(ptr-1);
						*(ptr-1) = num & 0xff;
						ptr++;
					} else {
						ptr = buf;
						par = 100;
					}
					break;
			}
		}
	} else {
		ptr = buf;
	}
	return (ptr - buf);
}
