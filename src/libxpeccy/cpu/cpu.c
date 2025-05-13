#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "cpu.h"

// common

int parity(int val) {
	int parity = 1;
	while (val) {
		parity ^= val;		// bit 0 is only significant
		val >>= 1;
	}
	return parity & 1;
}

// TODO: apply cpu adr bus mask here
int cpu_fetch(CPU* cpu, int adr) {return cpu->mrd(adr, 1, cpu->xptr);}
int cpu_mrd(CPU* cpu, int adr) {return cpu->mrd(adr, 0, cpu->xptr);}
void cpu_mwr(CPU* cpu, int adr, int v) {cpu->mwr(adr, v, cpu->xptr);}
int cpu_ird(CPU* cpu, int adr) {return cpu->ird(adr, cpu->xptr);}
void cpu_iwr(CPU* cpu, int adr, int v) {cpu->iwr(adr, v, cpu->xptr);}

// no-proc

const char nomnem[] = "undef";

xRegDsc nil_reg_tab[] = {
	{REG_NONE, "", 0, 0}
};

void nil_reset(CPU* cpu) {}
int nil_exec(CPU* cpu) {return 1;}
int nil_int(CPU* cpu) {return 1;}
int nil_nmi(CPU* cpu) {return 1;}
xAsmScan nil_asm(int adr, const char* com, char* buf) {
	xAsmScan res;
	res.match = 0;
	return res;
}
xMnem nil_mnem(CPU* cpu, int adr, cbdmr mrd, void* data) {
	xMnem res;
	res.len = 1;
	res.mnem = nomnem;
	res.mem = 0;
	res.cond = 0;
	return res;
}
void nil_get_regs(CPU* cpu, xRegBunch* bunch) {}
void nil_set_regs(CPU* cpu, xRegBunch bunch) {}

extern xRegDsc z80RegTab[];
extern xRegDsc m6502RegTab[];
extern xRegDsc lrRegTab[];
extern xRegDsc i8080RegTab[];
extern xRegDsc i286RegTab[];
extern xRegDsc pdp11RegTab[];

cpuCore cpuTab[] = {
	{CPU_Z80, 0,"Z80", z80RegTab, NULL, z80_reset, z80_exec, z80_asm, z80_mnem, z80_get_regs, z80_set_regs},
	{CPU_I8080, 0,"i8080", i8080RegTab, NULL, i8080_reset, i8080_exec, i8080_asm, i8080_mnem, i8080_get_regs, i8080_set_regs},
	{CPU_I8086, 0,"i8086", i286RegTab, NULL, i286_reset, i286_exec, i286_asm, i286_mnem, i286_get_regs, i286_set_regs},
	{CPU_I80186, 1,"i80186", i286RegTab, NULL, i286_reset, i286_exec, i286_asm, i286_mnem, i286_get_regs, i286_set_regs},
	{CPU_I80286, 2,"i80286", i286RegTab, NULL, i286_reset, i286_exec, i286_asm, i286_mnem, i286_get_regs, i286_set_regs},
	{CPU_LR35902, 0, "LR35902", lrRegTab, NULL, lr_reset, lr_exec, lr_asm, lr_mnem, lr_get_regs, lr_set_regs},
	{CPU_6502, 0, "MOS6502", m6502RegTab, NULL, m6502_reset, m6502_exec, m6502_asm, m6502_mnem, m6502_get_regs, m6502_set_regs},
	{CPU_VM1, 0, "1801VM1", pdp11RegTab, NULL, pdp11_reset, pdp11_exec, pdp11_asm, pdp11_mnem, pdp11_get_regs, pdp11_set_regs},
	{CPU_VM2, 1, "1801VM2", pdp11RegTab, NULL, pdp11_reset, pdp11_exec, pdp11_asm, pdp11_mnem, pdp11_get_regs, pdp11_set_regs},
	{CPU_NONE, 0, "none", nil_reg_tab, NULL, nil_reset, nil_exec, nil_asm, nil_mnem, nil_get_regs, nil_set_regs}
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

void cpuSetCore(CPU* cpu, cpuCore* core) {
	cpu->core = core;
	cpu->type = core->type;
	cpu->gen = core->gen;
	cpu->reset = core->reset;
	cpu->exec = core->exec;
	cpu->asmbl = core->asmbl;
	cpu->mnem = core->mnem;
	cpu->getregs = core->getregs;
	cpu->setregs = core->setregs;
	if (core->init) {
		core->init(cpu);
	}
}

void cpuSetType(CPU* cpu, int type) {
	cpuCore* core = findCore(type);
	if (core != NULL) {
		if (cpu->lib) {
			dlclose(cpu->libhnd);
			cpu->lib = 0;
		}
		cpuSetCore(cpu, core);
	}
}

// for future: CPU from external so/dll
int cpuSetLib(CPU* cpu, const char* name) {
	void* hnd = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
	int res = 0;
	if (hnd) {
		cpuCore*(*getCore)();
		getCore = dlsym(hnd, "getCore");
		if (!getCore) {
			printf("%s\n",dlerror());
			dlclose(hnd);
		} else {
			if (cpu->lib) {
				dlclose(cpu->libhnd);
			}
			cpu->lib = 1;
			cpu->libhnd = hnd;
			cpuCore* core = getCore();
			cpuSetCore(cpu, core);
			res = 1;
		}
	} else {
		printf("%s\n",dlerror());
	}
	return res;
}

CPU* cpuCreate(int type, cbmr fmr, cbmw fmw, cbir fir, cbiw fiw, cbiack frq, cbirq xirq, void* dt) {
	CPU* cpu = (CPU*)malloc(sizeof(CPU));
	memset(cpu, 0x00, sizeof(CPU));
	cpu->xptr = dt;
	cpu->mrd = fmr;
	cpu->mwr = fmw;
	cpu->ird = fir;
	cpu->iwr = fiw;
	cpu->xack = frq;
	cpu->xirq = xirq;
	cpuSetType(cpu, type);
	return cpu;
}

void cpuDestroy(CPU* cpu) {
	free(cpu);
}

// disasm

static const char halfByte[] = "0123456789ABCDEF";
static char tmpbuf[1024];

xMnem cpuDisasm(CPU* cpu, int adr, char* buf, cbdmr mrd, void* data) {
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
		// mn.oadr = -1;
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
						// *buf++ = '#';
						buf += sprintf(buf, "%o", dtw);
						break;
					case ':':
						*buf++ = ':';
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

	xAsmScan res = cpu->asmbl(adr, cbuf, buf);
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
static char* dumFlags = "--------";

xRegBunch cpuGetRegs(CPU* cpu) {
	xRegBunch bunch;
	int i;
	for (i = 0; i < 32; i++) {
		bunch.regs[i].name = noname;
		bunch.regs[i].id = REG_NONE;
		bunch.regs[i].value = 0;
		bunch.regs[i].base = 0;
		bunch.regs[i].type = 0;
	}
	bunch.flags = dumFlags;
	//memcpy(bunch.flags, "--------", 8);
	if (cpu->getregs) cpu->getregs(cpu, &bunch);
	return bunch;
}

int reg_get_value(CPU* cpu, xRegDsc* dsc) {
	int res = -1;
	if (dsc->type & REG_SEG) {
		res = ((xSegPtr*)((cpu + dsc->offset)))->idx & 0xffff;
	} else {
		void* ptr = ((void*)cpu) + dsc->offset;
		switch(dsc->type & REG_TMASK) {
			// case REG_BIT: res = (*(unsigned char*)ptr) & 1; break;
			case REG_BYTE: res = (*(unsigned char*)ptr) & 0xff; break;
			case REG_WORD: res = (*(unsigned short*)ptr) & 0xffff; break;
			case REG_24: res = (*(int*)ptr) & 0xffffff; break;
			case REG_32: res = (*(int*)ptr) & 0xffffffff; break;
		}
	}
	return res;
}

xRegister cpuGetReg(CPU* cpu, int id) {
	xRegister reg;
	reg.type = REG_NONE;
	xRegDsc* rt = cpu->core->rdsctab;
	int i = 0;
	int work = 1;
	while (work && (rt[i].id != REG_NONE)) {
		if (rt[i].id == id) {
			reg.id = id;
			reg.type = rt[i].type;
			reg.name = rt[i].name;
			reg.value = reg_get_value(cpu, &rt[i]);		// TODO: for segments - value=selector, base=address
			reg.base = 0;
			work = !(reg.value < 0);
		}
		i++;
	}
	return reg;
}

// f is pointer to bool variable: false if register doesn't exists
int cpu_get_reg(CPU* cpu, const char* name, bool* f) {
	int res = -1;
#if 1
	bool err = true;
	xRegDsc* rt = cpu->core->rdsctab;
	int i = 0;
	int work = 1;
	while (work && (rt[i].id != REG_NONE)) {
		if (!strcmp(name, rt[i].name) && (rt[i].offset != 0)) {
			res = reg_get_value(cpu, &rt[i]);
			if (res >= 0) {
				work = 0;
				err = false;
			}
		}
		i++;
	}
	if (f != NULL) {*f = err;}
#else
	xRegBunch bunch = cpuGetRegs(cpu);
	for (int i = 0; (i < 32) && (res == -1); i++) {
		if (bunch.regs[i].id != REG_NONE) {
			if (!strcmp(name, bunch.regs[i].name)) {
				res = bunch.regs[i].value;
			}
		}
	}
#endif
	return res;
}

bool cpu_set_reg(CPU* cpu, const char* name, int val) {
	int i = 0;
	int work = 1;
	void* ptr;
	xRegDsc* rt = cpu->core->rdsctab;
	while (work && (rt[i].id != REG_NONE)) {
		if (!strcmp(name, rt[i].name) && (rt[i].offset != 0) && !(rt[i].type & REG_SEG)) {
			ptr = ((void*)cpu) + rt[i].offset;
			work = 0;
			switch(rt[i].type & REG_TMASK) {
				case REG_BYTE: *(unsigned char*)ptr = val & 0xff; break;
				case REG_WORD: *(unsigned short*)ptr = val & 0xffff; break;
				case REG_24: *(int*)ptr = val & 0xffffff; break;
				case REG_32: *(int*)ptr = val & 0xffffffff; break;
				default: work = 1; break;
			}
		}
		i++;
	}
	return work ? false : true;
}

void cpuSetRegs(CPU* cpu, xRegBunch bunch) {
	cpu->setregs(cpu, bunch);
}
