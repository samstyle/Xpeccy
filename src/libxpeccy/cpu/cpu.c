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
void nil_get_regs(CPU* cpu, xRegBunch* bunch) {}
void nil_set_regs(CPU* cpu, xRegBunch bunch) {}

extern opCode npTab[256];
extern opCode lrTab[256];
extern opCode mosTab[256];

cpuCore cpuTab[] = {
	{CPU_Z80, "Z80", npTab, z80_reset, z80_exec, z80_asm, z80_mnem, z80_get_regs, z80_set_regs},
	{CPU_I8080, "i8080", NULL, i8080_reset, i8080_exec, i8080_asm, i8080_mnem, i8080_get_regs, i8080_set_regs},
	{CPU_LR35902, "LR35902", lrTab, lr_reset, lr_exec, lr_asm, lr_mnem, lr_get_regs, lr_set_regs},
	{CPU_6502, "MOS6502", mosTab, m6502_reset, m6502_exec, m6502_asm, m6502_mnem, m6502_get_regs, m6502_set_regs},
	{CPU_VM1, "1801VM1", NULL, pdp11_reset, pdp11_exec, pdp11_asm, pdp11_mnem, pdp11_get_regs, pdp11_set_regs},
	{CPU_NONE, "none", NULL, nil_reset, nil_exec, nil_asm, nil_mnem, nil_get_regs, nil_set_regs}
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
	cpu->asmbl = core->asmbl;
	cpu->mnem = core->mnem;
	cpu->tab = core->tab;
	cpu->getregs = core->getregs;
	cpu->setregs = core->setregs;
}

CPU* cpuCreate(int type, cbmr fmr, cbmw fmw, cbir fir, cbiw fiw, cbirq frq, void* dt) {
	CPU* cpu = (CPU*)malloc(sizeof(CPU));
	memset(cpu, 0x00, sizeof(CPU));
	cpu->data = dt;
	cpu->mrd = fmr;
	cpu->mwr = fmw;
	cpu->ird = fir;
	cpu->iwr = fiw;
	cpu->irq = frq;
	cpuSetType(cpu, type);
	return cpu;
}

void cpuDestroy(CPU* cpu) {
	free(cpu);
}

// disasm

static const char halfByte[] = "0123456789ABCDEF";
static char tmpbuf[1024];

xMnem cpuDisasm(CPU* cpu, unsigned short adr, char* buf, cbdmr mrd, void* data) {
	xMnem mn;
//	opCode* opt = cpu->tab;
	unsigned char op;
	unsigned char tmp;
	unsigned char dtl;
	unsigned char dth;
	unsigned short dtw;
	mn.mnem = NULL;
	mn.flag = 0;
	if (!buf) buf = tmpbuf;
//	if (opt == NULL) {			// no opcode tab
//		strcpy(buf, "undef");
//		mn.len = 1;
//	} else {
		mn = cpu->mnem(cpu, adr, mrd, data);
		mn.oadr = -1;
		adr += mn.len;
		tmp = mrd((adr - 2) & 0xffff, data);
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
						mn.oadr = dtl | (dth << 8);
						*buf++ = '#';
						*buf++ = halfByte[dth >> 4];
						*buf++ = halfByte[dth & 0x0f];
						*buf++ = halfByte[dtl >> 4];
						*buf++ = halfByte[dtl & 0x0f];
						break;
					case '3':		// word = adr + [e = (adr)]
						dtl = mrd(adr++,data);
						mn.len++;
						dtw = (adr + (signed char)dtl) & 0xffff;
						mn.oadr = dtw;
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
					case '6':		// = (adr + wrd[adr]) octal
						dtw = mrd(adr++, data);
						dtw |= (mrd(adr++, data) << 8);
						mn.len += 2;
						dtw += adr;
						buf += sprintf(buf, "%o", dtw);
						break;
					case '7':		// = #adr
						*buf++ = '#';
						*buf++ = halfByte[(adr >> 12) & 0x0f];
						*buf++ = halfByte[(adr >> 8) & 0x0f];
						*buf++ = halfByte[(adr >> 4) & 0x0f];
						*buf++ = halfByte[adr & 0x0f];
						break;
					case '8':		// = word (adr) octal
						dtw = mrd(adr++, data);
						dtw |= (mrd(adr++, data) << 8);
						mn.len += 2;
						*buf++ = '#';
						buf += sprintf(buf, "%o", dtw);
						break;
				}
			} else {
				*(buf++) = *(src++);
			}
		}
		*buf = 0;
//	}
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

// get/set reg bunch

static const char noname[] = "undef";

xRegBunch cpuGetRegs(CPU* cpu) {
	xRegBunch bunch;
	int i;
	for (i = 0; i < 32; i++) {
		bunch.regs[i].name = noname;
		bunch.regs[i].id = REG_NONE;
		bunch.regs[i].value = 0;
		bunch.regs[i].byte = 0;
	}
	memcpy(bunch.flags, "--------", 8);
	if (cpu->getregs) cpu->getregs(cpu, &bunch);
	return bunch;
}

void cpuSetRegs(CPU* cpu, xRegBunch bunch) {
	cpu->setregs(cpu, bunch);
	// cpu->r7 = cpu->r & 0x80;
}
