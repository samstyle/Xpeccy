#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include <stdio.h>

const char nomnem[] = "undef";

void nil_reset(CPU* cpu) {}
int nil_exec(CPU* cpu) {return 1;}
int nil_int(CPU* cpu) {return 1;}
int nil_nmi(CPU* cpu) {return 1;}
xAsmScan nil_asm(const char* com, char* buf) {
	xAsmScan res;
	res.match = 0;
	return res;
}
xMnem nil_mnem(CPU* cpu, unsigned short adr, cbdmr mrd, void* data) {
	xMnem res;
	res.len = 1;
	res.mnem = nomnem;
	res.mem = 0;
	res.cond = 0;
	return res;
}

extern opCode npTab[256];
extern opCode lrTab[256];

cpuCore cpuTab[] = {
	{CPU_Z80, "Z80", npTab, z80_reset, z80_exec, z80_int, z80_asm, z80_mnem},
	{CPU_LR35902, "LR35902", lrTab, lr_reset, lr_exec, lr_int, lr_asm, lr_mnem},
	{CPU_NONE, "none", NULL, nil_reset, nil_exec, nil_int, nil_asm, nil_mnem}
};

cpuCore* findCore(int type) {
	int idx = 0;
	while ((cpuTab[idx].type != CPU_NONE) && (cpuTab[idx].type != type)) {
		idx++;
	}
	return &cpuTab[idx];
}

int getCoreID(const char* name) {
	int idx = 0;
	while ((cpuTab[idx].type != CPU_NONE) && strcmp(name, cpuTab[idx].name)) {
		idx++;
	}
	return cpuTab[idx].type;
}

const char* getCoreName(int type) {
	cpuCore* core = findCore(type);
	return core->name;
}

void cpuSetType(CPU* cpu, int type) {
	cpuCore* core = findCore(type);
	cpu->type = core->type;
	cpu->reset = core->reset;
	cpu->exec = core->exec;
	cpu->intr = core->intr;
	cpu->asmbl = core->asmbl;
	cpu->mnem = core->mnem;
	cpu->tab = core->tab;
}

CPU* cpuCreate(int type, cbmr fmr, cbmw fmw, cbir fir, cbiw fiw, cbirq frq, void* dt) {
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
	cpuSetType(cpu, type);
	return cpu;
}

void cpuDestroy(CPU* cpu) {
	free(cpu);
}

// disasm

const char halfByte[] = "0123456789ABCDEF";

xMnem cpuDisasm(CPU* cpu, unsigned short adr, char* buf, cbdmr mrd, void* data) {
	xMnem mn;
	opCode* opt = cpu->tab;
	if (opt == NULL) {			// no opcode tab
		strcpy(buf, "undef");
		mn.len = 1;
	} else {
		unsigned char op;
		unsigned char tmp = 0;
		unsigned char dtl;
		unsigned char dth;
		unsigned short dtw;
		mn = cpu->mnem(cpu, adr, mrd, data);
		adr += mn.len;
		const char* src = mn.mnem;

		while (*src != 0) {
			if (*src == ':') {
				src++;
				op = *(src++);
				switch(op) {
					case '1':		// byte = (adr)
						dtl = mrd(adr++,data);
						mn.len++;
						*buf++ = '#';
						*buf++ = halfByte[dtl >> 4];
						*buf++ = halfByte[dtl & 0x0f];
						break;
					case '2':		// word = (adr,adr+1)
						dtl = mrd(adr++,data);
						dth = mrd(adr++,data);
						mn.len += 2;
						*buf++ = '#';
						*buf++ = halfByte[dth >> 4];
						*buf++ = halfByte[dth & 0x0f];
						*buf++ = halfByte[dtl >> 4];
						*buf++ = halfByte[dtl & 0x0f];
						break;
					case '3':		// word = adr + [e = (adr)]
						dtl = mrd(adr++,data);
						mn.len++;
						dtw = adr + (signed char)dtl;
						*buf++ = '#';
						*buf++ = halfByte[(dtw >> 12) & 0x0f];
						*buf++ = halfByte[(dtw >> 8) & 0x0f];
						*buf++ = halfByte[(dtw >> 4) & 0x0f];
						*buf++ = halfByte[dtw & 0x0f];
						break;
					case '4':		// signed byte e = (adr)
						tmp = mrd(adr++,data);
						mn.len++;
					case '5':		// signed byte e = tmp
						if (tmp < 0x80) {
							*(buf++) = '+';
						} else {
							*(buf++) = '-';
							tmp = (0xff - tmp) + 1;
						}
						*buf++ = '#';
						*buf++ = halfByte[tmp >> 4];
						*buf++ = halfByte[tmp & 0x0f];
						break;
				}
			} else {
				*(buf++) = *(src++);
			}
		}
		*buf = 0;
	}
	return mn;
}

// asm

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

// asm


int cpuAsm(CPU* cpu, const char* com, char* buf, unsigned short adr) {
	if (strlen(com) > 255) return 0;
	int par = 0;
	int num;
	int sign;
	char cbuf[256];
	const char* zptr;

	char* ptr = cbuf;	// cbuf = toLower(com)
	strcpy(cbuf, com);
	while (*ptr) {
		if ((*ptr >= 'A') && (*ptr <= 'Z')) {
			*ptr ^= 0x20;
		}
		ptr++;
	}

	xAsmScan res = cpu->asmbl(cbuf, buf);
	if (res.match) {
		ptr = res.ptr;
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
