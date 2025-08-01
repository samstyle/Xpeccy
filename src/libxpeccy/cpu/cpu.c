#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "cpu.h"

// TODO: cpu headers must be included cuz of callbacks, but flgX redefinition occurs
// buuuuuuuuut it doesn't matter from here  (no flgX macros used)
// undef redefined flgX after including
#include "1801vm1/1801vm1.h"
#undef flgZ
#include "i8080/i8080.h"
#undef flgT
#undef flgN
#include "i80286/i80286.h"
#undef flgC
#undef flgN
#undef flgZ
#include "LR35902/lr35902.h"
#undef flgC
#undef flgZ
#undef flgI
#undef flgD
#undef flgV
#undef flgN
#include "MOS6502/6502.h"
#undef flgN
#undef flgH
#undef flgZ
#include "Z80/z80.h"

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
void cpu_irq(CPU* cpu, int id) {cpu->xirq(id, cpu->xptr);}

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
int nil_get_flag(CPU* cpu) {return 0;}
void nil_set_flag(CPU* cpu, int v) {}

cpuCore dumCore = {CPU_NONE, 0, "none", nil_reg_tab, NULL, nil_reset, nil_exec, nil_asm, nil_mnem, nil_get_regs, nil_set_regs, nil_get_flag, nil_set_flag};

extern xRegDsc z80RegTab[];
extern xRegDsc m6502RegTab[];
extern xRegDsc lrRegTab[];
extern xRegDsc i8080RegTab[];
extern xRegDsc i286RegTab[];
extern xRegDsc pdp11RegTab[];

cpuCore cpuTab[] = {
	{CPU_Z80, 0,"Z80", z80RegTab, NULL, z80_reset, z80_exec, z80_asm, z80_mnem, z80_get_regs, z80_set_regs, z80_get_flag, z80_set_flag},
	{CPU_I8080, 0,"i8080", i8080RegTab, NULL, i8080_reset, i8080_exec, i8080_asm, i8080_mnem, i8080_get_regs, i8080_set_regs, i8080_get_flag, i8080_set_flag},
	{CPU_I8086, 0,"i8086", i286RegTab, NULL, i286_reset, i286_exec, i286_asm, i286_mnem, i286_get_regs, i286_set_regs, x86_get_flag, x86_set_flag},
	{CPU_I80186, 1,"i80186", i286RegTab, NULL, i286_reset, i286_exec, i286_asm, i286_mnem, i286_get_regs, i286_set_regs, x86_get_flag, x86_set_flag},
	{CPU_I80286, 2,"i80286", i286RegTab, NULL, i286_reset, i286_exec, i286_asm, i286_mnem, i286_get_regs, i286_set_regs, x86_get_flag, x86_set_flag},
	{CPU_LR35902, 0, "LR35902", lrRegTab, NULL, lr_reset, lr_exec, lr_asm, lr_mnem, lr_get_regs, lr_set_regs, lr_get_flag, lr_set_flag},
	{CPU_6502, 0, "MOS6502", m6502RegTab, NULL, m6502_reset, m6502_exec, m6502_asm, m6502_mnem, m6502_get_regs, m6502_set_regs, mos_get_flag, mos_set_flag},
	{CPU_VM1, 0, "1801VM1", pdp11RegTab, NULL, pdp11_reset, pdp11_exec, pdp11_asm, pdp11_mnem, pdp11_get_regs, pdp11_set_regs, pdp_get_flag, pdp_set_flag},
	{CPU_VM2, 1, "1801VM2", pdp11RegTab, NULL, pdp11_reset, pdp11_exec, pdp11_asm, pdp11_mnem, pdp11_get_regs, pdp11_set_regs, pdp_get_flag, pdp_set_flag},
	{CPU_NONE, 0, "none", nil_reg_tab, NULL, nil_reset, nil_exec, nil_asm, nil_mnem, nil_get_regs, nil_set_regs, nil_get_flag, nil_set_flag}
};

cpuCore* findCore(int type) {
	int idx = 0;
	while ((cpuTab[idx].type != CPU_NONE) && (cpuTab[idx].type != type)) {
		idx++;
	}
	return &cpuTab[idx];
}

void cpuSetCore(CPU* cpu, cpuCore* core) {
	cpu->core = core;
	cpu->type = core->type;
	cpu->gen = core->gen;
	if (core->init) {
		core->init(cpu);
	}
}

// new
int cpu_open_lib(CPU* cpu, const char* dir, const char* fname) {
	int res = 0;
	if (cpu->lib) {					// some library opened
		if (!strcmp(cpu->libname, fname)) {	// same lib
			res = 1;
		} else {
			dlclose(cpu->libhnd);
			cpu->lib = 0;
		}
	}
	if (!res) {
		char* buf = (char*)malloc(strlen(dir) + strlen(fname) + 2);
		strcpy(buf, dir);
		strcat(buf, "/");
		strcat(buf, fname);
		void* hnd = dlopen(buf, RTLD_LAZY | RTLD_GLOBAL);	// load library
		if (hnd) {
			cpuCore*(*getCore)();				// check callback
			getCore = dlsym(hnd, "getCore");
			if (getCore) {					// success
				cpu->lib = 1;
				cpu->libhnd = hnd;
				cpu->libname = realloc(cpu->libname, strlen(fname) + 1);
				strcpy(cpu->libname, fname);
				res = 1;
			}
		}
		free(buf);
	}
	return res;
}

void cpu_close_lib(CPU* cpu) {
	if (cpu->lib) {
		dlclose(cpu->libhnd);
		cpu->lib = 0;
	}
}

// if fname==NULL take it from built-in table
// e.g	cpu_set_type(cpu, "name", NULL, NULL) = built-in
//	cpu_set_type(cpu, "name", dir, file) = from library
int cpu_set_type(CPU* cpu, const char* name, const char* dir, const char* fname) {
	cpuCore* tab;
	int res = 0;
	if (fname) {
		if (cpu_open_lib(cpu, dir, fname)) {
			cpuCore*(*foo)();
			foo = dlsym(cpu->libhnd, "getCore");
			tab = foo();
		} else {
			tab = cpuTab;
		}
	} else {
		tab = cpuTab;
		cpu_close_lib(cpu);
	}
	int i = 0;
	while ((tab[i].type != CPU_NONE) && strcmp(tab[i].name, name)) {
		i++;
	}
	if (tab[i].type != CPU_NONE) {		// found?
		cpuSetCore(cpu, &tab[i]);
		res = 1;
	}
	return res;
}

int cpuSetType(CPU* cpu, int type) {
	cpuCore* core = findCore(type);
	if (core != NULL) {
		if (cpu->lib) {
			dlclose(cpu->libhnd);
			cpu->lib = 0;
		}
		cpuSetCore(cpu, core);
	} else {
		cpuSetCore(cpu, &dumCore);
	}
	return core ? 1 : 0;
}

/*
int cpuSetLib(CPU* cpu, const char* dir, const char* fname) {
	int res = 0;
	if (cpu_open_lib(cpu, dir, fname)) {
		cpuCore*(*foo)();
		foo = dlsym(cpu->libhnd, "getCore");
		cpuCore* tab = foo();
		cpuSetCore(cpu, &tab[0]);
		res = 1;
	} else {
		printf("%s\n",dlerror());
	}
	return res;
}
*/

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
	if (cpu->lib) cpu_close_lib(cpu);
	free(cpu);
}

void cpu_reset(CPU* cpu) {
	if (!cpu->core) return;
	if (!cpu->core->reset) return;
	cpu->core->reset(cpu);
}

int cpu_exec(CPU* cpu) {
	if (!cpu->core) return 1;
	if (!cpu->core->exec) return 1;
	return cpu->core->exec(cpu);
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
		mn = cpu->core->mnem(cpu, adr, mrd, data);
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

	xAsmScan res = cpu->core->asmbl(adr, cbuf, buf);
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

int cpu_get_flag(CPU* cpu) {
	return cpu->core->getflag ? cpu->core->getflag(cpu) : -1;
}

void cpu_set_flag(CPU* cpu, int v) {
	if (cpu->core->setflag)
		cpu->core->setflag(cpu, v);
}

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
	if (cpu->core->getregs) cpu->core->getregs(cpu, &bunch);
	return bunch;
}

int reg_get_value(CPU* cpu, xRegDsc* dsc) {
	int res = -1;
	if (dsc->flag & REG_SEG) {
		res = ((xSegPtr*)((cpu + dsc->offset)))->idx & 0xffff;
	} else if (dsc->offset == 0) {		// is flag
		res = cpu_get_flag(cpu);
	} else {
		void* ptr = ((void*)cpu) + dsc->offset;
		switch(dsc->type) {
			case REG_BIT: res = !!(*(bool*)ptr); break;
			case REG_BYTE: res = (*(unsigned char*)ptr) & 0xff; break;
			case REG_WORD: res = (*(unsigned short*)ptr) & 0xffff; break;
			case REG_24: res = (*(int*)ptr) & 0xffffff; break;
			case REG_32: res = (*(int*)ptr) & 0xffffffff; break;
		}
	}
	return res;
}

int reg_set_value(CPU* cpu, xRegDsc* dsc, int val) {
	int res = 1;
	if (dsc->flag & REG_SEG) return 0;		// what to do with x86 segment registers?
	if (dsc->offset == 0) {
		cpu_set_flag(cpu, val);
	} else {
		void* ptr = ((void*)cpu) + dsc->offset;
		switch(dsc->type) {
			case REG_BIT: *(bool*)ptr = !!val; break;
			case REG_BYTE: *(unsigned char*)ptr = val & 0xff; break;
			case REG_WORD: *(unsigned short*)ptr = val & 0xffff; break;
			case REG_24: *(int*)ptr = val & 0xffffff; break;
			case REG_32: *(int*)ptr = val & 0xffffffff; break;
			default: res = 0; break;		// fail, unknown type
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
			reg.flag = rt[i].flag;
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
	return res;
}

bool cpu_set_reg(CPU* cpu, const char* name, int val) {
	int i = 0;
	int work = 1;
//	void* ptr;
	xRegDsc* rt = cpu->core->rdsctab;
	while (work && (rt[i].id != REG_NONE)) {
		if (!strcmp(name, rt[i].name) && (rt[i].offset != 0) && !(rt[i].flag & REG_SEG)) {	// TODO: offset==0 -> setflag (no it's not: AF for example)
			work = !reg_set_value(cpu, &rt[i], val);	// 0 if succes, 1 if error
/*
			ptr = ((void*)cpu) + rt[i].offset;
			work = 0;
			switch(rt[i].type) {
				case REG_BIT: *(bool*)ptr = !!val; break;
				case REG_BYTE: *(unsigned char*)ptr = val & 0xff; break;
				case REG_WORD: *(unsigned short*)ptr = val & 0xffff; break;
				case REG_24: *(int*)ptr = val & 0xffffff; break;
				case REG_32: *(int*)ptr = val & 0xffffffff; break;
				default: work = 1; break;
			}
*/
		}
		i++;
	}
	return work ? false : true;
}

void cpuSetRegs(CPU* cpu, xRegBunch bunch) {
	if (cpu->core->setregs) {
		cpu->core->setregs(cpu, bunch);
	}
}

xRegDsc* cpu_find_onmsk(CPU* cpu, int msk) {
	int i = 0;
	int work = 1;
	xRegDsc* rt = cpu->core->rdsctab;
	while (work && (rt[i].id != REG_NONE)) {
		if (rt[i].flag & msk) {
			work = 0;
		} else {
			i++;
		}
	}
	return work ? NULL : &rt[i];
}

int cpu_get_onmsk(CPU* cpu, int msk) {
	xRegDsc* rd = cpu_find_onmsk(cpu, msk);
	return rd ? reg_get_value(cpu, rd) : -1;
}

void cpu_set_onmsk(CPU* cpu, int msk, int val) {
	xRegDsc* rd = cpu_find_onmsk(cpu, msk);
	if (rd) reg_set_value(cpu, rd, val);
}

int cpu_get_sp(CPU* cpu) {return cpu_get_onmsk(cpu, REG_SP);}
int cpu_get_pc(CPU* cpu) {return cpu_get_onmsk(cpu, REG_PC);}
void cpu_set_pc(CPU* cpu, int val) {cpu_set_onmsk(cpu, REG_PC, val);}
