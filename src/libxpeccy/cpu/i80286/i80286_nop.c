#include "i80286.h"
#include <stdio.h>

// TODO: protected mode
// segment:
//	b0,1 = rpl (request privelege level)
//	b2 = table type (0:GDT/1:current LDT)
//	b3..15 = descriptor index in table
// real.addr = descr.base(24 bit)+offset(adr)
// flag:
// [7:P][6,5:DPL][4:1][3:1][2:C][1:R][0:A]	code; R:segment can be readed (not only imm adr)
// [7:P][6,5:DPL][4:1][3:0][2:D][1:W][0:A]	data; W:segment is writeable
// [7:P][6,5:DPL][4:0][3-0:TYPE]		system (gates)
// rd/wr with segment P=0 -> interrupt

// https://wiki.osdev.org/Descriptor
// segment types:
// 1.1CRA - code
// 1.0EWA - data
// system segments:
// 1 - TSS 16bit (available)
// 2 - LDT
// 3 - TSS 16bit (busy)
// 4 - 16bit call gate
// 5 - Task gate
// 6 - 16bit interrupt gate
// 7 - 16bit trap gate
// 9 - TSS 32/64bit (available)
// 11 - TSS 32/64bit (busy)
// 12 - 32bit call gate
// 14 - 32bit interrupt gate
// 15 - 32bit trap gate

// gate/trap descriptor:
// +0,1	offset
// +2,3 code/TSS selector
// +4	bit0..4: number of words transported from caller stack to task stack (if dpl != cpl) - for call gate only
// +5	gate segment flags [7:P][6,5:DPL][4:0][3-0:TYPE]
// +6,7	offset bit 16..31 [32bit+ only][0]

// TSS descriptor
// +0,1		back link to previous TSS
// +2,3,4,5	SP,SS for CPL0 *
// +6,7,8,9	SP,SS for CPL1 *
// +10,11,12,13	SP,SS for CPL2 *
// +14,15	IP (entry point)
// +16,17	flag
// +18,19	ax
// +20,21	cx
// +22,23	dx
// +24,25	bx
// +26,27	sp
// +28,29	bp
// +30,31	si
// +32,33	di
// +34,35	ES selector
// +36,37	CS selector
// +38,39	SS selector
// +40,41	DS selector
// +42,43	LDT selector *
// * - doesn't changed during task switch

// NOTE: x86 parity counts on LSB

int i286_check_segment_limit(CPU* cpu, xSegPtr* seg, unsigned short adr) {
	if (!(cpu->msw & I286_FPE)) return 1;
	if (seg->limit < adr) return 0;
	return 1;
}

int i286_check_segment_exec(CPU* cpu, xSegPtr* seg) {
	if (!(cpu->msw & I286_FPE)) return 1;
	if (!(seg->ar & 0x10)) return 0;	// system segment
	if (!(seg->ar & 0x08)) return 0;	// data segment
	return 1;				// code segment
}

int i286_check_segment_rd(CPU* cpu, xSegPtr* seg) {
	if (!(cpu->msw & I286_FPE)) return 1;
	if (!(seg->ar & 0x10)) return 0;	// system segment
	if (!(seg->ar & 0x08)) return 1;	// data segment
	if (seg->ar & 2) return 1;		// readable code segment
	return 0;
}

int i286_check_segment_wr(CPU* cpu, xSegPtr* seg) {
	if (!(cpu->msw & I286_FPE)) return 1;
	if (!(seg->ar & 0x10)) return 0;	// system segment
	if (seg->ar & 0x08) return 0;		// code segment
	if (seg->ar & 0x02) return 1;		// writeable data segment
	return 0;
}

unsigned char i286_mrd(CPU* cpu, xSegPtr seg, int rpl, unsigned short adr) {
	return cpu->x86mrd(cpu, seg, rpl, adr);
}

void i286_mwr(CPU* cpu, xSegPtr seg, int rpl, unsigned short adr, int val) {
	cpu->x86mwr(cpu, seg, rpl, adr, val);
}

// system mrd/mwr, w/o checks

unsigned char i286_sys_mrd(CPU* cpu, xSegPtr seg, unsigned short adr) {
	return cpu->mrd(seg.base + adr, 0, cpu->xptr) & 0xff;
}

unsigned short i286_sys_mrdw(CPU* cpu, xSegPtr seg, unsigned short adr) {
	unsigned char rl = cpu->mrd(seg.base + adr, 0, cpu->xptr) & 0xff;
	unsigned char rh = cpu->mrd(seg.base + adr + 1, 0, cpu->xptr) & 0xff;
	return (rh << 8) | rl;
}

void i286_sys_mwr(CPU* cpu, xSegPtr seg, unsigned short adr, unsigned char v) {
	cpu->mwr(seg.base + adr, v, cpu->xptr);
}

void i286_sys_mwrw(CPU* cpu, xSegPtr seg, unsigned short adr, unsigned short v) {
	cpu->mwr(seg.base + adr, v & 0xff, cpu->xptr);
	cpu->mwr(seg.base + adr, (v >> 8) & 0xff, cpu->xptr);
}

// real mode

unsigned char i286_fetch_real(CPU* cpu) {
	unsigned char res = cpu->mrd(cpu->cs.base + cpu->pc, 1, cpu->xptr) & 0xff;
	cpu->pc++;
	return res;
}

unsigned char i286_mrd_real(CPU* cpu, xSegPtr seg, int rpl, unsigned short adr) {
	if (rpl && (cpu->seg.idx >= 0)) seg = cpu->seg;
	cpu->t++;
	return cpu->mrd(seg.base + adr, 0, cpu->xptr) & 0xff;
}

void i286_mwr_real(CPU* cpu, xSegPtr seg, int rpl, unsigned short adr, int val) {
	if (rpl && (cpu->seg.idx >= 0)) seg = cpu->seg;
	cpu->t++;
	cpu->mwr(seg.base + adr, val, cpu->xptr);
}

// stack (TODO: real/prt mode stack rd/wr procedures)

void i286_push(CPU* cpu, unsigned short w) {
	if ((cpu->msw & I286_FPE) && (cpu->sp < 2)) {
		THROW(I286_INT_SS);
	}
	cpu->sp -= 2;
	i286_mwr(cpu, cpu->ss, 0, cpu->sp, w & 0xff);
	i286_mwr(cpu, cpu->ss, 0, cpu->sp + 1, (w >> 8) & 0xff);
}

unsigned short i286_pop(CPU* cpu) {
	if ((cpu->msw & I286_FPE) && (cpu->sp + 2 > cpu->ss.limit)) {
		THROW(I286_INT_SS);
	}
	PAIR(w,h,l) rx;
	rx.l = i286_mrd(cpu, cpu->ss, 0, cpu->sp);
	rx.h = i286_mrd(cpu, cpu->ss, 0, cpu->sp + 1);
	cpu->sp += 2;
	return rx.w;
}

// protected mode

unsigned char i286_fetch_prt(CPU* cpu) {
	unsigned char res = 0xff;
//	if (i286_check_segment_exec(cpu, &cpu->cs)) {		// check it when CS changed
		if (i286_check_segment_limit(cpu, &cpu->cs, cpu->pc)) {
			res = cpu->mrd(cpu->cs.base + cpu->pc, 1, cpu->xptr) & 0xff;
			cpu->pc++;
		} else {
			THROW(I286_INT_SL);
		}
//	} else {
//		i286_push(cpu, cpu->cs.idx & 0xffff);
//		THROW(I286_INT_GP);
//	}
	return res;
}

unsigned char i286_mrd_prt(CPU* cpu, xSegPtr seg, int rpl, unsigned short adr) {
	unsigned char res = 0xff;
	if (rpl && (cpu->seg.idx >= 0)) seg = cpu->seg;
	if (i286_check_segment_rd(cpu, &seg)) {
		if (i286_check_segment_limit(cpu, &seg, adr)) {
			res = cpu->mrd(seg.base + adr, 0, cpu->xptr) & 0xff;
		} else {
			THROW(I286_INT_SL);
		}
	} else {
		THROW_EC(I286_INT_GP, seg.idx);
	}
	return res;
}

void i286_mwr_prt(CPU* cpu, xSegPtr seg, int rpl, unsigned short adr, int val) {
	if (rpl && (cpu->seg.idx >= 0)) seg = cpu->seg;
	if (i286_check_segment_wr(cpu, &seg)) {
		if (i286_check_segment_limit(cpu, &seg, adr)) {
			cpu->mwr(seg.base + adr, val, cpu->xptr);
		} else {
			cpu->pc = cpu->oldpc;
			THROW(I286_INT_SL);
		}
	} else {
		THROW_EC(I286_INT_GP, seg.idx);
	}
}

// iord/wr

unsigned short i286_ird(CPU* cpu, int adr) {
	return cpu->ird(adr, cpu->xptr) & 0xffff;
}

void i286_iwr(CPU* cpu, int adr, int val, int w) {
	cpu->wrd = !!w;
	cpu->iwr(adr, val, cpu->xptr);
	cpu->wrd = 0;
}

// set cpu mode

void x86_set_mode(CPU* cpu, int m) {
	switch(m) {
		case X86_REAL:
			cpu->x86fetch = i286_fetch_real;
			cpu->x86mrd = i286_mrd_real;
			cpu->x86mwr = i286_mwr_real;
			break;
		case X86_PROT:
			cpu->x86fetch = i286_fetch_prt;
			cpu->x86mrd = i286_mrd_prt;
			cpu->x86mwr = i286_mwr_prt;
			break;
	}
}

// imm: byte from ip
unsigned char i286_rd_imm(CPU* cpu) {
//	unsigned char res = i286_mrd(cpu, cpu->cs, 0, cpu->pc);
//	cpu->pc++;
//	return res;
	return cpu->x86fetch(cpu);
}

// immw:word from ip
unsigned short i286_rd_immw(CPU* cpu) {
	PAIR(w,h,l) rx;
	rx.l = i286_rd_imm(cpu);
	rx.h = i286_rd_imm(cpu);
	return rx.w;
}

// set segment registers

// by address of descriptor in memory
xSegPtr i286_cash_seg_a(CPU* cpu, int adr) {
	xSegPtr p;
	PAIR(w,h,l)off;
	unsigned short tmp;
	off.l = cpu->mrd(adr++, 0, cpu->xptr);	// limit:16
	off.h = cpu->mrd(adr++, 0, cpu->xptr);
	p.limit = off.w;
	off.l = cpu->mrd(adr++, 0, cpu->xptr);	// base:24
	off.h = cpu->mrd(adr++, 0, cpu->xptr);
	tmp = cpu->mrd(adr++, 0, cpu->xptr);
	p.base = (tmp << 16) | off.w;
	tmp = cpu->mrd(adr, 0, cpu->xptr);	// flags:8
	p.ar = tmp & 0x1f;
	p.pl = (tmp >> 5) & 3;
	p.pr = !!(tmp & 0x80);
	p.sys = !(tmp & 0x10);
	p.code = !!((tmp & 0x18) == 0x18);
	return p;
}

// by selector (GDT/LDT)
xSegPtr i286_cash_seg(CPU* cpu, unsigned short val) {
	xSegPtr p;
	int adr;
	unsigned short lim;
	p.idx = val;
	if (cpu->msw & I286_FPE) {
		if (val & 4) {
			adr = cpu->ldtr.base;
			lim = cpu->ldtr.limit;
		} else {
			adr = cpu->gdtr.base;
			lim = cpu->gdtr.limit;
		}
		val &= 0xfff8;					// offset in a table
		if (val > lim) {				// check gdt/ldt segment limit
			THROW_EC(I286_INT_GP, val);
		} else {
			adr += val;
			p = i286_cash_seg_a(cpu, adr);
		}
	} else {
		//p.flag = 0xf2;		// present, dpl=3, segment, data, writeable
		p.base = val << 4;
		p.limit = 0xffff;
	}
	p.idx = val;
	return p;
}

// switch task (protected mode)
// tsss - task switch segment selector
// nest - nested switch or not

void i286_switch_task(CPU* cpu, int tsss, int nest, int iret) {
	if (!(tsss & 0xfff8)) {		// segment 0
		THROW_EC(I286_INT_GP, 0);
	} else if (tsss & 4) {		// LDT
		THROW_EC(I286_INT_GP, tsss);
	} else {
		xSegPtr seg = i286_cash_seg(cpu, tsss);
		if (seg.pr) {		// present
			int dpl = seg.pl;
			int rpl = tsss & 3;
			int cpl = cpu->cs.pl;
			int f = 0;
			if (!iret && ((dpl < cpl) || (rpl < cpl))) {	// privelegies (skip if IRET)
				THROW_EC(I286_INT_GP, tsss);
			} else if (!iret && (seg.ar != 1)) {	// TSS busy (skip if IRET)
				THROW_EC(I286_INT_GP, tsss);
			} else {
				// set new TSS busy
				int adr = cpu->gdtr.base + (tsss & 0xfff8);
				f = cpu->mrd(adr + 5, 0, cpu->xptr);
				f |= 2;
				cpu->mwr(adr + 5, f, cpu->xptr);
				// save current state to current TSS
				i286_sys_mwrw(cpu, cpu->tsdr, 14, cpu->pc);
				i286_sys_mwrw(cpu, cpu->tsdr, 16, cpu->f);
				i286_sys_mwrw(cpu, cpu->tsdr, 18, cpu->ax);
				i286_sys_mwrw(cpu, cpu->tsdr, 20, cpu->cx);
				i286_sys_mwrw(cpu, cpu->tsdr, 22, cpu->dx);
				i286_sys_mwrw(cpu, cpu->tsdr, 24, cpu->bx);
				i286_sys_mwrw(cpu, cpu->tsdr, 26, cpu->sp);
				i286_sys_mwrw(cpu, cpu->tsdr, 28, cpu->bp);
				i286_sys_mwrw(cpu, cpu->tsdr, 30, cpu->si);
				i286_sys_mwrw(cpu, cpu->tsdr, 32, cpu->di);
				i286_sys_mwrw(cpu, cpu->tsdr, 34, cpu->es.idx);
				i286_sys_mwrw(cpu, cpu->tsdr, 36, cpu->cs.idx);
				i286_sys_mwrw(cpu, cpu->tsdr, 38, cpu->ss.idx);
				i286_sys_mwrw(cpu, cpu->tsdr, 40, cpu->ds.idx);
				// load registers from new tss
				i286_sys_mwrw(cpu, seg, 0, cpu->tsdr.idx);	// previous TSS selector as link
				cpu->tsdr = seg;				// switch to new TSS
				cpu->pc = i286_sys_mrdw(cpu, cpu->tsdr, 14);
				cpu->f = i286_sys_mrdw(cpu, cpu->tsdr, 16);
				cpu->ax = i286_sys_mrdw(cpu, cpu->tsdr, 18);
				cpu->cx = i286_sys_mrdw(cpu, cpu->tsdr, 20);
				cpu->dx = i286_sys_mrdw(cpu, cpu->tsdr, 22);
				cpu->bx = i286_sys_mrdw(cpu, cpu->tsdr, 24);
				cpu->bp = i286_sys_mrdw(cpu, cpu->tsdr, 28);
				cpu->si = i286_sys_mrdw(cpu, cpu->tsdr, 30);
				cpu->di = i286_sys_mrdw(cpu, cpu->tsdr, 32);
				// nested
				if (nest) {
					cpu->f |= I286_FN;
				}
				cpu->msw |= I286_FTS;
				// load ldt, then ss,cs,ds,es
				cpu->ldtr = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 42));
				cpu->es = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 34));
				cpu->cs = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 36));
				cpu->ds = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 40));
				if (cpu->pc > cpu->cs.limit) {
					THROW_EC(I286_INT_GP, 0);
				}
				// TODO: check it
				if (dpl < cpl) {		// switch stack
					cpu->sp = i286_sys_mrdw(cpu, cpu->tsdr, 2 + 4 * dpl);
					cpu->ss = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 4 + 4 * dpl));
				} else {			// don't
					cpu->sp = i286_sys_mrdw(cpu, cpu->tsdr, 26);
					cpu->ss = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 38));
				}
				// set CPL to RPL of CS selector
				cpu->cs.pl = cpu->cs.idx & 3;
			}
		} else {			// not present
			THROW_EC(I286_INT_NP, tsss);
		}
	}
}

// gates
// check cases for go to seg:ip (limit, gate/trap, etc)
// RPL = sn & 3				requested
// DPL = (seg.flag >> 5) & 3		destination
// CPL = (cs.flag >> 5) & 3		current
void i286_check_gate(CPU* cpu, int ip, int sn) {
	cpu->ea.seg = i286_cash_seg(cpu, sn);
	cpu->ea.adr = ip;
	cpu->ea.reg = 0;
	cpu->ea.cnt = 0;
	int rpl = sn & 3;
	int dpl = cpu->ea.seg.pl;
	int cpl = cpu->cs.pl;
	if (cpu->msw & I286_FPE) {			// protected mode
		if ((sn & 0xfff8) == 0) {		// segment 0 -> #GP(0)
			THROW_EC(I286_INT_GP, 0);
		} else if (cpu->ea.seg.ar & 0x10) {	// segment is code/data
			if (cpu->ea.seg.ar & 8) {	// code
				if (cpu->ea.seg.ar & 4) {	// conforming
					if (dpl > cpl) {	// DPL <= CPL
						THROW_EC(I286_INT_GP, sn);
					}
				} else {			// non-conforming
					if (rpl > cpl) {		// RPL <= CPL
						THROW_EC(I286_INT_GP, sn);
					} else if (dpl != cpl) {	// DPL = CPL
						THROW_EC(I286_INT_GP, sn);
					}
					cpu->ea.seg.pl = cpu->cs.pl;		// save CPL
				}
				if (!cpu->ea.seg.pr) {			// present
					THROW_EC(I286_INT_NP, sn);
				} else if (cpu->ea.adr > cpu->ea.seg.limit) {		// limit
					THROW_EC(I286_INT_GP, 0);
				}
				// result: code segment is valid
			} else {			// data -> #GP(sn)
				THROW_EC(I286_INT_GP, sn);
			}
		} else {				// segment is system (trap/gates)
			cpu->gate = cpu->ea.seg;
			switch (cpu->ea.seg.ar) {		// type
				case 1:			// TSS (available)
					if ((dpl < cpl) || (dpl < rpl)) {		// privelegies
						THROW_EC(I286_INT_GP, sn);
					} else if (!cpu->ea.seg.pr) {	// present
						THROW_EC(I286_INT_NP, sn);
					} else {
						i286_switch_task(cpu, sn, 1, 0);		// switch to this TSS
					}
					break;
				case 3:			// TSS (busy)
					THROW_EC(I286_INT_GP, 0);
					break;
				case 4:			// call gate
					if ((dpl < cpl) || (dpl < cpl)) {
						THROW_EC(I286_INT_GP, sn);
					} else if (!cpu->ea.seg.pr) {
						THROW_EC(I286_INT_NP, sn);
					} else {		// get call gate params
						cpu->ea.adr = cpu->ea.seg.base & 0xffff;	// offset (bytes 0,1)
						sn = cpu->ea.seg.limit;				// segment selector (bytes 2,3)
						cpu->ea.cnt = (cpu->ea.seg.base >> 16) & 0x1f;	// count of words transported from old stack to new stack (byte 4, bits 0..4)
						if (sn == 0) {
							THROW_EC(I286_INT_GP, 0);
						} else {
							cpu->seg = i286_cash_seg(cpu, sn);	// new code segment
							if ((cpu->seg.ar & 0x18) != 0x18) {	// is code segment?
								THROW_EC(I286_INT_GP, cpu->cs.idx);
							} else if ((cpu->seg.ar & 4) && (cpu->seg.pl > cpl)) {			// conforming
								THROW_EC(I286_INT_GP, cpu->cs.idx);
							} else if (!(cpu->seg.ar & 4) && (cpu->seg.pl != cpl)) {		// non-conforming
								THROW_EC(I286_INT_GP, cpu->cs.idx);
							} else if (!cpu->seg.pr) {
								THROW_EC(I286_INT_NP, cpu->cs.idx);
							} else if (cpu->ea.adr > cpu->seg.limit) {
								THROW_EC(I286_INT_GP, 0);
							} else {
								cpu->ea.seg = cpu->seg;
								cpu->ea.reg = 1;
							}
						}
					}
					break;
				case 5:			// task gate
					if ((dpl < cpl) || (dpl < rpl)) {		// privs
						THROW_EC(I286_INT_GP, sn);
					} else if (!cpu->ea.seg.pr) {	// present
						THROW_EC(I286_INT_NP, sn);
					} else {
						sn = cpu->ea.seg.base & 0xffff;		// TSS selector (bytes 2,3) // i286_sys_mrdw(cpu, cpu->ea.seg, 2);)
						i286_switch_task(cpu, sn, 0, 0);
					}
					break;
				case 6:			// interrupt gate - get tss selector and switch. = trap gate with disable interrupts
				case 7:			// trap gate
					if ((dpl < cpl) || (cpl < rpl)) {		// privs
						THROW_EC(I286_INT_GP, sn);
					} else if (!cpu->ea.seg.pr) {	// present
						THROW_EC(I286_INT_NP, sn);
					} else {
						cpu->ea.adr = cpu->ea.seg.base & 0xffff;		// offset (0,1)
						cpu->ea.seg = i286_cash_seg(cpu, cpu->ea.seg.limit);	// selector (2,3)
						if (!cpu->ea.seg.code) {				// not code segment
							THROW_EC(I286_INT_GP, cpu->ea.seg.idx);
						}
					}
					break;
				default:		// not recognized
					THROW_EC(I286_INT_GP, sn);
					break;
			}
		}
	} else {			// real mode: check limits only
		if (cpu->ea.adr > cpu->ea.seg.limit) {
			THROW_EC(I286_INT_GP, 0);
		}
	}
}

// protected mode checks

int i286_check_iopl(CPU* cpu) {		// CPL <= IOPL
	if (!(cpu->msw & I286_FPE)) return 1;	// real mode
	int iopl = (cpu->f >> 12) & 3;
	return (cpu->cs.pl <= iopl) ? 1 : 0;
}

// mod r/m

int i286_get_reg(CPU* cpu, int wrd) {
	int res = -1;
	if (wrd) {
		switch((cpu->mod >> 3) & 7) {
			case 0: res = cpu->ax; break;
			case 1: res = cpu->cx; break;
			case 2: res = cpu->dx; break;
			case 3: res = cpu->bx; break;
			case 4: res = cpu->sp; break;
			case 5: res = cpu->bp; break;
			case 6: res = cpu->si; break;
			case 7: res = cpu->di; break;
		}
		res &= 0xffff;
	} else {
		switch((cpu->mod >> 3) & 7) {
			case 0: res = cpu->al; break;
			case 1: res = cpu->cl; break;
			case 2: res = cpu->dl; break;
			case 3: res = cpu->bl; break;
			case 4: res = cpu->ah; break;
			case 5: res = cpu->ch; break;
			case 6: res = cpu->dh; break;
			case 7: res = cpu->bh; break;
		}
		res &= 0xff;
	}
	return res;
}

void i286_set_reg(CPU* cpu, int val, int wrd) {
	if (wrd) {
		val &= 0xffff;
		switch((cpu->mod >> 3) & 7) {
			case 0: cpu->ax = val; break;
			case 1: cpu->cx = val; break;
			case 2: cpu->dx = val; break;
			case 3: cpu->bx = val; break;
			case 4: cpu->sp = val; break;
			case 5: cpu->bp = val; break;
			case 6: cpu->si = val; break;
			case 7: cpu->di = val; break;
		}
	} else {
		val &= 0xff;
		switch((cpu->mod >> 3) & 7) {
			case 0: cpu->al = val; break;
			case 1: cpu->cl = val; break;
			case 2: cpu->dl = val; break;
			case 3: cpu->bl = val; break;
			case 4: cpu->ah = val; break;
			case 5: cpu->ch = val; break;
			case 6: cpu->dh = val; break;
			case 7: cpu->bh = val; break;
		}
	}
}

// read mod, calculate effective address in cpu->tmpi, set register N to cpu->twrd/ltw
// modbyte: [7.6:mod][5.4.3:regN][2.1.0:adr/reg]
// TODO: tmpi is still using somewhere (?)
void i286_get_ea(CPU* cpu, int wrd) {
	cpu->tmpw = 0;	// = disp
	cpu->mod = i286_rd_imm(cpu);
	cpu->twrd = i286_get_reg(cpu, wrd);
	if ((cpu->mod & 0xc0) == 0xc0) {		// ea is register
		cpu->ea.reg = 1;
		if (wrd) {
			switch(cpu->mod & 7) {
				case 0: cpu->tmpw = cpu->ax; break;
				case 1: cpu->tmpw = cpu->cx; break;
				case 2: cpu->tmpw = cpu->dx; break;
				case 3: cpu->tmpw = cpu->bx; break;
				case 4: cpu->tmpw = cpu->sp; break;
				case 5: cpu->tmpw = cpu->bp; break;
				case 6: cpu->tmpw = cpu->si; break;
				case 7: cpu->tmpw = cpu->di; break;
			}
		} else {
			cpu->htw = 0;
			switch(cpu->mod & 7) {
				case 0: cpu->ltw = cpu->al; break;
				case 1: cpu->ltw = cpu->cl; break;
				case 2: cpu->ltw = cpu->dl; break;
				case 3: cpu->ltw = cpu->bl; break;
				case 4: cpu->ltw = cpu->ah; break;
				case 5: cpu->ltw = cpu->ch; break;
				case 6: cpu->ltw = cpu->dh; break;
				case 7: cpu->ltw = cpu->bh; break;
			}
		}
		cpu->tmpi = -1;		// reg
	} else {					// ea is memory
		cpu->ea.reg = 0;
		if ((cpu->mod & 0xc0) == 0x40) {
			cpu->ltw = i286_rd_imm(cpu);
			cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
		} else if ((cpu->mod & 0xc0) == 0x80) {
			cpu->tmpw = i286_rd_immw(cpu);
		}
		switch(cpu->mod & 0x07) {
			case 0: cpu->ea.adr = cpu->bx + cpu->si + cpu->tmpw;
				cpu->ea.seg = cpu->ds;
				break;
			case 1: cpu->ea.adr = cpu->bx + cpu->di + cpu->tmpw;
				cpu->ea.seg = cpu->ds;
				break;
			case 2: cpu->ea.adr = cpu->bp + cpu->si + cpu->tmpw;
				cpu->ea.seg = cpu->ss;
				break;
			case 3: cpu->ea.adr = cpu->bp + cpu->di + cpu->tmpw;
				cpu->ea.seg = cpu->ss;
				break;
			case 4: cpu->ea.adr = cpu->si + cpu->tmpw;
				cpu->ea.seg = cpu->ds;
				break;
			case 5: cpu->ea.adr = cpu->di + cpu->tmpw;
				cpu->ea.seg = cpu->ds;	// TODO: or es in some opcodes (not overrideable)
				break;
			case 6:	if (cpu->mod & 0xc0) {
					cpu->ea.adr = cpu->bp + cpu->tmpw;
					cpu->ea.seg = cpu->ss;
				} else {
					cpu->ea.adr = i286_rd_immw(cpu);		// adr = immw
					cpu->ea.seg = cpu->ds;
				}
				break;
			case 7: cpu->ea.adr = cpu->bx + cpu->tmpw;
				cpu->ea.seg = cpu->ds;
				break;
		}
		cpu->tmpi = 0;		// memory address
	}
}

// = get ea & read byte/word from EA to cpu->tmpw
void i286_rd_ea(CPU* cpu, int wrd) {
	i286_get_ea(cpu, wrd);
	if (!cpu->ea.reg) {
		cpu->ltw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr);
		cpu->htw = wrd ? i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 1) : 0;
	}
}

// must be called after i286_rd_ea, cpu->ea must be calculated, cpu->mod setted
void i286_wr_ea(CPU* cpu, int val, int wrd) {
	if (cpu->ea.reg) {		// this is a reg
		if (wrd) {
			switch(cpu->mod & 7) {
				case 0: cpu->ax = val & 0xffff; break;
				case 1: cpu->cx = val & 0xffff; break;
				case 2: cpu->dx = val & 0xffff; break;
				case 3: cpu->bx = val & 0xffff; break;
				case 4: cpu->sp = val & 0xffff; break;
				case 5: cpu->bp = val & 0xffff; break;
				case 6: cpu->si = val & 0xffff; break;
				case 7: cpu->di = val & 0xffff; break;
			}
		} else {
			switch(cpu->mod & 7) {
				case 0: cpu->al = val & 0xff; break;
				case 1: cpu->cl = val & 0xff; break;
				case 2: cpu->dl = val & 0xff; break;
				case 3: cpu->bl = val & 0xff; break;
				case 4: cpu->ah = val & 0xff; break;
				case 5: cpu->ch = val & 0xff; break;
				case 6: cpu->dh = val & 0xff; break;
				case 7: cpu->bh = val & 0xff; break;
			}
		}
	} else {
		i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr, val & 0xff);
		if (wrd) i286_mwr(cpu, cpu->ea.seg, 1, cpu->ea.adr + 1, (val >> 8) & 0xff);
	}
}

// add/adc

static const int i286_add_FO[8] = {0, 0, 0, 1, 1, 0, 0, 0};

unsigned char i286_add8(CPU* cpu, unsigned char p1, unsigned char p2, int cf) {
	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xff;
	int r2 = (p2 & 0xff) + (cf ? 1 : 0);
	int res = r1 + r2;
	//cpu->tmp = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((res & 0x80) >> 5);
	//if (i286_add_FO[cpu->tmp]) cpu->f |= I286_FO;
	if (((p1 ^ p2 ^ 0x80) & (res ^ p2)) & 0x80) cpu->f |= I286_FO;
	if (res & 0x80) cpu->f |= I286_FS;
	if (!(res & 0xff)) cpu->f |= I286_FZ;
	if ((p1 & 15) + (p2 & 15) > 15) cpu->f |= I286_FA;
	if (parity(res & 0xff)) cpu->f |= I286_FP;
	if (res & ~0xff) cpu->f |= I286_FC;
	return res & 0xff;
}

unsigned short i286_add16(CPU* cpu, unsigned short p1, unsigned short p2, int cf) {
	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xffff;
	int r2 = (p2 & 0xffff) + (cf ? 1 : 0);
	int res = r1 + r2;
	//cpu->tmp = ((p1 & 0x8000) >> 15) | ((p2 & 0x8000) >> 14) | ((res & 0x8000) >> 13);
	//if (i286_add_FO[cpu->tmp]) cpu->f |= I286_FO;
	if (((p1 ^ p2 ^ 0x8000) & (res ^ p2)) & 0x8000) cpu->f |= I286_FO;
	if (res & 0x8000) cpu->f |= I286_FS;
	if (!(res & 0xffff)) cpu->f |= I286_FZ;
	if ((p1 & 0xfff) + (p2 & 0xfff) > 0xfff) cpu->f |= I286_FA;
	if (parity(res & 0xff)) cpu->f |= I286_FP;
	if (res & ~0xffff) cpu->f |= I286_FC;
	return res & 0xffff;
}

// 00,mod: add eb,rb	EA.byte += reg.byte
void i286_op00(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_add8(cpu, cpu->ltw, cpu->lwr, 0);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 01,mod: add ew,rw	EA.word += reg.word
void i286_op01(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, 0);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 02,mod: add rb,eb	reg.byte += EA.byte
void i286_op02(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_add8(cpu, cpu->ltw, cpu->lwr, 0);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 03,mod: add rw,ew	reg.word += EA.word
void i286_op03(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, 0);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 04,db: add AL,db	AL += db
void i286_op04(CPU* cpu) {
	cpu->lwr = i286_rd_imm(cpu);
	cpu->al = i286_add8(cpu, cpu->al, cpu->lwr, 0);
}

// 05,dw: add AX,dw	AX += dw
void i286_op05(CPU* cpu) {
	cpu->twrd = i286_rd_immw(cpu);
	cpu->ax = i286_add16(cpu, cpu->ax, cpu->twrd, 0);
}

// 06: push es
void i286_op06(CPU* cpu) {
	i286_push(cpu, cpu->es.idx);
}

// 07: pop es
void i286_op07(CPU* cpu) {
	cpu->tmpw = i286_pop(cpu);
	cpu->es = i286_cash_seg(cpu, cpu->tmpw);
}

// or

unsigned char i286_or8(CPU* cpu, unsigned char p1, unsigned char p2) {
	unsigned char res =  p1 | p2;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (res & 0x80) cpu->f |= I286_FS;
	if (!res) cpu->f |= I286_FZ;
	if (parity(res & 0xff)) cpu->f |= I286_FP;
	return res;
}

unsigned short i286_or16(CPU* cpu, unsigned short p1, unsigned short p2) {
	unsigned short res =  p1 | p2;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (res & 0x8000) cpu->f |= I286_FS;
	if (!res) cpu->f |= I286_FZ;
	if (parity(res & 0xff)) cpu->f |= I286_FP;
	return res;
}

// 08,mod: or eb,rb
void i286_op08(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_or8(cpu, cpu->ltw, cpu->lwr);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 09,mod: or ew,rw
void i286_op09(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_or16(cpu, cpu->tmpw, cpu->twrd);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 0a,mod: or rb,eb
void i286_op0A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_or8(cpu, cpu->ltw, cpu->lwr);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 0b,mod: or rw,ew
void i286_op0B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_or16(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 0c,db: or al,db
void i286_op0C(CPU* cpu) {
	cpu->tmp = i286_rd_imm(cpu);
	cpu->al = i286_or8(cpu, cpu->al, cpu->tmp);
}

// 0d,dw: or ax,dw
void i286_op0D(CPU* cpu) {
	cpu->twrd = i286_rd_immw(cpu);
	cpu->ax = i286_or16(cpu, cpu->ax, cpu->twrd);
}

// 0e: push cs
void i286_op0E(CPU* cpu) {
	i286_push(cpu, cpu->cs.idx);
}

// 0f: prefix
extern opCode i286_0f_tab[256];

void i286_op0F(CPU* cpu) {
	cpu->opTab = i286_0f_tab;
}

// 10,mod: adc eb,rb
void i286_op10(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_add8(cpu, cpu->ltw, cpu->lwr, cpu->f & I286_FC);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 11,mod: adc ew,rw
void i286_op11(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, cpu->f & I286_FC);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 12,mod: adc rb,eb
void i286_op12(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_add8(cpu, cpu->ltw, cpu->lwr, cpu->f & I286_FC);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 13,mod: adc rw,ew
void i286_op13(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_add16(cpu, cpu->tmpw, cpu->twrd, cpu->f & I286_FC);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 14,db: adc al,db
void i286_op14(CPU* cpu) {
	cpu->lwr = i286_rd_imm(cpu);
	cpu->al = i286_add8(cpu, cpu->al, cpu->lwr, cpu->f & I286_FC);
}

// 15,dw: adc ax,dw
void i286_op15(CPU* cpu) {
	cpu->twrd = i286_rd_immw(cpu);
	cpu->ax = i286_add16(cpu, cpu->ax, cpu->twrd, cpu->f & I286_FC);
}

// 16: push ss
void i286_op16(CPU* cpu) {
	i286_push(cpu, cpu->ss.idx);
}

// 17: pop ss
void i286_op17(CPU* cpu) {
	cpu->tmpw = i286_pop(cpu);
	cpu->ss = i286_cash_seg(cpu, cpu->tmpw);
}

// sub/sbc

// static const int i286_sub_FO[8] = {0, 1, 0, 0, 0, 0, 1, 0};

unsigned char i286_sub8(CPU* cpu, unsigned char p1, unsigned char p2, int cf) {
	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xff;
	int r2 = (p2 & 0xff) + (cf ? 1 : 0);
	int res = r1 - r2;
	//cpu->tmp = ((p1 & 0x80) >> 7) | ((p2 & 0x80) >> 6) | ((res & 0x80) >> 5);
	//if (i286_sub_FO[cpu->tmp & 7]) cpu->f |= I286_FO;
	if (((p1 ^ p2) & (p1 ^ res)) & 0x80) cpu->f |= I286_FO;
	if (res & 0x80) cpu->f |= I286_FS;
	if (!(res & 0xff)) cpu->f |= I286_FZ;
	if (parity(res & 0xff)) cpu->f |= I286_FP;
	if (res & ~0xff) cpu->f |= I286_FC;
	if ((p1 & 0x0f) < (p2 & 0x0f)) cpu->f |= I286_FA;
	return res & 0xff;
}

unsigned short i286_sub16(CPU* cpu, unsigned short p1, unsigned short p2, int cf) {
	cpu->f &= ~(I286_FS | I286_FZ | I286_FP | I286_FO | I286_FC | I286_FA);
	int r1 = p1 & 0xffff;
	int r2 = (p2 & 0xffff) + (cf ? 1 : 0);
	int res = r1 - r2;
	//cpu->tmp = ((p1 & 0x8000) >> 15) | ((p2 & 0x8000) >> 14) | ((res & 0x8000) >> 13);
	//if (i286_sub_FO[cpu->tmp & 7]) cpu->f |= I286_FO;
	if (((p1 ^ p2) & (p1 ^ res)) & 0x8000) cpu->f |= I286_FO;
	if (res & 0x8000) cpu->f |= I286_FS;
	if (!(res & 0xffff)) cpu->f |= I286_FZ;
	if (parity(res & 0xff)) cpu->f |= I286_FP;
	if (res & ~0xffff) cpu->f |= I286_FC;
	if ((p1 & 0x0fff) < (p2 & 0x0fff)) cpu->f |= I286_FA;
	return res & 0xffff;
}

// 18,mod: sbb eb,rb	NOTE: sbc
void i286_op18(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_sub8(cpu, cpu->ltw, cpu->lwr, cpu->f & I286_FC);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 19,mod: sbb ew,rw
void i286_op19(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->tmpw, cpu->twrd, cpu->f & I286_FC);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 1a,mod: sbb rb,eb
void i286_op1A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_sub8(cpu, cpu->lwr, cpu->ltw, cpu->f & I286_FC);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 1b,mod: sbb rw,ew
void i286_op1B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->twrd, cpu->tmpw, cpu->f & I286_FC);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 1c,db: sbb al,db
void i286_op1C(CPU* cpu) {
	cpu->lwr = i286_rd_imm(cpu);
	cpu->al = i286_sub8(cpu, cpu->al, cpu->lwr, cpu->f & I286_FC);
}

// 1d,dw: sbb ax,dw
void i286_op1D(CPU* cpu) {
	cpu->twrd = i286_rd_immw(cpu);
	cpu->ax = i286_sub16(cpu, cpu->ax, cpu->twrd, cpu->f & I286_FC);
}

// 1e: push ds
void i286_op1E(CPU* cpu) {
	i286_push(cpu, cpu->ds.idx);
}

// 1f: pop ds
void i286_op1F(CPU* cpu) {
	cpu->tmpw = i286_pop(cpu);
	cpu->ds = i286_cash_seg(cpu, cpu->tmpw);
}

// and

unsigned char i286_and8(CPU* cpu, unsigned char p1, unsigned char p2) {
	cpu->f &= ~(I286_FO | I286_FS | I286_FP | I286_FZ | I286_FC);
	p1 &= p2;
	if (p1 & 0x80) cpu->f |= I286_FS;
	if (!p1) cpu->f |= I286_FZ;
	if (parity(p1 & 0xff)) cpu->f |= I286_FP;
	return p1;
}

unsigned short i286_and16(CPU* cpu, unsigned short p1, unsigned short p2) {
	cpu->f &= ~(I286_FO | I286_FS | I286_FP | I286_FZ | I286_FC);
	p1 &= p2;
	if (p1 & 0x8000) cpu->f |= I286_FS;
	if (!p1) cpu->f |= I286_FZ;
	if (parity(p1 & 0xff)) cpu->f |= I286_FP;
	return p1;
}

// 20,mod: and eb,rb
void i286_op20(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_and8(cpu, cpu->ltw, cpu->lwr);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 21,mod: and ew,rw
void i286_op21(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_and16(cpu, cpu->tmpw, cpu->twrd);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 22,mod: and rb,eb
void i286_op22(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_and8(cpu, cpu->ltw, cpu->lwr);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 23,mod: and rw,ew
void i286_op23(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_and16(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 24,db: and al,db
void i286_op24(CPU* cpu) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->al = i286_and8(cpu, cpu->al, cpu->ltw);
}

// 25,dw: and ax,dw
void i286_op25(CPU* cpu) {
	cpu->twrd = i286_rd_immw(cpu);
	cpu->ax = i286_and16(cpu, cpu->ax, cpu->twrd);
}

// 26: ES segment override prefix
void i286_op26(CPU* cpu) {
	cpu->seg = cpu->es;
}

// 27: daa
void i286_op27(CPU* cpu) {
	if (((cpu->al & 0x0f) > 9) || (cpu->f & I286_FA)) {
		cpu->al += 6;
		cpu->f |= I286_FA;
	} else {
		cpu->f &= ~I286_FA;
	}
	if ((cpu->al > 0x9f) || (cpu->f & I286_FC)) {
		cpu->al += 0x60;
		cpu->f |= I286_FC;
	} else {
		cpu->f &= ~I286_FC;
	}
	cpu->f &= ~(I286_FS | I286_FZ | I286_FP);
	if (cpu->al & 0x80) cpu->f |= I286_FS;
	if (!cpu->al) cpu->f |= I286_FZ;
	if (parity(cpu->al & 0xff)) cpu->f |= I286_FP;
}

// 28,mod: sub eb,rb
void i286_op28(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_sub8(cpu, cpu->ltw, cpu->lwr, 0);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 29,mod: sub ew,rw
void i286_op29(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->tmpw, cpu->twrd, 0);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 2a,mod: sub rb,eb
void i286_op2A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_sub8(cpu, cpu->lwr, cpu->ltw, 0);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 2b,mod: sub rw,ew
void i286_op2B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->twrd, cpu->tmpw, 0);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 2c,db: sub al,db
void i286_op2C(CPU* cpu) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->al =  i286_sub8(cpu, cpu->al, cpu->ltw, 0);
}

// 2d,dw: sub ax,dw
void i286_op2D(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->ax =  i286_sub16(cpu, cpu->ax, cpu->tmpw, 0);
}

// 2e: CS segment override prefix
void i286_op2E(CPU* cpu) {
	cpu->seg = cpu->cs;
}

// 2f: das
void i286_op2F(CPU* cpu) {
	if (((cpu->al & 15) > 9) || (cpu->f & I286_FA)) {
		cpu->al -= 9;
		cpu->f |= I286_FA;
	} else {
		cpu->f &= ~I286_FA;
	}
	if ((cpu->al > 0x9f) || (cpu->f & I286_FC)) {
		cpu->al -= 0x60;
		cpu->f |= I286_FC;
	} else {
		cpu->f &= ~I286_FC;
	}
	cpu->f &= ~(I286_FS | I286_FZ | I286_FP);
	if (cpu->al & 0x80) cpu->f |= I286_FS;
	if (!cpu->al) cpu->f |= I286_FZ;
	if (parity(cpu->al & 0xff)) cpu->f |= I286_FP;
}

// xor

unsigned char i286_xor8(CPU* cpu, unsigned char p1, unsigned char p2) {
	p1 ^= p2;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (p1 & 0x80) cpu->f |= I286_FS;
	if (!p1) cpu->f |= I286_FZ;
	if (parity(p1 & 0xff)) cpu->f |= I286_FP;
	return p1;
}

unsigned short i286_xor16(CPU* cpu, unsigned short p1, unsigned short p2) {
	p1 ^= p2;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FP | I286_FC);
	if (p1 & 0x8000) cpu->f |= I286_FS;
	if (!p1) cpu->f |= I286_FZ;
	if (parity(p1 & 0xff)) cpu->f |= I286_FP;
	return p1;
}

// 30,mod: xor eb,rb
void i286_op30(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_xor8(cpu, cpu->ltw, cpu->lwr);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// 31,mod: xor ew,rw
void i286_op31(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_xor16(cpu, cpu->tmpw, cpu->twrd);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// 32,mod: xor rb,eb
void i286_op32(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->ltw = i286_xor8(cpu, cpu->ltw, cpu->lwr);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// CHECK: xor di,di (33,ff)
// FF: [11:reg][111:di][111:reg.di]
// 33,mod: xor rw,ew
void i286_op33(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_xor16(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 34,db: xor al,db
void i286_op34(CPU* cpu) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->al = i286_xor8(cpu, cpu->al, cpu->ltw);
}

// 35,dw: xor ax,dw
void i286_op35(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->ax = i286_xor16(cpu, cpu->ax, cpu->tmpw);
}

// 36: SS segment override prefix
void i286_op36(CPU* cpu) {
	cpu->seg = cpu->ss;
}

// 37: aaa
void i286_op37(CPU* cpu) {
	if (((cpu->al & 0x0f) > 0x09) || (cpu->f & I286_FA)) {
		cpu->al += 6;
		cpu->ah++;
		cpu->f |= (I286_FA | I286_FC);
	} else {
		cpu->f &= ~(I286_FA | I286_FC);
	}
}

// 38,mod: cmp eb,rb
void i286_op38(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmp = i286_sub8(cpu, cpu->ltw, cpu->lwr, 0);
}

// 39,mod: cmp ew,rw
void i286_op39(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->tmpw, cpu->twrd, 0);
}

// 3a,mod: cmp rb,eb
void i286_op3A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmp = i286_sub8(cpu, cpu->lwr, cpu->ltw, 0);
}

// 3b,mod: cmp rw,ew
void i286_op3B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_sub16(cpu, cpu->twrd, cpu->tmpw, 0);
}

// 3c,db: cmp al,db
void i286_op3C(CPU* cpu) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->ltw = i286_sub8(cpu, cpu->al, cpu->ltw, 0);
}

// 3d,dw: cmp ax,dw
void i286_op3D(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->tmpw = i286_sub16(cpu, cpu->ax, cpu->tmpw, 0);
}

// 3e: DS segment override prefix
void i286_op3E(CPU* cpu) {
	cpu->seg = cpu->ds;
}

// 3f: aas
void i286_op3F(CPU* cpu) {
	if (((cpu->al & 15) > 9) | (cpu->f & I286_FA)) {
		cpu->al -= 6;
		cpu->ah--;
		cpu->f |= (I286_FA | I286_FC);
	} else {
		cpu->f &= ~(I286_FA | I286_FC);
	}
	cpu->al &= 15;
}

// inc

unsigned char i286_inc8(CPU* cpu, unsigned char r) {
	r++;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FA | I286_FP);
	if (r == 0x80) cpu->f |= I286_FO;
	if (r & 0x80) cpu->f |= I286_FS;
	if (!r) cpu->f |= I286_FZ;
	if ((r & 15) == 0) cpu->f |= I286_FA;	// ? 0fff
	if (parity(r & 0xff)) cpu->f |= I286_FP;
	return r;
}

unsigned short i286_inc16(CPU* cpu, unsigned short r) {
	r++;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FA | I286_FP);
	if (r == 0x8000) cpu->f |= I286_FO;
	if (r & 0x8000) cpu->f |= I286_FS;
	if (!r) cpu->f |= I286_FZ;
	if ((r & 15) == 0) cpu->f |= I286_FA;	// ? 0fff
	if (parity(r & 0xff)) cpu->f |= I286_FP;
	return r;
}

// 40: inc ax
void i286_op40(CPU* cpu) {
	cpu->ax = i286_inc16(cpu, cpu->ax);
}

// 41: inc cx
void i286_op41(CPU* cpu) {
	cpu->cx = i286_inc16(cpu, cpu->cx);
}

// 42: inc dx
void i286_op42(CPU* cpu) {
	cpu->dx = i286_inc16(cpu, cpu->dx);
}

// 43: inc bx
void i286_op43(CPU* cpu) {
	cpu->bx = i286_inc16(cpu, cpu->bx);
}

// 44: inc sp
void i286_op44(CPU* cpu) {
	cpu->sp = i286_inc16(cpu, cpu->sp);
}

// 45: inc bp
void i286_op45(CPU* cpu) {
	cpu->bp = i286_inc16(cpu, cpu->bp);
}

// 46: inc si
void i286_op46(CPU* cpu) {
	cpu->si = i286_inc16(cpu, cpu->si);
}

// 47: inc di
void i286_op47(CPU* cpu) {
	cpu->di = i286_inc16(cpu, cpu->di);
}

// dec

unsigned char i286_dec8(CPU* cpu, unsigned char r) {
	r--;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FA | I286_FP);
	if (r == 0x7f) cpu->f |= I286_FO;
	if (r & 0x80) cpu->f |= I286_FS;
	if (!r) cpu->f |= I286_FZ;
	if ((r & 15) == 15) cpu->f |= I286_FA;
	if (parity(r & 0xff)) cpu->f |= I286_FP;
	return r;
}

unsigned short i286_dec16(CPU* cpu, unsigned short r) {
	r--;
	cpu->f &= ~(I286_FO | I286_FS | I286_FZ | I286_FA | I286_FP);
	if (r == 0x7fff) cpu->f |= I286_FO;
	if (r & 0x8000) cpu->f |= I286_FS;
	if (!r) cpu->f |= I286_FZ;
	if ((r & 15) == 15) cpu->f |= I286_FA;
	if (parity(r & 0xff)) cpu->f |= I286_FP;
	return r;
}

// 48: dec ax
void i286_op48(CPU* cpu) {
	cpu->ax = i286_dec16(cpu, cpu->ax);
}

// 49: dec cx
void i286_op49(CPU* cpu) {
	cpu->cx = i286_dec16(cpu, cpu->cx);
}

// 4a: dec dx
void i286_op4A(CPU* cpu) {
	cpu->dx = i286_dec16(cpu, cpu->dx);
}

// 4b: dec bx
void i286_op4B(CPU* cpu) {
	cpu->bx = i286_dec16(cpu, cpu->bx);
}

// 4c: dec sp
void i286_op4C(CPU* cpu) {
	cpu->sp = i286_dec16(cpu, cpu->sp);
}

// 4d: dec bp
void i286_op4D(CPU* cpu) {
	cpu->bp = i286_dec16(cpu, cpu->bp);
}

// 4e: dec si
void i286_op4E(CPU* cpu) {
	cpu->si = i286_dec16(cpu, cpu->si);
}

// 4f: dec di
void i286_op4F(CPU* cpu) {
	cpu->di = i286_dec16(cpu, cpu->di);
}

// 50: push ax
void i286_op50(CPU* cpu) {
	i286_push(cpu, cpu->ax);
}

// 51: push cx
void i286_op51(CPU* cpu) {
	i286_push(cpu, cpu->cx);
}

// 52: push dx
void i286_op52(CPU* cpu) {
	i286_push(cpu, cpu->dx);
}

// 53: push bx
void i286_op53(CPU* cpu) {
	i286_push(cpu, cpu->bx);
}

// 54: push sp
void i286_op54(CPU* cpu) {
	i286_push(cpu, cpu->sp);
}

// 55: push bp
void i286_op55(CPU* cpu) {
	i286_push(cpu, cpu->bp);
}

// 56: push si
void i286_op56(CPU* cpu) {
	i286_push(cpu, cpu->si);
}

// 57: push di
void i286_op57(CPU* cpu) {
	i286_push(cpu, cpu->di);
}

// 58: pop ax
void i286_op58(CPU* cpu) {
	cpu->ax = i286_pop(cpu);
}

// 59: pop cx
void i286_op59(CPU* cpu) {
	cpu->cx = i286_pop(cpu);
}

// 5a: pop dx
void i286_op5A(CPU* cpu) {
	cpu->dx = i286_pop(cpu);
}

// 5b: pop bx
void i286_op5B(CPU* cpu) {
	cpu->bx = i286_pop(cpu);
}

// 5c: pop sp
void i286_op5C(CPU* cpu) {
	cpu->sp = i286_pop(cpu);
}

// 5d: pop bp
void i286_op5D(CPU* cpu) {
	cpu->bp = i286_pop(cpu);
}

// 5e: pop si
void i286_op5E(CPU* cpu) {
	cpu->si = i286_pop(cpu);
}

// 5f: pop di
void i286_op5F(CPU* cpu) {
	cpu->di = i286_pop(cpu);
}

// 60: pusha	(push ax,cx,dx,bx,orig.sp,bp,si,di)
void i286_op60(CPU* cpu) {
	cpu->tmpw = cpu->sp;
	i286_push(cpu, cpu->ax);
	i286_push(cpu, cpu->cx);
	i286_push(cpu, cpu->dx);
	i286_push(cpu, cpu->bx);
	i286_push(cpu, cpu->tmpw);
	i286_push(cpu, cpu->bp);
	i286_push(cpu, cpu->si);
	i286_push(cpu, cpu->di);
}

// 61: popa	(pop di,si,bp,(ignore sp),bx,dx,cx,ax)
void i286_op61(CPU* cpu) {
	cpu->di = i286_pop(cpu);
	cpu->si = i286_pop(cpu);
	cpu->bp = i286_pop(cpu);
	cpu->tmpw = i286_pop(cpu);
	cpu->bx = i286_pop(cpu);
	cpu->dx = i286_pop(cpu);
	cpu->cx = i286_pop(cpu);
	cpu->ax = i286_pop(cpu);
}

// 62,mod: bound rw,md		@eff.addr (md): 2words = min,max. check if min<=rw<=max, INT5 if not
void i286_op62(CPU* cpu) {
	i286_rd_ea(cpu, 1);	// twrd=rw, tmpw=min
	if (cpu->ea.reg) {	// interrupts. TODO: fix for protected mode
		THROW(I286_INT_UD);		// bad mod
	} else if ((signed short)cpu->twrd < (signed short)cpu->tmpw) {	// not in bounds: INT5
		THROW(I286_INT_BR);
	} else {
		cpu->ltw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 2);
		cpu->htw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 3);
		if ((signed short)cpu->twrd > (signed short)cpu->tmpw) {
			THROW(I286_INT_BR);
		}
	}
}

// 63,mod: arpl ew,rw		adjust RPL of EW not less than RPL of RW
void i286_op63(CPU* cpu) {
	THROW(I286_INT_UD);	// real mode
	// TODO: protected mode
}

// 64: repeat next cmps/scas cx times or cf=1
void i286_op64(CPU* cpu) {}

// 65: repeat next cmps/scas cx times or cf=0
void i286_op65(CPU* cpu) {}

// 66: operand size override prefix
void i286_op66(CPU* cpu) {}

// 67: address size override prefix
void i286_op67(CPU* cpu) {}

// 68: push wrd
void i286_op68(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	i286_push(cpu, cpu->tmpw);
}

// mul
int i286_smul(CPU* cpu, signed short p1, signed short p2) {
	int res = p1 * p2;
	cpu->f &= ~(I286_FO | I286_FC);
	if ((p1 & 0x7fff) * (p2 & 0x7fff) > 0xffff)
		cpu->f |= I286_FC;
	cpu->tmp = ((p1 & 0x8000) >> 15) | ((p2 & 0x8000) >> 14) | ((res & 0x8000) >> 13);
	if (i286_add_FO[cpu->tmp & 7])
		cpu->f |= I286_FO;
	return res;
}

// 69,mod,dw: imul rw,ea,dw: rw = ea.w * wrd
void i286_op69(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->twrd = i286_rd_immw(cpu);
	cpu->tmpi = i286_smul(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpi & 0xffff, 1);
}

// 6a,db: push byte (sign extended to word)
void i286_op6A(CPU* cpu) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
	i286_push(cpu, cpu->tmpw);
}

// 6b,mod,db: imul rw,ea,db: rw = ea.w * db
void i286_op6B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->lwr = i286_rd_imm(cpu);
	cpu->hwr = (cpu->lwr & 0x80) ? 0xff : 0x00;
	cpu->tmpi = i286_smul(cpu, cpu->tmpw, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpi & 0xffff, 1);
}

// rep opcode template
// note: repz/repnz both working as rep

void i286_rep(CPU* cpu, cbcpu foo) {
	if (cpu->rep == I286_REP_NONE) {
		foo(cpu);
	} else {
		if (cpu->cx) {
			foo(cpu);
			cpu->cx--;
			if (cpu->cx)			// don't do last dummy cycle (cx=0)
				cpu->pc = cpu->oldpc;
		}
	}
}

// 6c: insb: [es:di] = in dx;
void i286_6c_cb(CPU* cpu) {
	cpu->tmp = i286_ird(cpu, cpu->dx);
	i286_mwr(cpu, cpu->es, 0, cpu->di, cpu->tmp);	// no segment override
	cpu->di += (cpu->f & I286_FD) ? -1 : 1;
}
void i286_op6C(CPU* cpu) {i286_rep(cpu, i286_6c_cb);}

// 6d: insw: word [es:di] = in dx;
void i286_6d_cb(CPU* cpu) {
	cpu->tmpw = i286_ird(cpu, cpu->dx);
	i286_mwr(cpu, cpu->es, 0, cpu->di, cpu->ltw);
	i286_mwr(cpu, cpu->es, 0, cpu->di + 1, cpu->htw);
	cpu->di += (cpu->f & I286_FD) ? -2 : 2;
}
void i286_op6D(CPU* cpu) {
	if (cpu->di == 0xffff) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		i286_rep(cpu, i286_6d_cb);
	}
}

// 6e: outsb: out (dx),word [ds:si]
void i286_6e_cb(CPU* cpu) {
	cpu->tmp = i286_mrd(cpu, cpu->ds, 1, cpu->si);
	i286_iwr(cpu, cpu->dx, cpu->tmp, 0);
	cpu->si += (cpu->f & I286_FD) ? -1 : 1;
}
void i286_op6E(CPU* cpu) {i286_rep(cpu, i286_6e_cb);}

// 6f: outs dx,wrd
void i286_6f_cb(CPU* cpu) {
	cpu->ltw = i286_mrd(cpu, cpu->ds, 1, cpu->si);
	cpu->htw = i286_mrd(cpu, cpu->ds, 1, cpu->si + 1);
	i286_iwr(cpu, cpu->dx, cpu->tmpw, 1);
	cpu->si += (cpu->f & I286_FD) ? -2 : 2;
}
void i286_op6F(CPU* cpu) {
	if (cpu->si == 0xffff) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		i286_rep(cpu, i286_6f_cb);
	}
}

// cond jump
void i286_jr(CPU* cpu, int cnd) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
	if (cnd) {
		cpu->pc += cpu->tmpw;
		cpu->t += 4;
	}
}

// 70: jo cb
void i286_op70(CPU* cpu) {i286_jr(cpu, cpu->f & I286_FO);}
// 71: jno cb
void i286_op71(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FO));}
// 72: jc cb (aka jb,jnae)
void i286_op72(CPU* cpu) {i286_jr(cpu, cpu->f & I286_FC);}
// 73: jnc cb (aka jnb,jae)
void i286_op73(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FC));}
// 74: jz cb (aka je)
void i286_op74(CPU* cpu) {i286_jr(cpu, cpu->f & I286_FZ);}
// 75: jnz cb (aka jne)
void i286_op75(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FZ));}
// 76: jbe cb (aka jna): CF=1 || Z=1
void i286_op76(CPU* cpu) {i286_jr(cpu, (cpu->f & I286_FC) || (cpu->f & I286_FZ));}
// 77: ja cb (aka jnbe): CF=0 && Z=0
void i286_op77(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FC) && !(cpu->f & I286_FZ));}
// 78: js cb
void i286_op78(CPU* cpu) {i286_jr(cpu, cpu->f & I286_FS);}
// 79: jns cb
void i286_op79(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FS));}
// 7a: jp cb (aka jpe)
void i286_op7A(CPU* cpu) {i286_jr(cpu, cpu->f & I286_FP);}
// 7b: jnp cb (aka jpo)
void i286_op7B(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FP));}
// 7c: jl cb (aka jngl) FS!=FO
void i286_op7C(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FS) != !(cpu->f & I286_FO));}
// 7d: jnl cb (aka jgl) FS==FO
void i286_op7D(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FS) == !(cpu->f & I286_FO));}
// 7e: jle cb (aka jng) (FZ=1)||(FS!=FO)
void i286_op7E(CPU* cpu) {i286_jr(cpu, (cpu->f & I286_FZ) || (!(cpu->f & I286_FS) != !(cpu->f & I286_FO)));}
// 7f: jnle cb (aka jg) (FZ=0)&&(FS=FO)
void i286_op7F(CPU* cpu) {i286_jr(cpu, !(cpu->f & I286_FZ) && (!(cpu->f & I286_FS) == !(cpu->f & I286_FO)));}

// 80: ALU eb,byte
void i286_op80(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpb = i286_rd_imm(cpu);
	switch((cpu->mod >> 3) & 7) {
		case 0: cpu->tmpb = i286_add8(cpu, cpu->ltw, cpu->tmpb, 0); break;		// add
		case 1: cpu->tmpb = i286_or8(cpu, cpu->ltw, cpu->tmpb); break;			// or
		case 2: cpu->tmpb = i286_add8(cpu, cpu->ltw, cpu->tmpb, cpu->f & I286_FC);	// adc
			break;
		case 3: cpu->tmpb = i286_sub8(cpu, cpu->ltw, cpu->tmpb, cpu->f & I286_FC);	// sbb
			break;
		case 4: cpu->tmpb = i286_and8(cpu, cpu->ltw, cpu->tmpb); break;			// and
		case 5:										// sub
		case 7: cpu->tmpb = i286_sub8(cpu, cpu->ltw, cpu->tmpb, 0); break;		// cmp
		case 6: cpu->tmpb = i286_xor8(cpu, cpu->ltw, cpu->tmpb); break;			// xor
	}
	if ((cpu->mod & 0x38) != 0x38)		// CMP drop result of SUB
		i286_wr_ea(cpu, cpu->tmpb, 0);
}

void i286_alu16(CPU* cpu) {
	switch((cpu->mod >> 3) & 7) {
		case 0: cpu->twrd = i286_add16(cpu, cpu->tmpw, cpu->twrd, 0); break;
		case 1: cpu->twrd = i286_or16(cpu, cpu->tmpw, cpu->twrd); break;
		case 2: cpu->twrd = i286_add16(cpu, cpu->tmpw, cpu->twrd, cpu->f & I286_FC);
			break;
		case 3: cpu->twrd = i286_sub16(cpu, cpu->tmpw, cpu->twrd, cpu->f & I286_FC);
			break;
		case 4: cpu->twrd = i286_and16(cpu, cpu->tmpw, cpu->twrd); break;
		case 5:
		case 7: cpu->twrd = i286_sub16(cpu, cpu->tmpw, cpu->twrd, 0); break;
		case 6: cpu->twrd = i286_xor16(cpu, cpu->tmpw, cpu->twrd); break;
	}
	if ((cpu->mod & 0x38) != 0x38)		// cmp
		i286_wr_ea(cpu, cpu->twrd, 1);
}

// 81: ALU ew,word
void i286_op81(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->twrd = i286_rd_immw(cpu);
	i286_alu16(cpu);
}

// 82: ALU eb,byte (==80)
void i286_op82(CPU* cpu) {
	i286_op80(cpu);
}

// 83: ALU ew,signed.byte
void i286_op83(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->lwr = i286_rd_imm(cpu);
	cpu->hwr = (cpu->lwr & 0x80) ? 0xff : 0x00;
	i286_alu16(cpu);
}

// 84,mod: test eb,rb = and w/o storing result
void i286_op84(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpb = i286_and8(cpu, cpu->ltw, cpu->lwr);
}

// 85,mod: test ew,rw
void i286_op85(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpw = i286_and16(cpu, cpu->tmpw, cpu->twrd);
}

// 86,mod: xchg eb,rb = swap values
void i286_op86(CPU* cpu) {
	i286_rd_ea(cpu, 0);	// tmpw=ea, twrd=reg
	i286_wr_ea(cpu, cpu->lwr, 0);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 87,mod: xchg ew,rw
void i286_op87(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	i286_wr_ea(cpu, cpu->twrd, 1);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 88,mod: mov eb,rb
void i286_op88(CPU* cpu) {
	i286_get_ea(cpu, 0);
	cpu->lwr = i286_get_reg(cpu, 0);
	i286_wr_ea(cpu, cpu->lwr, 0);
}

// 89,mod: mov ew,rw
void i286_op89(CPU* cpu) {
	i286_get_ea(cpu, 1);
	cpu->twrd = i286_get_reg(cpu, 1);
	i286_wr_ea(cpu, cpu->twrd, 1);
}

// 8a,mod: mov rb,eb
void i286_op8A(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	i286_set_reg(cpu, cpu->ltw, 0);
}

// 8b,mod: mov rw,ew
void i286_op8B(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// 8c,mod: mov ew,[es,cs,ss,ds]	TODO: ignore N.bit2?
void i286_op8C(CPU* cpu) {
	i286_get_ea(cpu, 1);
	switch((cpu->mod & 0x18) >> 3) {
		case 0: cpu->twrd = cpu->es.idx; break;
		case 1: cpu->twrd = cpu->cs.idx; break;
		case 2: cpu->twrd = cpu->ss.idx; break;
		case 3: cpu->twrd = cpu->ds.idx; break;
	}
	i286_wr_ea(cpu, cpu->twrd, 1);
}

// 8d: lea rw,ea	rw = ea.offset
void i286_op8D(CPU* cpu) {
	i286_get_ea(cpu, 0);
	if (cpu->ea.reg) {	// 2nd operand is register
		THROW(I286_INT_UD);
	} else {
		i286_set_reg(cpu, cpu->ea.adr, 1);
	}
}

// 8e,mod: mov [es,not cs,ss,ds],ew
void i286_op8E(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	switch((cpu->mod & 0x38) >> 3) {
		case 0:	cpu->es = i286_cash_seg(cpu, cpu->tmpw); break;
		case 1: break;
		case 2: cpu->ss = i286_cash_seg(cpu, cpu->tmpw); break;
		case 3: cpu->ds = i286_cash_seg(cpu, cpu->tmpw); break;
	}
}

// 8f, mod: pop ew
void i286_op8F(CPU* cpu) {
	i286_get_ea(cpu, 0);
	cpu->tmpw = i286_pop(cpu);
	if (cpu->ea.reg) {
		i286_set_reg(cpu, cpu->tmpw, 1);
	} else {
		i286_wr_ea(cpu, cpu->tmpw, 1);
	}
}

// 90: nop = xchg ax,ax
void i286_op90(CPU* cpu) {}

// 91: xchg ax,cx
void i286_op91(CPU* cpu) {
	cpu->tmpw = cpu->ax;
	cpu->ax = cpu->cx;
	cpu->cx = cpu->tmpw;
}

// 92: xchg ax,dx
void i286_op92(CPU* cpu) {
	cpu->tmpw = cpu->ax;
	cpu->ax = cpu->dx;
	cpu->dx = cpu->tmpw;
}

// 93: xchg ax,bx
void i286_op93(CPU* cpu) {
	cpu->tmpw = cpu->ax;
	cpu->ax = cpu->bx;
	cpu->bx = cpu->tmpw;
}

// 94: xchg ax,sp
void i286_op94(CPU* cpu) {
	cpu->tmpw = cpu->ax;
	cpu->ax = cpu->sp;
	cpu->sp = cpu->tmpw;
}

// 95:xchg ax,bp
void i286_op95(CPU* cpu) {
	cpu->tmpw = cpu->ax;
	cpu->ax = cpu->bp;
	cpu->bp = cpu->tmpw;
}

// 96:xchg ax,si
void i286_op96(CPU* cpu) {
	cpu->tmpw = cpu->ax;
	cpu->ax = cpu->si;
	cpu->si = cpu->tmpw;
}

// 97:xchg ax,di
void i286_op97(CPU* cpu) {
	cpu->tmpw = cpu->ax;
	cpu->ax = cpu->di;
	cpu->di = cpu->tmpw;
}

// 98:cbw : sign extend AL to AX
void i286_op98(CPU* cpu) {
	cpu->ah = (cpu->al & 0x80) ? 0xff : 0x00;
}

// 99:cwd : sign extend AX to DX:AX
void i286_op99(CPU* cpu) {
	cpu->dx = (cpu->ah & 0x80) ? 0xffff : 0x0000;
}

// callf to ncs:nip
void i286_callf(CPU* cpu, int nip, int ncs) {
	if (cpu->msw & I286_FPE) {
		i286_check_gate(cpu, nip, ncs);		// ea.seg:ea.adr is new destination
		if (cpu->ea.reg) {		// call gate?
			if (cpu->ea.seg.pl < cpu->cs.pl) {	// more priv
				unsigned short osp = cpu->sp;
				unsigned short oss = cpu->ss.idx;
				unsigned int sadr = cpu->ss.base + cpu->sp;		// old stack top
				cpu->sp = i286_sys_mrdw(cpu, cpu->tsdr, 2 + (4 * cpu->ea.seg.pl));			// ss:sp for new PL
				cpu->ss = i286_cash_seg(cpu, i286_sys_mrdw(cpu, cpu->tsdr, 4 + (4 * cpu->ea.seg.pl)));
				i286_push(cpu, oss);		// push old sp
				i286_push(cpu, osp);
				cpu->cs = cpu->ea.seg;
				cpu->pc = cpu->ea.adr;
				int cnt = (cpu->ea.cnt << 1);
				cpu->sp -= cnt;	// space for params
				int dadr = cpu->ss.base + cpu->sp;			// new stack top
				cpu->t = 86;
				while (cnt > 0) {
					cpu->mwr(dadr, cpu->mrd(sadr, 0, cpu->xptr), cpu->xptr);
					dadr++;
					sadr++;
					cnt--;
				}
			} else {
				cpu->t = 82;
			}
		} else {
			cpu->t = 41;
		}
		i286_push(cpu, cpu->cs.idx);		// push return addr
		i286_push(cpu, cpu->pc);
		cpu->cs = cpu->ea.seg;
		cpu->pc = cpu->ea.adr;
	} else {
		i286_push(cpu, cpu->cs.idx);
		i286_push(cpu, cpu->pc);
		cpu->cs = i286_cash_seg(cpu, ncs);
		cpu->pc = nip;
		cpu->t = 41;
	}
}

// 9a: callf cd (cd=SEG:ADR)
void i286_op9A(CPU* cpu) {
	unsigned short nip = i286_rd_immw(cpu);		// offset
	unsigned short ncs = i286_rd_immw(cpu);		// segment
	i286_callf(cpu, nip, ncs);
}

// 9b: wait
void i286_op9B(CPU* cpu) {
	// wait for busy=0
	if (cpu->msw & I286_FMP) {
		THROW(I286_INT_NM);		// int 7
	} else if ((cpu->msw & I286_FTS) && (cpu->msw & I286_FMP)) {
		THROW(I286_INT_NM);		// int 7
	} else if (cpu->x87sr & 0x80) {
		THROW(I286_INT_MF);		// int 16
	}
}

// 9c: pushf
void i286_op9C(CPU* cpu) {
	i286_push(cpu, cpu->f);
}

// 9d: popf (real mode: NT and IOP flags not modified)
// real mode: IOP can changed if cpl=0, FI - if (cpl <= iop)
// TODO: 80286 - only low 8 bits changed in real mode?
void i286_op9D(CPU* cpu) {
	cpu->tmpw = i286_pop(cpu);
	if (cpu->msw & I286_FPE) {
		cpu->f = (cpu->f & (I286_FN | I286_FIP)) | (cpu->tmpw & ~(I286_FN | I286_FIP));
	} else {
		cpu->f = cpu->tmpw;
	}
}

// 9e: sahf
void i286_op9E(CPU* cpu) {
	cpu->f &= ~(I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
	cpu->f |= cpu->ah & (I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
}

// 9f: lahf
void i286_op9F(CPU* cpu) {
	cpu->ah &= ~(I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
	cpu->ah |= cpu->f & (I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
}

// a0,iw: mov al,[*ds:iw]
void i286_opA0(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->al = i286_mrd(cpu, cpu->ds, 1, cpu->tmpw);
}

// a1,iw: mov ax,[*ds:iw]
void i286_opA1(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	if (cpu->tmpw == 0xffff) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		cpu->al = i286_mrd(cpu, cpu->ds, 1, cpu->tmpw);
		cpu->ah = i286_mrd(cpu, cpu->ds, 1, cpu->tmpw + 1);
	}
}

// a2,xb: mov [iw],al
void i286_opA2(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	i286_mwr(cpu, cpu->ds, 1, cpu->tmpw, cpu->al);
}

// a3,xw: mov [iw],ax
void i286_opA3(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	if (cpu->tmpw == 0xffff) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		i286_mwr(cpu, cpu->ds, 1, cpu->tmpw, cpu->al);
		i286_mwr(cpu, cpu->ds, 1, cpu->tmpw + 1, cpu->ah);
	}
}

// a4: movsb: [*ds:si]->[es:di], si,di ++ or --
void i286_a4_cb(CPU* cpu) {
	cpu->tmp = i286_mrd(cpu, cpu->ds, 1, cpu->si);
	i286_mwr(cpu, cpu->es, 0, cpu->di, cpu->tmp);
	if (cpu->f & I286_FD) {
		cpu->si--;
		cpu->di--;
	} else {
		cpu->si++;
		cpu->di++;
	}
}
void i286_opA4(CPU* cpu) {i286_rep(cpu, i286_a4_cb);}

// a5: movsw [*ds:si]->[es:di] by word, si,di +/-= 2
void i286_a5_cb(CPU* cpu) {
	cpu->ltw = i286_mrd(cpu, cpu->ds, 1, cpu->si);
	cpu->htw = i286_mrd(cpu, cpu->ds, 1, cpu->si + 1);
	i286_mwr(cpu, cpu->es, 0, cpu->di, cpu->ltw);
	i286_mwr(cpu, cpu->es, 0, cpu->di + 1, cpu->htw);
	if (cpu->f & I286_FD) {
		cpu->si -= 2;
		cpu->di -= 2;
	} else {
		cpu->si += 2;
		cpu->di += 2;
	}
}
void i286_opA5(CPU* cpu) {
	if ((cpu->si == 0xffff) || (cpu->di == 0xffff)) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		i286_rep(cpu, i286_a5_cb);
	}
}

// check condition for repz/repnz for cmps/scas opcodes
void i286_rep_fz(CPU* cpu, cbcpu foo) {
	if (cpu->rep == I286_REP_NONE) {
		foo(cpu);
	} else if (cpu->cx) {
		cpu->cx--;
		foo(cpu);
		if ((cpu->rep == I286_REPZ) && (cpu->f & I286_FZ) && cpu->cx) {
			cpu->pc = cpu->oldpc;
		} else if ((cpu->rep == I286_REPNZ) && !(cpu->f & I286_FZ) && cpu->cx) {
			cpu->pc = cpu->oldpc;
		}
	}
}

// a6: cmpsb: cmp [*ds:si]-[es:di], adv si,di
void i286_a6_cb(CPU* cpu) {
	cpu->ltw = i286_mrd(cpu, cpu->ds, 1, cpu->si);
	cpu->lwr = i286_mrd(cpu, cpu->es, 0, cpu->di);
	cpu->htw = i286_sub8(cpu, cpu->ltw, cpu->lwr, 0);
	if (cpu->f & I286_FD) {
		cpu->si--;
		cpu->di--;
	} else {
		cpu->si++;
		cpu->di++;
	}
}
void i286_opA6(CPU* cpu) {
	i286_rep_fz(cpu, i286_a6_cb);
}

// a7: cmpsw
void i286_a7_cb(CPU* cpu) {
	cpu->ltw = i286_mrd(cpu, cpu->ds, 1, cpu->si);
	cpu->htw = i286_mrd(cpu, cpu->ds, 1, cpu->si + 1);
	cpu->lwr = i286_mrd(cpu, cpu->es, 0, cpu->di);
	cpu->hwr = i286_mrd(cpu, cpu->es, 0, cpu->di + 1);
	cpu->tmpw = i286_sub16(cpu, cpu->tmpw, cpu->twrd, 0);
	if (cpu->f & I286_FD) {
		cpu->si -= 2;
		cpu->di -= 2;
	} else {
		cpu->si += 2;
		cpu->di += 2;
	}
}
void i286_opA7(CPU* cpu) {
	if ((cpu->si == 0xffff) || (cpu->di == 0xffff)) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		i286_rep_fz(cpu, i286_a7_cb);
	}
}

// a8,byte: test al,byte
void i286_opA8(CPU* cpu) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->lwr = i286_and8(cpu, cpu->al, cpu->ltw);
}

// a9,wrd: test ax,wrd
void i286_opA9(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->twrd = i286_and16(cpu, cpu->ax, cpu->tmpw);
}

// aa: stosb  al->[es:di], adv di
void i286_aa_cb(CPU* cpu) {
	i286_mwr(cpu, cpu->es, 0, cpu->di, cpu->al);
	cpu->di += (cpu->f & I286_FD) ? -1 : 1;
}
void i286_opAA(CPU* cpu) {i286_rep(cpu, i286_aa_cb);}

// ab: stosw: ax->[es:di], adv di
void i286_ab_cb(CPU* cpu) {
	i286_mwr(cpu, cpu->es, 0, cpu->di, cpu->al);
	i286_mwr(cpu, cpu->es, 0, cpu->di + 1, cpu->ah);
	cpu->di += (cpu->f & I286_FD) ? -2 : 2;
}
void i286_opAB(CPU* cpu) {
	if (cpu->di == 0xffff) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		i286_rep(cpu, i286_ab_cb);
	}
}

// ac: lodsb: [*ds:si]->al, adv si
void i286_opAC(CPU* cpu) {
	cpu->al = i286_mrd(cpu, cpu->ds, 1, cpu->si);
	cpu->si += (cpu->f & I286_FD) ? -1 : 1;
}

// ad: lodsw [*ds:si]->ax, adv si
void i286_opAD(CPU* cpu) {
	if (cpu->si == 0xffff) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		cpu->al = i286_mrd(cpu, cpu->ds, 1, cpu->si);
		cpu->ah = i286_mrd(cpu, cpu->ds, 1, cpu->si + 1);
		cpu->si += (cpu->f & I286_FD) ? -2 : 2;
	}
}

// ae: scasb	cmp al,[es:di]
void i286_ae_cb(CPU* cpu) {
	cpu->ltw = i286_mrd(cpu, cpu->es, 0, cpu->di);
	cpu->lwr = i286_sub8(cpu, cpu->al, cpu->ltw, 0);
	cpu->di += (cpu->f & I286_FD) ? -1 : 1;
}
void i286_opAE(CPU* cpu) {
	i286_rep_fz(cpu, i286_ae_cb);
}

// af: scasw	cmp ax,[es:di]
void i286_af_cb(CPU* cpu) {
	cpu->ltw = i286_mrd(cpu, cpu->es, 0, cpu->di);
	cpu->htw = i286_mrd(cpu, cpu->es, 0, cpu->di + 1);
	cpu->twrd = i286_sub16(cpu, cpu->ax, cpu->tmpw, 0);
	cpu->di += (cpu->f & I286_FD) ? -2 : 2;
}
void i286_opAF(CPU* cpu) {
	if (cpu->di == 0xffff) {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	} else {
		i286_rep_fz(cpu, i286_af_cb);
	}
}

// b0..b7,ib: mov rb,ib
void i286_opB0(CPU* cpu) {cpu->al = i286_rd_imm(cpu);}
void i286_opB1(CPU* cpu) {cpu->cl = i286_rd_imm(cpu);}
void i286_opB2(CPU* cpu) {cpu->dl = i286_rd_imm(cpu);}
void i286_opB3(CPU* cpu) {cpu->bl = i286_rd_imm(cpu);}
void i286_opB4(CPU* cpu) {cpu->ah = i286_rd_imm(cpu);}
void i286_opB5(CPU* cpu) {cpu->ch = i286_rd_imm(cpu);}
void i286_opB6(CPU* cpu) {cpu->dh = i286_rd_imm(cpu);}
void i286_opB7(CPU* cpu) {cpu->bh = i286_rd_imm(cpu);}

// b8..bf,iw: mov rw,iw
void i286_opB8(CPU* cpu) {cpu->ax = i286_rd_immw(cpu);}
void i286_opB9(CPU* cpu) {cpu->cx = i286_rd_immw(cpu);}
void i286_opBA(CPU* cpu) {cpu->dx = i286_rd_immw(cpu);}
void i286_opBB(CPU* cpu) {cpu->bx = i286_rd_immw(cpu);}
void i286_opBC(CPU* cpu) {cpu->sp = i286_rd_immw(cpu);}
void i286_opBD(CPU* cpu) {cpu->bp = i286_rd_immw(cpu);}
void i286_opBE(CPU* cpu) {cpu->si = i286_rd_immw(cpu);}
void i286_opBF(CPU* cpu) {cpu->di = i286_rd_immw(cpu);}

// rotate/shift

// rol: FC<-b7...b0<-b7
unsigned char i286_rol8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p << 1) | ((p & 0x80) ? 1 : 0);
	if (p & 1) cpu->f |= I286_FC;
	if (!(cpu->f & I286_FC) != !(p & 0x80)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_rol16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p << 1) | ((p & 0x8000) ? 1 : 0);
	if (p & 1) cpu->f |= I286_FC;
	if (!(cpu->f & I286_FC) != !(p & 0x8000)) cpu->f |= I286_FO;
	return p;
}

// ror: b0->b7...b0->CF
unsigned char i286_ror8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p >> 1) | ((p & 1) ? 0x80 : 0);
	if (p & 0x80) cpu->f |= I286_FC;
	if (!(p & 0x80) != !(p & 0x40)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_ror16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO);
	p = (p >> 1) | ((p & 1) ? 0x8000 : 0);
	if (p & 0x8000) cpu->f |= I286_FC;
	if (!(p & 0x8000) != !(p & 0x4000)) cpu->f |= I286_FO;
	return p;
}

// rcl: CF<-b7..b0<-CF
unsigned char i286_rcl8(CPU* cpu, unsigned char p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 0x80) cpu->f |= I286_FC;
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	if (!(cpu->f & I286_FC) != !(p & 0x80)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_rcl16(CPU* cpu, unsigned short p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 0x8000) cpu->f |= I286_FC;
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	if (!(cpu->f & I286_FC) != !(p & 0x8000)) cpu->f |= I286_FO;
	return p;
}

// rcr: CF->b7..b0->CF
unsigned char i286_rcr8(CPU* cpu, unsigned char p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 1) cpu->f |= I286_FC;
	p >>= 1;
	if (cpu->tmp) p |= 0x80;
	if (!(p & 0x80) != !(p & 0x40)) cpu->f |= I286_FO;
	return p;
}

unsigned short i286_rcr16(CPU* cpu, unsigned short p) {
	cpu->tmp = (cpu->f & I286_FC);
	cpu->f &= ~(I286_FC | I286_FO);
	if (p & 1) cpu->f |= I286_FC;
	p >>= 1;
	if (cpu->tmp) p |= 0x8000;
	if (!(p & 0x8000) != !(p & 0x4000)) cpu->f |= I286_FO;
	return p;
}

// sal: CF<-b7..b0<-0
unsigned char i286_sal8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 0x80) cpu->f |= I286_FC;
	p <<= 1;
	if (!(cpu->f & I286_FC) != !(p & 0x80)) cpu->f |= I286_FO;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x80) cpu->f |= I286_FS;
	return p;
}

unsigned short i286_sal16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 0x8000) cpu->f |= I286_FC;
	p <<= 1;
	if (!(cpu->f & I286_FC) != !(p & 0x8000)) cpu->f |= I286_FO;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x8000) cpu->f |= I286_FS;
	return p;
}

// shr 0->b7..b0->CF
unsigned char i286_shr8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	if (p & 0x80) cpu->f |= I286_FO;
	p >>= 1;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x80) cpu->f |= I286_FS;
	return p;
}

unsigned short i286_shr16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	if (p & 0x8000) cpu->f |= I286_FO;
	p >>= 1;
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x8000) cpu->f |= I286_FS;
	return p;
}

// sar b7->b7..b0->CF
unsigned char i286_sar8(CPU* cpu, unsigned char p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	p = (p >> 1) | (p & 0x80);
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x80) cpu->f |= I286_FS;
	return p;
}

unsigned short i286_sar16(CPU* cpu, unsigned short p) {
	cpu->f &= ~(I286_FC | I286_FO | I286_FZ | I286_FP | I286_FS);
	if (p & 1) cpu->f |= I286_FC;
	p = (p >> 1) | (p & 0x8000);
	if (!p) cpu->f |= I286_FZ;
	if (parity(p & 0xff)) cpu->f |= I286_FP;
	if (p & 0x8000) cpu->f |= I286_FS;
	return p;
}

typedef unsigned char(*cb286rot8)(CPU*, unsigned char);
typedef unsigned short(*cb286rot16)(CPU*, unsigned short);

// shl (/4) = sal (/6)
static cb286rot8 i286_rot8_tab[8] = {
	i286_rol8, i286_ror8, i286_rcl8, i286_rcr8,
	i286_sal8, i286_shr8, i286_sal8, i286_sar8
};

static cb286rot16 i286_rot16_tab[8] = {
	i286_rol16, i286_ror16, i286_rcl16, i286_rcr16,
	i286_sal16, i286_shr16, i286_sal16, i286_sar16
};

void i286_rotsh8(CPU* cpu, int cnt) {
	// i286_rd_ea already called, cpu->tmpb is number of repeats
	// cpu->ltw = ea.byte, result should be back in cpu->ltw. flags is setted
	cnt &= 0x1f;		// only 5 bits is counted
	cpu->t += cnt;		// 1T for each iteration
	cb286rot8 foo = i286_rot8_tab[(cpu->mod >> 3) & 7];
	if (foo) {
		while (cnt) {
			cpu->ltw = foo(cpu, cpu->ltw);
			cnt--;
		}
	}
}

void i286_rotsh16(CPU* cpu, int cnt) {
	cnt &= 0x1f;
	cpu->t += cpu->tmpb;
	cb286rot16 foo = i286_rot16_tab[(cpu->mod >> 3) & 7];
	if (foo) {
		while (cnt) {
			cpu->tmpw = foo(cpu, cpu->tmpw);
			cnt--;
		}
	}
}

// c0,mod,db: rotate/shift ea byte db times
void i286_opC0(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->tmpb = i286_rd_imm(cpu);
	i286_rotsh8(cpu, cpu->tmpb);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// c1,mod,db: rotate/shift ea word db times
void i286_opC1(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->tmpb = i286_rd_imm(cpu);
	i286_rotsh16(cpu, cpu->tmpb);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// c2,iw: ret iw	pop pc, pop iw bytes
void i286_opC2(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->pc = i286_pop(cpu);
	cpu->sp += cpu->tmpw;
}

// c3: ret
void i286_opC3(CPU* cpu) {
	cpu->pc = i286_pop(cpu);
}

// c4,mod: les rw,ed
void i286_opC4(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->ltw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 0);		// address
	cpu->htw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 1);
	cpu->lwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 2);		// segment
	cpu->hwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 3);
	cpu->es = i286_cash_seg(cpu, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// c5,mod: lds rw,ed (same c4 with ds)
void i286_opC5(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->ltw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 0);		// address
	cpu->htw = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 1);
	cpu->lwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 2);		// segment
	cpu->hwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 3);
	cpu->ds = i286_cash_seg(cpu, cpu->twrd);
	i286_set_reg(cpu, cpu->tmpw, 1);
}

// c6,mod,ib: mov ea,ib
void i286_opC6(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	cpu->lwr = i286_rd_imm(cpu);
	i286_wr_ea(cpu, cpu->lwr, 0);
}

// c7,mod,iw: mov ea,iw
// mod 44
// [01:byte offset][000:ax][100:si+offset]
void i286_opC7(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	cpu->twrd = i286_rd_immw(cpu);
	i286_wr_ea(cpu, cpu->twrd, 1);
}

// c8,iw,ib: enter iw,ib
void i286_opC8(CPU* cpu) {
	cpu->t = 11;
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->tmpb = i286_rd_imm(cpu) & 0x1f;
	i286_push(cpu, cpu->bp);
	cpu->ea.adr = cpu->sp;
	if (cpu->tmpb > 0) {
		while(--cpu->tmpb) {
			cpu->bp -= 2;
			cpu->lwr = i286_mrd(cpu, cpu->ds, 1, cpu->bp);		// +1T		TODO: segment override?
			cpu->hwr = i286_mrd(cpu, cpu->ds, 1, cpu->bp + 1);	// +1T
			i286_push(cpu, cpu->twrd);				// +2T
		}
		i286_push(cpu, cpu->ea.adr);					// +2T (1?)
	}
	cpu->bp = cpu->ea.adr;
	cpu->sp -= cpu->tmpw;
}

// c9: leave
void i286_opC9(CPU* cpu) {
	cpu->sp = cpu->bp;
	cpu->bp = i286_pop(cpu);
}

// ca,iw: retf iw	pop ip,cs,iw bytes	15/25/55T
void i286_opCA(CPU* cpu) {
	cpu->twrd = i286_rd_immw(cpu);
	cpu->pc = i286_pop(cpu);
	cpu->tmpw = i286_pop(cpu);
	cpu->cs = i286_cash_seg(cpu, cpu->tmpw);
	cpu->sp += cpu->twrd;
	cpu->t += cpu->twrd;
}

// cb: retf	pop ip,cs
void i286_opCB(CPU* cpu) {
	cpu->pc = i286_pop(cpu);
	cpu->tmpw = i286_pop(cpu);
	cpu->cs = i286_cash_seg(cpu, cpu->tmpw);
}

// cc: int 3
void i286_opCC(CPU* cpu) {
	i286_interrupt(cpu, 3);
}

// cd,ib: int ib
void i286_opCD(CPU* cpu) {
	cpu->tmp = i286_rd_imm(cpu);
	i286_interrupt(cpu, cpu->tmp);
}

// ce: into	int 4 if FO=1
void i286_opCE(CPU* cpu) {
	if (cpu->f & I286_FO)
		i286_interrupt(cpu, I286_INT_OF);
}

// cf: iret	pop ip,cs,flag
void i286_opCF(CPU* cpu) {
	if (cpu->msw & I286_FPE) {		// protected
		xSegPtr seg;
		if (cpu->f & I286_FN) {
			int tss = i286_sys_mrdw(cpu, cpu->tsdr, 0);	// back link
			if (tss & 4) {		// must be in GDT
				THROW_EC(I286_INT_TS, tss);
			} else if ((tss & 0xfff8) > cpu->gdtr.limit) {
				THROW_EC(I286_INT_TS, tss);
			} else {
				seg = i286_cash_seg(cpu, tss);	// new tss
				if ((seg.ar & 0x0f) != 3) {	// busy TSS
					THROW_EC(I286_INT_TS, tss);
				} else if (!seg.pr) {	// present
					THROW_EC(I286_INT_NP, tss);
				}
				seg = cpu->tsdr;			// current TS
				i286_switch_task(cpu, tss, 0, 1);	// switch to new task
				unsigned char tf = i286_sys_mrd(cpu, seg, 5);	// mark old TS as not-busy
				tf &= ~2;
				i286_sys_mwr(cpu, seg, 5, tf);
				if (cpu->pc > cpu->cs.limit) {
					THROW_EC(I286_INT_GP, 0);
				}
			}
		} else {
			unsigned short nip = i286_sys_mrdw(cpu, cpu->ss, 0);	// return ip
			unsigned short ncs = i286_sys_mrdw(cpu, cpu->ss, 2);	// return cs selector
			seg = i286_cash_seg(cpu, ncs);
			if (((ncs >> 3) & 0x60) == cpu->cs.pl) {		// rpl = cpl: return at same PL
				if ((ncs & 0xfff8) == 0) {
					THROW_EC(I286_INT_GP, 0);
				} else if ((ncs & 3) < cpu->cs.pl) {
					THROW_EC(I286_INT_GP, ncs);
				} else if ((seg.ar & 0x18) != 0x18) {
					THROW_EC(I286_INT_GP, ncs);
				} else if ((seg.ar & 4) && (seg.pl > cpu->cs.pl)) {	// conforming and DPL > CPL
					THROW_EC(I286_INT_GP, ncs);
				} else if (!(seg.ar & 4) && (seg.pl != cpu->cs.pl)) {	// non-conforming and DPL != CPL
					THROW_EC(I286_INT_GP, ncs);
				} else if (!seg.pr) {		// present
					THROW_EC(I286_INT_NP, ncs);
				} else if (nip > seg.limit) {
					THROW_EC(I286_INT_GP, 0);
				} else {
					cpu->pc = i286_pop(cpu);
					cpu->cs = i286_cash_seg(cpu, i286_pop(cpu));
					cpu->f = i286_pop(cpu);
				}
			} else {					// return at lower PL
				if ((ncs & 0xfff8) == 0) {
					THROW_EC(I286_INT_GP, 0);
				} else if ((seg.ar & 0x18) != 0x18) {
					THROW_EC(I286_INT_GP, ncs);
				} else if ((seg.ar & 4) && (seg.pl <= cpu->cs.pl)) {
					THROW_EC(I286_INT_GP, ncs);
				} else if (!(seg.ar & 4) && (seg.pl != (ncs & 3))) {
					THROW_EC(I286_INT_GP, ncs);
				} else if (!seg.pr) {
					THROW_EC(I286_INT_NP, ncs);
				} else {
					cpu->pc = i286_pop(cpu);
					cpu->cs = i286_cash_seg(cpu, i286_pop(cpu));
					cpu->f = i286_pop(cpu);
					int nsp = i286_pop(cpu);
					cpu->ss = i286_cash_seg(cpu, i286_pop(cpu));
					cpu->sp = nsp;
				}


			}
		}
	} else {				// real
		cpu->pc = i286_pop(cpu);
		cpu->tmpw = i286_pop(cpu);
		cpu->f = i286_pop(cpu);
		cpu->cs = i286_cash_seg(cpu, cpu->tmpw);
	}
	cpu->inten &= ~I286_BLK_NMI;		// nmi on
}

// d0,mod: rot/shift ea.byte 1 time
void i286_opD0(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	i286_rotsh8(cpu, 1);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// d1,mod: rot/shift ea.word 1 time
void i286_opD1(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	i286_rotsh16(cpu, 1);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// d2,mod: rot/shift ea.byte CL times
void i286_opD2(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	i286_rotsh8(cpu, cpu->cl);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

// d3,mod: rot/shift ea.word CL times
void i286_opD3(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	i286_rotsh16(cpu, cpu->cl);
	i286_wr_ea(cpu, cpu->tmpw, 1);
}

// d4 0a: aam ib
void i286_opD4(CPU* cpu) {
	cpu->tmpb = i286_rd_imm(cpu);
	if (cpu->tmpb == 0) {
		THROW(-1);	// I286_INT_DE = 0
	} else {
		cpu->ah = cpu->al / cpu->tmpb;
		cpu->al = cpu->al % cpu->tmpb;
		cpu->f &= ~(I286_FS | I286_FZ | I286_FP);
		if (cpu->ah & 0x80) cpu->f |= I286_FS;
		if (!cpu->ax) cpu->f |= I286_FZ;
		if (parity(cpu->ax & 0xff)) cpu->f |= I286_FP;
	}
}

// d5 0a: aad ib
void i286_opD5(CPU* cpu) {
	cpu->tmpb = i286_rd_imm(cpu);
	cpu->al = cpu->ah * cpu->tmpb + cpu->al;
	cpu->ah = 0;
	cpu->f &= ~(I286_FS | I286_FZ | I286_FP);
	if (cpu->al & 0x80) cpu->f |= I286_FS;
	if (!cpu->al) cpu->f |= I286_FZ;
	if (parity(cpu->al & 0xff)) cpu->f |= I286_FP;
}

// d6:
void i286_opD6(CPU* cpu) {
	THROW(I286_INT_UD);
}

// d7: xlatb	al = [ds:bx+al]		// segment replacement must work
void i286_opD7(CPU* cpu) {
	cpu->tmpw = cpu->bx + cpu->al;
	cpu->al = i286_mrd(cpu, cpu->ds, 1, cpu->tmpw);
}

// 80287 template
// opcodes: D8-DF (11011xxx),mod,[data,[data]]

extern void x87_exec(CPU*);

void i286_fpu(CPU* cpu) {
	i286_get_ea(cpu, 1);
	if (cpu->msw & I286_FEM) {
		THROW(I286_INT_NM);
	} else if (cpu->x87sr & 0x80) {		// x87 exception
		THROW(I286_INT_MF);
	} else {
//		printf("%.8X : x87 : %.2X %.2X\n", cpu->cs.base + cpu->oldpc, cpu->com, cpu->mod);
//		THROW(I286_INT_NM);
		x87_exec(cpu);
	}
}

// e0,cb: loopnz cb:	cx--,jump short if (cx!=0)&&(fz=0)
void i286_opE0(CPU* cpu) {
	cpu->cx--;
	i286_jr(cpu, cpu->cx && !(cpu->f & I286_FZ));
}

// e1,cb: loopz cb
void i286_opE1(CPU* cpu) {
	cpu->cx--;
	i286_jr(cpu, cpu->cx && (cpu->f & I286_FZ));
}

// e2,cb: loop cb	check only cx
void i286_opE2(CPU* cpu) {
	cpu->cx--;
	i286_jr(cpu, cpu->cx);
}

// e3: jcxz cb
void i286_opE3(CPU* cpu) {
	i286_jr(cpu, !cpu->cx);
}

// e4,ib: in al,ib	al = in(ib)
void i286_opE4(CPU* cpu) {
	cpu->tmpb = i286_rd_imm(cpu);
	cpu->al = i286_ird(cpu, cpu->tmpb) & 0xff;
}

// e5,ib: in ax,ib	ax = in(ib)[x2]
void i286_opE5(CPU* cpu) {
	cpu->tmpb = i286_rd_imm(cpu);
	cpu->ax = i286_ird(cpu, cpu->tmpb);
}

// e6,ib: out ib,al	out(ib),al
void i286_opE6(CPU* cpu) {
	cpu->tmpb = i286_rd_imm(cpu);
	i286_iwr(cpu, cpu->tmpb, cpu->al, 0);
}

// e7,ib: out ib,ax	out(ib),ax
void i286_opE7(CPU* cpu) {
	cpu->tmpb = i286_rd_imm(cpu);
	i286_iwr(cpu, cpu->tmpb, cpu->ax, 1);
}

// e8,cw: call cw	(relative call)
void i286_opE8(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	i286_push(cpu, cpu->pc);
	cpu->pc += cpu->tmpw;
}

// e9,cw: jmp cw	(relative jump)
void i286_opE9(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);
	cpu->pc += cpu->tmpw;
}

// common: jmpf to ncs:nip using gates in protected mode
void i286_jmpf(CPU* cpu, int nip, int ncs) {
	if (cpu->msw & I286_FPE) {		// protected
		i286_check_gate(cpu, nip, ncs);
		cpu->pc = cpu->ea.adr;
		cpu->cs = cpu->ea.seg;
	} else {				// real
		cpu->pc = nip;
		cpu->cs = i286_cash_seg(cpu, ncs);
	}
}

// ea,ow,sw: jmpf ow:sw	(far jump)
void i286_opEA(CPU* cpu) {
	cpu->tmpw = i286_rd_immw(cpu);	// ip
	cpu->twrd = i286_rd_immw(cpu);	// cs.selector
	i286_jmpf(cpu, cpu->tmpw, cpu->twrd);
}

// eb,cb: jump cb	(short jump)
void i286_opEB(CPU* cpu) {
	cpu->ltw = i286_rd_imm(cpu);
	cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
	cpu->pc += cpu->tmpw;
}

// ec: in al,dx
void i286_opEC(CPU* cpu) {
	cpu->al = i286_ird(cpu, cpu->dx) & 0xff;
}

// ed: in ax,dx
void i286_opED(CPU* cpu) {
	cpu->ax = i286_ird(cpu, cpu->dx);
}

// ee: out dx,al
void i286_opEE(CPU* cpu) {
	i286_iwr(cpu, cpu->dx, cpu->al, 0);
}

// ef: out dx,ax
void i286_opEF(CPU* cpu) {
	i286_iwr(cpu, cpu->dx, cpu->ax, 1);
}

// f0: lock prefix (for multi-CPU)
void i286_opF0(CPU* cpu) {
	cpu->lock = 1;
}

// f1: undef, doesn't cause interrupt
void i286_opF1(CPU* cpu) {}

// f2: REPNZ prefix for scas/cmps: repeat until Z=1
void i286_opF2(CPU* cpu) {
	cpu->rep = I286_REPNZ;
}

// f3: REPZ prefix for scas/cmps: repeat until Z=0
// f3: REP prefix for ins/movs/outs/stos: cx--,repeat if cx!=0
void i286_opF3(CPU* cpu) {
	cpu->rep = I286_REPZ;
}

// f4: hlt	halt until interrupt
void i286_opF4(CPU* cpu) {
	if (!((cpu->intrq & cpu->inten) && (cpu->f & I286_FI))) {
		cpu->halt = 1;
		cpu->pc = cpu->oldpc;
	} else {
		cpu->halt = 0;
	}
}

// f5:cmc
void i286_opF5(CPU* cpu) {
	cpu->f ^= I286_FC;
}

// f6,mod:
void i286_opF60(CPU* cpu) {		// test eb,ib
	cpu->tmpb = i286_rd_imm(cpu);
	cpu->tmpb = i286_and8(cpu, cpu->ltw, cpu->tmpb);
}

void i286_opF62(CPU* cpu) {		// not eb
	i286_wr_ea(cpu, cpu->ltw ^ 0xff, 0);
}

void i286_opF63(CPU* cpu) {		// neg eb
	cpu->ltw = i286_sub8(cpu, 0, cpu->ltw, 0);
	i286_wr_ea(cpu, cpu->ltw, 0);
}

void i286_opF64(CPU* cpu) {		// mul eb
	cpu->ax = cpu->ltw * cpu->al;
	cpu->f &= ~(I286_FO | I286_FC);
	if (cpu->ah) cpu->f |= (I286_FC | I286_FO);
}

void i286_opF65(CPU* cpu) {		// imul eb
	cpu->ax = (signed char)cpu->ltw * (signed char)cpu->al;
	cpu->f &= ~(I286_FO | I286_FC);
	if (cpu->ah != ((cpu->al & 0x80) ? 0xff : 0x00)) cpu->f |= (I286_FO | I286_FC);
}

void i286_opF66(CPU* cpu) {		// div eb
	if (cpu->ltw == 0) {				// div by zero
		THROW(-1);	// I286_INT_DE
	} else {
		if (cpu->ax / cpu->ltw > 0xff) {	// cpu->ah >= cpu->ltw ?
			THROW(-1);	// I286_INT_DE
		} else {
			cpu->twrd = cpu->ax % cpu->ltw;
			cpu->tmpw = cpu->ax / cpu->ltw;
			cpu->al = cpu->ltw;
			cpu->ah = cpu->lwr;
		}
	}
}

void i286_opF67(CPU* cpu) {		// idiv eb
	if (cpu->ltw == 0) {
		THROW(-1); //	I286_INT_DE
	} else {
		// TODO: int0 if quo>0xff
		if (cpu->ax / cpu->ltw > 0xff) {	// cpu->ah >= cpu->ltw
			THROW(-1);	// I286_INT_DE);
		} else {
			cpu->twrd = (signed short)cpu->ax % (signed char)cpu->ltw;
			cpu->tmpw = (signed short)cpu->ax / (signed char)cpu->ltw;
			cpu->al = cpu->ltw;
			cpu->ah = cpu->lwr;
		}
	}
}

cbcpu i286_F6_tab[8] = {
	i286_opF60, i286_opF60, i286_opF62, i286_opF63,
	i286_opF64, i286_opF65, i286_opF66, i286_opF67
};

void i286_opF6(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	i286_F6_tab[(cpu->mod >> 3) & 7](cpu);
}

// f7,mod:

void i286_opF70(CPU* cpu) {		// test ew,iw
	cpu->twrd = i286_rd_immw(cpu);
	cpu->twrd = i286_and16(cpu, cpu->tmpw, cpu->twrd);
}

void i286_opF72(CPU* cpu) {		// not ew
	i286_wr_ea(cpu, cpu->tmpw ^ 0xffff, 1);
}

void i286_opF73(CPU* cpu) {		// neg ew
	cpu->twrd = i286_sub16(cpu, 0, cpu->tmpw, 0);
	i286_wr_ea(cpu, cpu->twrd, 1);
}

void i286_opF74(CPU* cpu) {		// mul ew
	cpu->tmpi = cpu->tmpw * cpu->ax;
	cpu->ax = cpu->tmpi & 0xffff;
	cpu->dx = (cpu->tmpi >> 16) & 0xffff;
	cpu->f &= ~(I286_FO | I286_FC);
	if (cpu->dx) cpu->f |= (I286_FC | I286_FO);
}

void i286_opF75(CPU* cpu) {		// imul ew
	cpu->tmpi = (signed short)cpu->tmpw * (signed short)cpu->ax;
	cpu->ax = cpu->tmpi & 0xffff;
	cpu->dx = (cpu->tmpi >> 16) & 0xffff;
	cpu->f &= ~(I286_FO | I286_FC);
	if (cpu->dx != ((cpu->ah & 0x80) ? 0xff : 0x00)) cpu->f |= (I286_FO | I286_FC);
}

void i286_opF76(CPU* cpu) {		// div ew
	if (cpu->tmpw == 0) {				// div by zero
		THROW(-1); // I286_INT_DE
	} else {
		cpu->tmpi = (cpu->dx << 16) | cpu->ax;
		if (cpu->tmpi / cpu->tmpw > 0xffff) {		// cpu->dx >= cpu->tmpw
			THROW(-1);	// I286_INT_DE
		} else {
			cpu->ax = cpu->tmpi / cpu->tmpw;
			cpu->dx = cpu->tmpi % cpu->tmpw;
		}
	}
}

void i286_opF77(CPU* cpu) {		// idiv ew
	if (cpu->tmpw == 0) {
		THROW(-1);	// I286_INT_DE
	} else {
		if ((signed int)cpu->tmpi / (signed short)cpu->tmpw > 0xffff) {		// cpu->dx >= cpu->tmpw ?
			THROW(-1);	// I286_INT_DE
		} else {
			cpu->tmpi = (cpu->dx << 16) | cpu->ax;
			cpu->ax = (signed int)cpu->tmpi / (signed short)cpu->tmpw;
			cpu->dx = (signed int)cpu->tmpi % (signed short)cpu->tmpw;
		}
	}
}

cbcpu i286_F7_tab[8] = {
	i286_opF70, i286_opF70, i286_opF72, i286_opF73,
	i286_opF74, i286_opF75, i286_opF76, i286_opF77
};

void i286_opF7(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	i286_F7_tab[(cpu->mod >> 3) & 7](cpu);
}

// f8: clc
void i286_opF8(CPU* cpu) {
	cpu->f &= ~I286_FC;
}

// f9: stc
void i286_opF9(CPU* cpu) {
	cpu->f |= I286_FC;
}

// fa: cli
void i286_opFA(CPU* cpu) {
	if (i286_check_iopl(cpu)) {
		cpu->f &= ~I286_FI;
	} else {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	}
}

// fb: sti
void i286_opFB(CPU* cpu) {
	if (i286_check_iopl(cpu)) {
		cpu->f |= I286_FI;
	} else {
		i286_push(cpu, 0);
		THROW(I286_INT_GP);
	}
}

// fc: cld
void i286_opFC(CPU* cpu) {
	cpu->f &= ~I286_FD;
}

// fd: std
void i286_opFD(CPU* cpu) {
	cpu->f |= I286_FD;
}

// fe: inc/dec ea.byte
void i286_opFE(CPU* cpu) {
	i286_rd_ea(cpu, 0);
	switch((cpu->mod >> 3) & 7) {
		case 0: cpu->ltw = i286_inc8(cpu, cpu->ltw);
			i286_wr_ea(cpu, cpu->ltw, 0);
			break;
		case 1: cpu->ltw = i286_dec8(cpu, cpu->ltw);
			i286_wr_ea(cpu, cpu->ltw, 0);
			break;
	}
}

// ff: extend ops ea.word
void i286_opFF(CPU* cpu) {
	i286_rd_ea(cpu, 1);
	switch((cpu->mod >> 3) & 7) {
		case 0: cpu->tmpw = i286_inc16(cpu, cpu->tmpw);
			i286_wr_ea(cpu, cpu->tmpw, 1);
			break;	// inc ew
		case 1: cpu->tmpw = i286_dec16(cpu, cpu->tmpw);
			i286_wr_ea(cpu, cpu->tmpw, 1);
			break;	// dec ew
		case 2:	i286_push(cpu, cpu->pc);
			cpu->pc = cpu->tmpw;
			break; // call ew
		case 3:	cpu->lwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 2);	// twrd = segment
			cpu->hwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 3);
			i286_callf(cpu, cpu->tmpw, cpu->twrd);
			break; // callf ed
		case 4: cpu->pc = cpu->tmpw;
			break; // jmp ew
		case 5:	cpu->lwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 2);
			cpu->hwr = i286_mrd(cpu, cpu->ea.seg, 1, cpu->ea.adr + 3);
			i286_jmpf(cpu, cpu->tmpw, cpu->twrd);
			break; // jmpf ed
		case 6: i286_push(cpu, cpu->tmpw);
			break; // push ew
		case 7:
			break;	// ???
	}
}

// :e - effective address/register
// :r - register (n)
// :1 - imm.byte
// :2 - imm.word
// :3 - byte signed offset
// :n - word signed offset
// :4 - far address
// :p - far address
// :s - segment register (n)
// :L - rep(repz)|repnz if present
// :A - add|or|adc|sbb|and|sub|xor|cmp
// :R - rol|ror|rcl|rcr|sal|shr|/6|sar
// :X - test|*test|not|neg|mul|imul|div|idiv
// :E - inc|dec|call|callf|jmp|jmpf|push|/7
// :Q - sldt|str|lldt|ltr|verr|verw|/6|/7
// :W - sgdt|sidt|lgdt|lidt|smsw|/5|lmsw|/7

opCode i80286_tab[256] = {
	{0, 1, i286_op00, 0, "add :e,:r"},
	{OF_WORD, 1, i286_op01, 0, "add :e,:r"},
	{0, 1, i286_op02, 0, "add :r,:e"},
	{OF_WORD, 1, i286_op03, 0, "add :r,:e"},
	{0, 1, i286_op04, 0, "add al,:1"},
	{0, 1, i286_op05, 0, "add ax,:2"},
	{0, 1, i286_op06, 0, "push es"},
	{0, 1, i286_op07, 0, "pop es"},
	{0, 1, i286_op08, 0, "or :e,:r"},
	{OF_WORD, 1, i286_op09, 0, "or :e,:r"},
	{0, 1, i286_op0A, 0, "or :r,:e"},
	{OF_WORD, 1, i286_op0B, 0, "or :r,:e"},
	{0, 1, i286_op0C, 0, "or al,:1"},
	{0, 1, i286_op0D, 0, "or ax,:2"},
	{0, 1, i286_op0E, 0, "push cs"},
	{OF_PREFIX, 1, i286_op0F, 0, "prefix 0F"},
	{0, 1, i286_op10, 0, "adc :e,:r"},
	{OF_WORD, 1, i286_op11, 0, "adc :e,:r"},
	{0, 1, i286_op12, 0, "adc :r,:e"},
	{OF_WORD, 1, i286_op13, 0, "adc :r,:e"},
	{0, 1, i286_op14, 0, "adc al,:1"},
	{0, 1, i286_op15, 0, "adc ax,:2"},
	{0, 1, i286_op16, 0, "push ss"},
	{0, 1, i286_op17, 0, "pop ss"},
	{0, 1, i286_op18, 0, "sbb :e,:r"},
	{OF_WORD, 1, i286_op19, 0, "sbb :e,:r"},
	{0, 1, i286_op1A, 0, "sbb :r,:e"},
	{OF_WORD, 1, i286_op1B, 0, "sbb :r,:e"},
	{0, 1, i286_op1C, 0, "sbb al,:1"},
	{0, 1, i286_op1D, 0, "sbb ax,:2"},
	{0, 1, i286_op1E, 0, "push ds"},
	{0, 1, i286_op1F, 0, "pop ds"},
	{0, 1, i286_op20, 0, "and :e,:r"},
	{OF_WORD, 1, i286_op21, 0, "and :e,:r"},
	{0, 1, i286_op22, 0, "and :r,:e"},
	{OF_WORD, 1, i286_op23, 0, "and :r,:e"},
	{0, 1, i286_op24, 0, "and al,:1"},
	{0, 1, i286_op25, 0, "and ax,:2"},
	{OF_PREFIX, 1, i286_op26, 0, "segment ES"},
	{0, 1, i286_op27, 0, "daa"},
	{0, 1, i286_op28, 0, "sub :e,:r"},
	{OF_WORD, 1, i286_op29, 0, "sub :e,:r"},
	{0, 1, i286_op2A, 0, "sub :r,:e"},
	{OF_WORD, 1, i286_op2B, 0, "sub :r,:e"},
	{0, 1, i286_op2C, 0, "sub al,:1"},
	{0, 1, i286_op2D, 0, "sub ax,:2"},
	{OF_PREFIX, 1, i286_op2E, 0, "segment CS"},
	{0, 1, i286_op2F, 0, "das"},
	{0, 1, i286_op30, 0, "xor :e,:r"},
	{OF_WORD, 1, i286_op31, 0, "xor :e,:r"},
	{0, 1, i286_op32, 0, "xor :r,:e"},
	{OF_WORD, 1, i286_op33, 0, "xor :r,:e"},
	{0, 1, i286_op34, 0, "xor al,:1"},
	{0, 1, i286_op35, 0, "xor ax,:2"},
	{OF_PREFIX, 1, i286_op36, 0, "segment SS"},
	{0, 1, i286_op37, 0, "aaa"},
	{0, 1, i286_op38, 0, "cmp :e,:r"},
	{OF_WORD, 1, i286_op39, 0, "cmp :e,:r"},
	{0, 1, i286_op3A, 0, "cmp :r,:e"},
	{OF_WORD, 1, i286_op3B, 0, "cmp :r,:e"},
	{0, 1, i286_op3C, 0, "cmp al,:1"},
	{0, 1, i286_op3D, 0, "cmp ax,:2"},
	{OF_PREFIX, 1, i286_op3E, 0, "segment DS"},
	{0, 1, i286_op3F, 0, "aas"},
	{0, 1, i286_op40, 0, "inc ax"},
	{0, 1, i286_op41, 0, "inc cx"},
	{0, 1, i286_op42, 0, "inc dx"},
	{0, 1, i286_op43, 0, "inc bx"},
	{0, 1, i286_op44, 0, "inc sp"},
	{0, 1, i286_op45, 0, "inc bp"},
	{0, 1, i286_op46, 0, "inc si"},
	{0, 1, i286_op47, 0, "inc di"},
	{0, 1, i286_op48, 0, "dec ax"},
	{0, 1, i286_op49, 0, "dec cx"},
	{0, 1, i286_op4A, 0, "dec dx"},
	{0, 1, i286_op4B, 0, "dec bx"},
	{0, 1, i286_op4C, 0, "dec sp"},
	{0, 1, i286_op4D, 0, "dec bp"},
	{0, 1, i286_op4E, 0, "dec si"},
	{0, 1, i286_op4F, 0, "dec di"},
	{0, 1, i286_op50, 0, "push ax"},
	{0, 1, i286_op51, 0, "push cx"},
	{0, 1, i286_op52, 0, "push dx"},
	{0, 1, i286_op53, 0, "push bx"},
	{0, 1, i286_op54, 0, "push sp"},
	{0, 1, i286_op55, 0, "push bp"},
	{0, 1, i286_op56, 0, "push si"},
	{0, 1, i286_op57, 0, "push di"},
	{0, 1, i286_op58, 0, "pop ax"},
	{0, 1, i286_op59, 0, "pop cx"},
	{0, 1, i286_op5A, 0, "pop dx"},
	{0, 1, i286_op5B, 0, "pop bx"},
	{0, 1, i286_op5C, 0, "pop sp"},
	{0, 1, i286_op5D, 0, "pop bp"},
	{0, 1, i286_op5E, 0, "pop si"},
	{0, 1, i286_op5F, 0, "pop di"},
	{0, 1, i286_op60, 0, "pusha"},
	{0, 1, i286_op61, 0, "popa"},
	{OF_WORD, 1, i286_op62, 0, "bound :r,:e"},
	{OF_WORD, 1, i286_op63, 0, "arpl :e,:r"},
	{OF_PREFIX, 1, i286_op64, 0, "repnc"},		// repeat following cmps/scas cx times or until cf=1
	{OF_PREFIX, 1, i286_op65, 0, "repc"},		// repeat following cmps/scas cx times or until cf=0
	{OF_PREFIX, 1, i286_op66, 0, ""},
	{OF_PREFIX, 1, i286_op67, 0, ""},
	{0, 1, i286_op68, 0, "push :2"},
	{OF_WORD, 1, i286_op69, 0, "imul :r,:e,:2"},
	{0, 1, i286_op6A, 0, "push :1"},
	{OF_WORD, 1, i286_op6B, 0, "imul :r,:e,:1"},
	{OF_SKIPABLE, 1, i286_op6C, 0, ":Linsb [es::di]"},
	{OF_SKIPABLE, 1, i286_op6D, 0, ":Linsw [es::di]"},
	{OF_SKIPABLE, 1, i286_op6E, 0, ":Loutsb [:D::si]"},
	{OF_SKIPABLE, 1, i286_op6F, 0, ":Loutsw [:D::si]"},
	{0, 1, i286_op70, 0, "jo :3"},
	{0, 1, i286_op71, 0, "jno :3"},
	{0, 1, i286_op72, 0, "jc :3"},		// jb, jnae
	{0, 1, i286_op73, 0, "jnc :3"},		// jnb, jae
	{0, 1, i286_op74, 0, "jz :3"},
	{0, 1, i286_op75, 0, "jnz :3"},
	{0, 1, i286_op76, 0, "jbe :3"},		// jna
	{0, 1, i286_op77, 0, "jnbe :3"},	// ja
	{0, 1, i286_op78, 0, "js :3"},
	{0, 1, i286_op79, 0, "jns :3"},
	{0, 1, i286_op7A, 0, "jpe :3"},
	{0, 1, i286_op7B, 0, "jpo :3"},
	{0, 1, i286_op7C, 0, "jl :3"},
	{0, 1, i286_op7D, 0, "jnl :3"},
	{0, 1, i286_op7E, 0, "jle :3"},		// jng
	{0, 1, i286_op7F, 0, "jnle :3"},	// jg
	{0, 1, i286_op80, 0, ":A :e,:1"},	// :A = mod:N (add,or,adc,sbb,and,sub,xor,cmp)
	{OF_WORD, 1, i286_op81, 0, ":A :e,:2"},
	{0, 1, i286_op82, 0, ":A :e,:1"},
	{OF_WORD, 1, i286_op83, 0, ":A :e,:1"},
	{0, 1, i286_op84, 0, "test :e,:r"},
	{OF_WORD, 1, i286_op85, 0, "test :e,:r"},
	{0, 1, i286_op86, 0, "xchg :e,:r"},
	{OF_WORD, 1, i286_op87, 0, "xchg :e,:r"},
	{0, 1, i286_op88, 0, "mov :e,:r"},
	{OF_WORD, 1, i286_op89, 0, "mov :e,:r"},
	{0, 1, i286_op8A, 0, "mov :r,:e"},
	{OF_WORD, 1, i286_op8B, 0, "mov :r,:e"},
	{OF_WORD, 1, i286_op8C, 0, "mov :e,:s"},	// :s segment register from mod:N
	{OF_WORD, 1, i286_op8D, 0, "lea :r,:e"},
	{OF_WORD, 1, i286_op8E, 0, "mov :s,:e"},
	{OF_WORD, 1, i286_op8F, 0, "push :e"},
	{0, 1, i286_op90, 0, "nop"},
	{0, 1, i286_op91, 0, "xchg ax,cx"},
	{0, 1, i286_op92, 0, "xchg ax,dx"},
	{0, 1, i286_op93, 0, "xchg ax,bx"},
	{0, 1, i286_op94, 0, "xchg ax,sp"},
	{0, 1, i286_op95, 0, "xchg ax,bp"},
	{0, 1, i286_op96, 0, "xchg ax,si"},
	{0, 1, i286_op97, 0, "xchg ax,di"},
	{0, 1, i286_op98, 0, "cbw"},
	{0, 1, i286_op99, 0, "cwd"},
	{OF_SKIPABLE, 1, i286_op9A, 0, "callf :p"},
	{0, 1, i286_op9B, 0, "wait"},
	{0, 1, i286_op9C, 0, "pushf"},
	{0, 1, i286_op9D, 0, "popf"},
	{0, 1, i286_op9E, 0, "sahf"},
	{0, 1, i286_op9F, 0, "lahf"},
	{0, 1, i286_opA0, 0, "mov al,[:D:::2]"},
	{0, 1, i286_opA1, 0, "mov ax,[:D:::2]"},
	{0, 1, i286_opA2, 0, "mov [:D:::2],al"},
	{0, 1, i286_opA3, 0, "mov [:D:::2],ax"},
	{OF_SKIPABLE, 1, i286_opA4, 0, ":Lmovsb [:D::si],[es::di]"},
	{OF_SKIPABLE, 1, i286_opA5, 0, ":Lmovsw [:D::si],[es::di]"},
	{OF_SKIPABLE, 1, i286_opA6, 0, ":Lcmpsb [:D::si],[es::di]"},
	{OF_SKIPABLE, 1, i286_opA7, 0, ":Lcmpsw [:D::si],[es::di]"},
	{0, 1, i286_opA8, 0, "test al,:1"},
	{0, 1, i286_opA9, 0, "test ax,:2"},
	{OF_SKIPABLE, 1, i286_opAA, 0, ":Lstosb [es::di],al"},		// TODO (all rep-opcodes): not skipable w/o rep
	{OF_SKIPABLE, 1, i286_opAB, 0, ":Lstosw [es::di],ax"},
	{0, 1, i286_opAC, 0, "lodsb al,[:D::si]"},
	{0, 1, i286_opAD, 0, "lodsw ax,[:D::si]"},
	{OF_SKIPABLE, 1, i286_opAE, 0, ":Lscasb al,[es::di]"},
	{OF_SKIPABLE, 1, i286_opAF, 0, ":Lscasw ax,[es::di]"},
	{0, 1, i286_opB0, 0, "mov al,:1"},
	{0, 1, i286_opB1, 0, "mov cl,:1"},
	{0, 1, i286_opB2, 0, "mov dl,:1"},
	{0, 1, i286_opB3, 0, "mov bl,:1"},
	{0, 1, i286_opB4, 0, "mov ah,:1"},
	{0, 1, i286_opB5, 0, "mov ch,:1"},
	{0, 1, i286_opB6, 0, "mov dh,:1"},
	{0, 1, i286_opB7, 0, "mov bh,:1"},
	{0, 1, i286_opB8, 0, "mov ax,:2"},
	{0, 1, i286_opB9, 0, "mov cx,:2"},
	{0, 1, i286_opBA, 0, "mov dx,:2"},
	{0, 1, i286_opBB, 0, "mov bx,:2"},
	{0, 1, i286_opBC, 0, "mov sp,:2"},
	{0, 1, i286_opBD, 0, "mov bp,:2"},
	{0, 1, i286_opBE, 0, "mov si,:2"},
	{0, 1, i286_opBF, 0, "mov di,:2"},
	{0, 1, i286_opC0, 0, ":R :e,:1"},	// :R rotate group (rol,ror,rcl,rcr,sal,shr,*rot6,sar)
	{OF_WORD, 1, i286_opC1, 0, ":R :e,:1"},
	{0, 1, i286_opC2, 0, "ret :2"},
	{0, 1, i286_opC3, 0, "ret"},
	{OF_WORD, 1, i286_opC4, 0, "les :r,:e"},
	{OF_WORD, 1, i286_opC5, 0, "lds :r,:e"},
	{0, 1, i286_opC6, 0, "mov :e,:1"},
	{OF_WORD, 1, i286_opC7, 0, "mov :e,:2"},
	{0, 1, i286_opC8, 0, "enter :2,:1"},
	{0, 1, i286_opC9, 0, "leave"},
	{0, 1, i286_opCA, 0, "retf :2"},
	{0, 1, i286_opCB, 0, "retf"},
	{OF_SKIPABLE, 1, i286_opCC, 0, "int 3"},
	{OF_SKIPABLE, 1, i286_opCD, 0, "int :1"},
	{OF_SKIPABLE, 1, i286_opCE, 0, "into"},
	{0, 1, i286_opCF, 0, "iret"},
	{0, 1, i286_opD0, 0, ":R :e,1"},
	{OF_WORD, 1, i286_opD1, 0, ":R :e,1"},
	{0, 1, i286_opD2, 0, ":R :e,cl"},
	{OF_WORD, 1, i286_opD3, 0, ":R :e,cl"},
	{0, 1, i286_opD4, 0, "aam :1"},
	{0, 1, i286_opD5, 0, "aad :1"},
	{0, 1, i286_opD6, 0, "? salc"},
	{0, 1, i286_opD7, 0, "xlatb al,[:D::bx+al]"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_fpu, 0, "* x87 :z"},
	{0, 1, i286_opE0, 0, "loopnz :3"},
	{0, 1, i286_opE1, 0, "loopz :3"},
	{0, 1, i286_opE2, 0, "loop :3"},
	{0, 1, i286_opE3, 0, "jcxz :3"},
	{0, 1, i286_opE4, 0, "in al,:1"},
	{0, 1, i286_opE5, 0, "in ax,:1"},
	{0, 1, i286_opE6, 0, "out :1,al"},
	{0, 1, i286_opE7, 0, "out :1,ax"},
	{OF_SKIPABLE, 1, i286_opE8, 0, "call :n"},		// :n = near, 2byte offset
	{0, 1, i286_opE9, 0, "jmp :n"},			// jmp near
	{0, 1, i286_opEA, 0, "jmpf :p"},		// jmp far
	{0, 1, i286_opEB, 0, "jmp :3"},			// jmp short
	{0, 1, i286_opEC, 0, "in al,dx"},
	{0, 1, i286_opED, 0, "in ax,dx"},
	{0, 1, i286_opEE, 0, "out dx,al"},
	{0, 1, i286_opEF, 0, "out dx,ax"},
	{OF_PREFIX, 1, i286_opF0, 0, "lock"},
	{0, 1, i286_opF1, 0, "undef"},
	{OF_PREFIX, 1, i286_opF2, 0, "repnz"},
	{OF_PREFIX, 1, i286_opF3, 0, "repz/rep"},
	{0, 1, i286_opF4, 0, "hlt"},
	{0, 1, i286_opF5, 0, "cmc"},
	{0, 1, i286_opF6, 0, ":X :e"},		// test,test,not,neg,mul,imul,div,idiv
	{OF_WORD, 1, i286_opF7, 0, ":Y :e"},
	{0, 1, i286_opF8, 0, "clc"},
	{0, 1, i286_opF9, 0, "stc"},
	{0, 1, i286_opFA, 0, "cli"},
	{0, 1, i286_opFB, 0, "sti"},
	{0, 1, i286_opFC, 0, "cld"},
	{0, 1, i286_opFD, 0, "std"},
	{0, 1, i286_opFE, 0, ":E :e"},		// inc,dec,...
	{OF_WORD|OF_SKIPABLE, 1, i286_opFF, 0, ":F :e"},	// incw,decw,not,neg,call,callf,jmp,jmpf,push,???
};
