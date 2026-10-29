#include "i80286.h"

// segment descriptor flag
// b7:		1 if segment presented in memory / 0 - invalid
// b5,6		priv.level. max(caller,request) must be not greater
// b4		1 if it's code/data segment / 0 if special
// b1,2,3	type
// b0		accessed. set this flag if there is using of segment

xSegPtr i286_cash_seg(CPU*, unsigned short);
xSegPtr i286_cash_seg_a(CPU*, int);
void i286_push(CPU*, unsigned short);
void x86_exception(CPU*, int, int);

// unrecognized opcode
void i286_0Fxx(CPU* cpu) {
	x86_exception(cpu, I286_INT_UD, 0);
}

// 0f 00 /0: sldt ew (prt.mode only)	[ew] = ldtr.idx
void i286_0F000(CPU* cpu) {
	i286_wr_ea(cpu, cpu->ldtr.idx, 1);
}

// 0f 00 /1 str ew (prt.mode only)	[ew] = task.idx
void i286_0F001(CPU* cpu) {
	i286_wr_ea(cpu, cpu->tsdr.idx, 1);
}

xSegPtr i286_get_dsc(CPU* cpu, int sel) {
	xSegPtr dt;
	if (sel & ~0xffff) {
		sel &= 0xffff;
		cpu->tmpi = cpu->idtr.base;
		cpu->twrd = cpu->idtr.limit;
	} else {
		cpu->tmpi = (sel & 4) ? cpu->ldtr.base : cpu->gdtr.base;
		cpu->twrd = (sel & 4) ? cpu->ldtr.limit : cpu->gdtr.limit;
	}
	if (sel > cpu->twrd) {
		dt.idx = -1;		// index out of bounds
	} else {
		cpu->tmpi += (sel & 0xfff8);
		dt = i286_cash_seg_a(cpu, cpu->tmpi);
		dt.idx = sel;
	}
	return dt;
}

// 0f 00 /2 lldt ew (prt.mode only)	ldtr.idx = [ew], update base,size fields
void i286_0F002(CPU* cpu) {
	cpu->tmpdr = i286_get_dsc(cpu, cpu->tmpw & ~4);	// GDT table only
	if (cpu->tmpdr.idx < 0) {			// index out of bounds -> #GP(sel)
		x86_exception(cpu, I286_INT_GP, cpu->tmpw);
//	} else if (cpu->tmpdr.ar != 2) {		// type is not 2(ldt) -> #GP(sel)
//		x86_exception(cpu, I286_INT_GP, cpu->tmpw);
	} else if (!cpu->tmpdr.pr) {			// not present -> #NP(sel)
		x86_exception(cpu, I286_INT_NP, cpu->tmpw);
	} else {
		cpu->ldtr = cpu->tmpdr;
	}
}

// 0f 00 /3 ltr ew (prt mode only)	load task register
void i286_0F003(CPU* cpu) {
	cpu->tmpdr = i286_get_dsc(cpu, cpu->tmpw);
	if (cpu->tmpdr.idx < 0) {
		x86_exception(cpu, I286_INT_NP, cpu->tmpw);
//	} else if (cpu->tmpdr.ar != 1) {		// type is not 1(tss) -> #GP(sel)	1,3,9,11
//		x86_exception(cpu, I286_INT_GP, cpu->tmpw);
	} else if (!cpu->tmpdr.pr) {			// not present -> #NP(sel)
		x86_exception(cpu, I286_INT_NP, cpu->tmpw);
	} else {
		cpu->tsdr = cpu->tmpdr;
	}
}

// 0f 00 /4 verr ew (prt mode oly)	verify segment to read
void i286_0F004(CPU* cpu) {
	cpu->tmpdr = i286_get_dsc(cpu, cpu->tmpw);
	cpu->flgZ = 0;;
	if (cpu->tmpdr.idx < 0) return;		// index out of bounds
	if (!cpu->tmpdr.pr) return;		// not present
	if (!(cpu->tmpdr.ar & 0x10)) return;	// special system segment
	if ((cpu->tmpw & 3) > cpu->tmpdr.pl) return;	// dpl < rpl
	if ((cpu->tmpdr.ar & 0x08) && !(cpu->tmpdr.ar & 2)) return;	// code segment, not readable
	cpu->flgZ = 1;
}

// 0f 00 /5 verw ew (prt mode only)	verify segment to write
void i286_0F005(CPU* cpu) {
	cpu->tmpdr = i286_get_dsc(cpu, cpu->tmpw);
	cpu->flgZ = 0;;
	if (cpu->tmpdr.idx < 0) return;		// index out of bounds
	if (!cpu->tmpdr.pr) return;	// not present
	if (!(cpu->tmpdr.ar & 0x10)) return;	// special system segment
	if ((cpu->tmpw & 3) > cpu->tmpdr.pl) return;	// dpl < rpl
	if (cpu->tmpdr.ar & 0x08) return;		// code segment is not writeable
	if (!(cpu->tmpdr.ar & 2)) return;		// not writeable
	cpu->flgZ = 1;
}

cbcpu i286_0f00_tab[8] = {
	i286_0F000,i286_0F001,i286_0F002,i286_0F003,
	i286_0F004,i286_0F005,i286_0Fxx,i286_0Fxx
};

void i286_0F00(CPU* cpu) {
	if (!(cpu->regMSW & I286_FPE)) {
		x86_exception(cpu, I286_INT_UD, -1);
	} else {
		i286_rd_ea(cpu, 1);
		i286_0f00_tab[(cpu->regMOD >> 3) & 7](cpu);
	}
}

// 0f 01 /0: sgdt ew
void i286_0F010(CPU* cpu) {
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, cpu->gdtr.limit & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, (cpu->gdtr.limit >> 8) & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, cpu->gdtr.base & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, (cpu->gdtr.base >> 8) & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr, (cpu->gdtr.base >> 16) & 0xff);
}

// 0f 01 /1 sidt ew
void i286_0F011(CPU* cpu) {
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, cpu->idtr.limit & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, (cpu->idtr.limit >> 8) & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, cpu->idtr.base & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr++, (cpu->idtr.base >> 8) & 0xff);
	i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr, (cpu->idtr.base >> 16) & 0xff);
}

void i286_rd_ea40(CPU* cpu) {
	cpu->ltw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr++);
	cpu->htw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr++);
	cpu->lwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr++);
	cpu->hwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr++);
	cpu->tmp = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr);
}

// 0f 01 /2 lgdt eq	set gdtr (40 bit)
void i286_0F012(CPU* cpu) {
	if (cpu->ea.reg) {
		x86_exception(cpu, I286_INT_UD, 0);		// ea is register
	} else {
		i286_rd_ea40(cpu);
		cpu->gdtr.limit = cpu->tmpw;
		cpu->gdtr.base = ((cpu->tmp << 16) | cpu->twrd) & 0x00ffffff;
	}
}

// 0f 01 /3 lidt ew	set idtr (40 bit)
void i286_0F013(CPU* cpu) {
	if (cpu->ea.reg) {
		x86_exception(cpu, I286_INT_UD, 0);
	} else {
		i286_rd_ea40(cpu);
		cpu->idtr.limit = cpu->tmpw;
		cpu->idtr.base = ((cpu->tmp << 16) | cpu->twrd) & 0x00ffffff;
	}
}

// 0f 01 /4 smsw ew	[ea] = msw
void i286_0F014(CPU* cpu) {
	i286_wr_ea(cpu, cpu->regMSW, 1);
}

// 0f 01 /6 lmsw ew	msw = [ea]
void i286_0F016(CPU* cpu) {
	cpu->regMSW = (cpu->regMSW & 1) | cpu->tmpw;		// can't set real mode by software
	if (cpu->regMSW & I286_FPE) {
		x86_set_mode(cpu, X86_PROT);
	}
}

cbcpu i286_0f01_tab[8] = {
	i286_0F010,i286_0F011,i286_0F012,i286_0F013,
	i286_0F014,i286_0Fxx,i286_0F016,i286_0Fxx
};

void i286_0F01(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	i286_0f01_tab[(cpu->regMOD >> 3) & 7](cpu);
}

static int lar_allowed_ar[16] = {0,1,1,1,1,1,0,0,0,1,0,1,1,0,0,0};

// 0F 02 /r : lar rw,ew		rw = (seg.descriptor flags << 8)
void i286_0F02(CPU* cpu) {
	if (!(cpu->regMSW & I286_FPE)) {
		x86_exception(cpu, I286_INT_UD, -1);		// not present in real mode
	} else {
		i286_rd_ea(cpu, 1);	// tmpw = ew
		cpu->tmpi = (cpu->tmpw & 4) ? cpu->ldtr.base : cpu->gdtr.base;
		cpu->twrd = (cpu->tmpw & 4) ? cpu->ldtr.limit : cpu->gdtr.limit;
		if ((cpu->tmpw & 0xfff8) > cpu->twrd) {
			cpu->flgZ = 0;
		} else {
			cpu->tmpi += (cpu->tmpw & 0xfff8);
			xSegPtr s = i286_cash_seg_a(cpu, cpu->tmpi);	// load segment dsc
			if (!lar_allowed_ar[s.ar & 0x0f]) {
				cpu->flgZ = 0;
			} else if ((cpu->cs.pl > s.pl) || ((cpu->tmpw & 3) > s.pl)) {
				cpu->flgZ = 0;
			} else {
				cpu->flgZ = 1;
				cpu->twrd = (s.flag & 0xff) << 8;	// return value
				i286_set_reg(cpu, cpu->twrd, 1);
			}
//			cpu->tmpi += 5;
//			cpu->htw = cpu_mrd(cpu, cpu->tmpi);
//			cpu->ltw = 0;
//			i286_set_reg(cpu, cpu->tmpw, 1);
		}
	}
}

// 0F 03 /r : lsl rw,ew		rw = seg.descriptor limit
void i286_0F03(CPU* cpu) {
	if (!(cpu->regMSW & I286_FPE)) {
		x86_exception(cpu, I286_INT_UD, -1);		// not present in treal mode
	} else {
		i286_rd_ea(cpu, 1);
		cpu->tmpi = (cpu->tmpw & 4) ? cpu->ldtr.base : cpu->gdtr.base;
		cpu->twrd = (cpu->tmpw & 4) ? cpu->ldtr.limit : cpu->gdtr.limit;
		if ((cpu->tmpw & 0xfff8) > cpu->twrd) {
			cpu->flgZ = 0;
		} else {
			cpu->tmpi += (cpu->tmpw & 0xfff8);
			xSegPtr s = i286_cash_seg_a(cpu, cpu->tmpi);
			if (!lar_allowed_ar[s.ar & 0x0f]) {
				cpu->flgZ = 0;
			} else if ((cpu->cs.pl > s.pl) || ((cpu->tmpw & 3) > s.pl)) {
				cpu->flgZ = 0;
			} else {
				cpu->flgZ = 1;
				cpu->twrd = s.limit;
				i286_set_reg(cpu, cpu->twrd, 1);
			}
//			cpu->ltw = cpu_mrd(cpu, cpu->tmpi);		// +0,1 : limit
//			cpu->htw = cpu_mrd(cpu, cpu->tmpi + 1);
//			i286_set_reg(cpu, cpu->tmpw, 1);
		}
	}
}

// 0F 06: clts			clear task segment flag
void i286_0F06(CPU* cpu) {
	cpu->regMSW &= ~I286_FTS;
}

opCode i286_0f_tab[256] = {
	{OF_MODCOM | OF_PRT | OF_WORD, 1, i286_0F00, 0, ":Q :e"},		// sldt,str,lldt,ltr,verr,verw,?,? ew
	{OF_MODCOM | OF_WORD, 1, i286_0F01, 0, ":W :e"},			// sgdt,sidt,lgdt,lidt,smsw,?,lmsw,? ew
	{OF_MODRM | OF_PRT | OF_WORD, 1, i286_0F02, 0, "lar :r,:e"},	// lar rw,ew
	{OF_MODRM | OF_PRT | OF_WORD, 1, i286_0F03, 0, "lsl :r,:e"},	// lsl rw,ew
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},				// (?) loadall
	{0, 1, i286_0F06, 0, "clts"},				// clts
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
	{0, 1, i286_0Fxx, 0, "undef"},
};
