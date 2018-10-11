#include "1801vm1.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void pdp_wr(CPU* cpu, unsigned short adr, unsigned short val) {
	cpu->mwr(adr++, val & 0xff, cpu->data);
	cpu->mwr(adr, (val >> 8) & 0xff, cpu->data);
	cpu->t += 8;
}

unsigned short pdp_rd(CPU* cpu, unsigned short adr) {
	unsigned short res = cpu->mrd(adr++, 0, cpu->data) & 0xff;
	res |= (cpu->mrd(adr, 0, cpu->data) << 8);
	cpu->t += 8;
	return res;
}

// reset
// R7 = (0xffce + Ncpu * 16) & 0xff00;	??? 0x8000
// F = 0xe0
void pdp11_reset(CPU* cpu) {
	cpu->preg[7] = pdp_rd(cpu, 0xffce) & 0xff00;
	cpu->pc = cpu->preg[7];
	cpu->pflag = 0xe0;
	cpu->inten = 0xff;
}

void pdp_push(CPU* cpu, unsigned short val) {
	cpu->preg[6] -= 2;
	pdp_wr(cpu, cpu->preg[6], val);
}

unsigned short pdp_pop(CPU* cpu) {
	unsigned short res = pdp_rd(cpu, cpu->preg[6]);
	cpu->preg[6] += 2;
	return res;
}

void pdp_trap(CPU* cpu, unsigned short adr) {
	pdp_push(cpu, cpu->pflag);
	pdp_push(cpu, cpu->preg[7]);
	cpu->preg[7] = pdp_rd(cpu, adr);
	cpu->pflag = pdp_rd(cpu, adr+2);
}

int pdp11_int(CPU* cpu) {
	if ((cpu->intrq & PDP_INT_IRQ1) && !(cpu->pflag & (PDP_F10 | PDP_F11))) {
		cpu->intrq &= ~PDP_INT_IRQ1;
		pdp_wr(cpu, 0xffbc, cpu->preg[7]);
		pdp_wr(cpu, 0xffbe, cpu->pflag);
		pdp_trap(cpu, 0xe002);
	} else if ((cpu->intrq & PDP_INT_IRQ2) && !(cpu->pflag & (PDP_F10 | PDP_F7))) {
		cpu->intrq &= ~PDP_INT_IRQ2;
		pdp_trap(cpu, 0x40);			// #40 = 100(8)
	} else if ((cpu->intrq & PDP_INT_IRQ3) && !(cpu->pflag & (PDP_F10 | PDP_F7))) {
		cpu->intrq &= ~PDP_INT_IRQ3;
		pdp_trap(cpu, 0x00b8);			// #b8 = 270(8)
	} else if (cpu->intrq & PDP_INT_VIRQ) {
		cpu->intrq &= ~PDP_INT_VIRQ;
		pdp_trap(cpu, cpu->intvec);
	} else {
		return 0;
	}
	return 10;
}

// addressation

int pdp_adr(CPU* cpu, int type, int b) {
	int res = -1;
	switch (type & 0x38) {
		case 0x00: res = -1;			// Rn (no addr)
			break;
		case 0x08: res = cpu->preg[type & 7];	// @Rn
			break;
		case 0x10: res = cpu->preg[type & 7];	// (Rn)+
			cpu->preg[type & 7] += b ? 1 : 2;
			break;
		case 0x18: res = cpu->preg[type & 7];	// @(Rn)+
			if ((type & 7) == 7) b = 0;
			cpu->preg[type & 7] += 2;
			res = pdp_rd(cpu, res & 0xffff);
			break;
		case 0x20:				// -(Rn)
			cpu->preg[type & 7] -= b ? 1 : 2;
			res = cpu->preg[type & 7];
			break;
		case 0x28:				// @-(Rn)
			if ((type & 7) == 7) b = 0;
			cpu->preg[type & 7] -= 2;
			res = cpu->preg[type & 7];
			res = pdp_rd(cpu, res & 0xffff);
			break;
		case 0x30:				// E(Rn)
			res = pdp_rd(cpu, cpu->preg[7]);
			cpu->preg[7] += 2;
			res += cpu->preg[type & 7];
			break;
		case 0x38:				// @E(Rn)
			res = pdp_rd(cpu, cpu->preg[7]);
			cpu->preg[7] += 2;
			res += cpu->preg[type & 7];
			res = pdp_rd(cpu, res & 0xffff);
			break;
	}
	return res;
}

unsigned short pdp_src(CPU* cpu, int type, int b) {
	unsigned short res = 0xffff;
	switch(type & 0x38) {
		case 0x00: res = cpu->preg[type & 7];			// Rn
			break;
		case 0x08:
			cpu->mptr = cpu->preg[type & 7];		// @Rn
			res = pdp_rd(cpu, cpu->mptr);
			break;
		case 0x10: cpu->mptr = cpu->preg[type & 7];		// (Rn)+
			res = pdp_rd(cpu, cpu->mptr);
			cpu->preg[type & 7] += b ? 1 : 2;
			break;
		case 0x18:						// @(Rn)+
			cpu->mptr = cpu->preg[type & 7];
			cpu->mptr = pdp_rd(cpu, cpu->mptr);
			cpu->preg[type & 7] += 2;
			res = pdp_rd(cpu, cpu->mptr);
			break;
		case 0x20: cpu->preg[type & 7] -= b ? 1 : 2;		// -(Rn)
			cpu->mptr = cpu->preg[type & 7];
			res = pdp_rd(cpu, cpu->mptr);
			break;
		case 0x28: cpu->preg[type & 7] -= 2;			// @-(Rn)
			cpu->mptr = pdp_rd(cpu, cpu->preg[type & 7]);
			res = pdp_rd(cpu, cpu->mptr);
			break;
		case 0x30: cpu->mptr = pdp_rd(cpu, cpu->preg[7]);	// N(Rn)
			cpu->preg[7] += 2;
			cpu->mptr += cpu->preg[type & 7];
			res = pdp_rd(cpu, cpu->mptr);
			break;
		case 0x38: cpu->mptr = pdp_rd(cpu, cpu->preg[7]);		// @N(Rn)
			cpu->preg[7] += 2;
			cpu->mptr += cpu->preg[type & 7];
			cpu->mptr = pdp_rd(cpu, cpu->mptr);
			res = pdp_rd(cpu, cpu->mptr);
			break;
	}
	if (b) res &= 0xff;
	return res;
}

void pdp_dst(CPU* cpu, unsigned short wrd, int type, int b) {
	switch (type & 0x38) {
		case 0x00: if (b) {
				cpu->preg[type & 7] &= 0xff00;
				cpu->preg[type & 7] |= (wrd & 0xff);
			} else {
				cpu->preg[type & 7] = wrd;
			}
			break;
		case 0x08: cpu->mptr = cpu->preg[type & 7];
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x10: cpu->mptr = cpu->preg[type & 7];
			cpu->preg[type & 7] += b ? 1 : 2;
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x18:
			cpu->mptr = cpu->preg[type & 7];
			cpu->tmp = cpu->mrd(cpu->mptr++, 0, cpu->data);
			cpu->hptr = cpu->mrd(cpu->mptr, 0, cpu->data);
			cpu->lptr = cpu->tmp;
			cpu->preg[type & 7] += 2;
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x20: cpu->preg[type & 7] -= b ? 1 : 2;
			cpu->mptr = cpu->preg[type & 7];
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x28: cpu->preg[type & 7] -= 2;
			cpu->mptr = cpu->preg[type & 7];
			cpu->tmp = cpu->mrd(cpu->mptr++, 0, cpu->data);
			cpu->hptr = cpu->mrd(cpu->mptr, 0, cpu->data);
			cpu->lptr = cpu->tmp;
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x30: cpu->mptr = pdp_rd(cpu, cpu->preg[7]);
			cpu->preg[7] += 2;
			cpu->mptr += cpu->preg[type & 7];
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x38: cpu->mptr = pdp_rd(cpu, cpu->preg[7]);
			cpu->preg[7] += 2;
			cpu->mptr += cpu->preg[type & 7];
			cpu->tmp = cpu->mrd(cpu->mptr++, 0, cpu->data);
			cpu->hptr = cpu->mrd(cpu->mptr, 0, cpu->data);
			cpu->lptr = cpu->tmp;
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
	}
}

// commands

static unsigned short twsrc;
static unsigned short twdst;
static int twres;

void pdp_undef(CPU* cpu) {
	printf("undef command %.4X : %.4X\n", cpu->preg[7] - 2, cpu->com);
	assert(0);
	pdp_trap(cpu, 16);
}

// 000x

// 0000:halt
void pdp_halt(CPU* cpu) {
	cpu->mcir = 3;	// 011
	cpu->vsel = 11;	// 1011
	pdp_trap(cpu, 0160002);
}

// 0001:wait
void pdp_wait(CPU* cpu) {
	cpu->mcir = 0;
	cpu->wait = 1;
}

// 0002:rti
void pdp_rti(CPU* cpu) {
	cpu->preg[7] = pdp_rd(cpu, cpu->preg[6]);
	cpu->preg[6] += 2;
	cpu->pflag = pdp_rd(cpu, cpu->preg[6]);
	cpu->preg[6] += 2;
	cpu->pflag &= 0xff;
	if (cpu->pflag & PDP_FT) {
		cpu->mcir = 5;
		cpu->vsel = 3;
		pdp_trap(cpu, 014);
	}
}

// 0003:bpt
void pdp_bpt(CPU* cpu) {
	cpu->mcir = 5;
	cpu->vsel = 3;
	pdp_trap(cpu, 014);		// 14(8) = 12(10)
}

// 0004:iot
void pdp_iot(CPU* cpu) {
	cpu->mcir = 5;
	cpu->vsel = 1;
	pdp_trap(cpu, 020);		// 20(8) = 16(10)
}

// 0005:reset
void pdp_res(CPU* cpu) {
}

// 0006:rtt
void pdp_rtt(CPU* cpu) {
	cpu->preg[7] = pdp_rd(cpu, cpu->preg[6]);
	cpu->preg[6] += 2;
	cpu->pflag = pdp_rd(cpu, cpu->preg[6]);
	cpu->preg[6] += 2;
	cpu->pflag &= 0xff;
}

// 0007..000A : start

void pdp_start(CPU* cpu) {
	cpu->preg[7] = pdp_rd(cpu, 0177674);
	cpu->pflag = pdp_rd(cpu, 0177676);
	cpu->mptr = pdp_rd(cpu, 0177716);
	cpu->mptr &= ~8;
	pdp_wr(cpu, 0177716, cpu->mptr);
}

// 000B..000F : step

void pdp_step(CPU* cpu) {
	cpu->preg[7] = pdp_rd(cpu, 0177674);
	cpu->pflag = pdp_rd(cpu, 0177676);
	cpu->mptr = pdp_rd(cpu, 0177716);
	cpu->mptr &= ~8;
	pdp_wr(cpu, 0177716, cpu->mptr);
}

static cbcpu pdp_000n_tab[16] = {
	pdp_halt, pdp_wait, pdp_rti, pdp_bpt,
	pdp_iot, pdp_res, pdp_rtt, pdp_undef,
	pdp_start, pdp_start, pdp_start, pdp_start,
	pdp_step, pdp_step, pdp_step, pdp_step
};

void pdp_000x(CPU* cpu) {
	pdp_000n_tab[cpu->com & 0x0f](cpu);
}

//0000 0000 01dd dddd	jmp		r7 = [dd]
void pdp_jmp(CPU* cpu) {
	twres = pdp_adr(cpu, cpu->com, 0);
	if (twres < 0) {
		cpu->mcir = 5;
		cpu->vsel = 4;
		pdp_trap(cpu, 4);
	} else {
		cpu->preg[7] = twres & 0xffff;	// jmp take address, not value
	}
}

//0000 0000 1000 0rrr	rts		r7=reg:pop reg
//0000 0000 1000 1xxx	?
void pdp_008x(CPU* cpu) {
	if (cpu->com & 8) {
		pdp_undef(cpu);
	} else {
		cpu->mcir = 3;
		cpu->preg[7] = cpu->preg[cpu->com & 7];
		cpu->preg[cpu->com & 7] = pdp_rd(cpu, cpu->preg[6]);
		cpu->preg[6] += 2;
	}
}

// 0000 0000 1001 xxxx	?

// 0000 0000 1010 nzvc	clear flags
void pdp_cl(CPU* cpu) {
	cpu->pflag &= ~(cpu->com & 0x0f);
}

// 0000 0000 1011 nzvc	set flags
void pdp_se(CPU* cpu) {
	cpu->pflag |= (cpu->com & 0x0f);
}

// 0000 0000 11dd dddd	swab		swap hi/lo bytes in [dd]
void pdp_swab(CPU* cpu) {
	cpu->mcir = 7;
	if (!(cpu->com & 070)) {		// amode 0
		twsrc = cpu->preg[cpu->com & 7];
		twdst = ((twsrc << 8) & 0xff00) | ((twsrc >> 8) & 0xff);
		cpu->preg[cpu->com & 7] = twdst;
	} else {
		twsrc = pdp_src(cpu, cpu->com, 0);	// mptr = addr
		twdst = ((twsrc << 8) & 0xff00) | ((twsrc >> 8) & 0xff);
		pdp_wr(cpu, cpu->mptr, twdst);		// write it back (not pdp_dst)
	}
	cpu->pflag &= ~(PDP_FC | PDP_FV);	// reset c,v
}

static cbcpu pdp_00nx_tab[16] = {
	pdp_000x, pdp_undef, pdp_undef, pdp_undef,
	pdp_jmp, pdp_jmp, pdp_jmp, pdp_jmp,
	pdp_008x, pdp_undef, pdp_cl, pdp_se,
	pdp_swab, pdp_swab, pdp_swab, pdp_swab
};

void pdp_00xx(CPU* cpu) {
	pdp_00nx_tab[(cpu->com >> 4) & 0x0f](cpu);
}

// conditional jumps

// br
void pdp_01xx(CPU* cpu) {
	twsrc = (cpu->com << 1) & 0x1fe;
	if (twsrc & 0x100)
		twsrc |= 0xff00;
	cpu->preg[7] += twsrc;
}

// bne
void pdp_02xx(CPU* cpu) {
	if (~cpu->pflag & PDP_FZ)
		pdp_01xx(cpu);
}

// beq
void pdp_03xx(CPU* cpu) {
	if (cpu->pflag & PDP_FZ)
		pdp_01xx(cpu);
}

// bge
void pdp_04xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (!twres)
		pdp_01xx(cpu);
}

// blt
void pdp_05xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (twres)
		pdp_01xx(cpu);
}

// bgt
void pdp_06xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (cpu->pflag & PDP_FZ) twres |= 1;
	if (!twres)
		pdp_01xx(cpu);
}

// ble
void pdp_07xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (cpu->pflag & PDP_FZ) twres |= 1;
	if (twres)
		pdp_01xx(cpu);
}

// 0000 100r rrdd dddd	jsr		push reg:reg=r7:r7=[dd]
// !!! if addressation method = 0, exception (4)
void pdp_jsr(CPU* cpu) {
	twres = pdp_adr(cpu, cpu->com, 0);
	if (twres < 0) {		// addr type 0: exception
		cpu->mcir = 5;
		cpu->vsel = 4;
		pdp_trap(cpu, 4);
	} else {
		cpu->mcir = 4;
		twsrc = (cpu->com >> 6) & 7;
		cpu->preg[6] -= 2;
		pdp_wr(cpu, cpu->preg[6], cpu->preg[twsrc]);
		cpu->preg[twsrc] = cpu->preg[7];
		cpu->preg[7] = twres & 0xffff;
	}
}

//0000 1010 00dd dddd	clr		dd = 0
void pdp_clr(CPU* cpu) {
	cpu->mcir = (cpu->com & 0x38) ? 7 : 5;
	pdp_dst(cpu, 0, cpu->com, 0);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV);
	cpu->pflag |= PDP_FZ;
}

//0000 1010 01dd dddd	com		invert all bits (cpl)
void pdp_com(CPU* cpu) {
	cpu->mcir = (cpu->com & 0x38) ? 7 : 5;
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc ^= 0xffff;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
	cpu->pflag &= ~(PDP_FN | PDP_FC | PDP_FZ | PDP_FV);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	cpu->pflag |= PDP_FC;
}

//0000 1010 10dd dddd	inc
void pdp_inc(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc++;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
	cpu->pflag &= ~(PDP_FN | PDP_FC | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (twsrc == 0x8000) cpu->pflag |= PDP_FC;
}

//0000 1010 11dd dddd	dec
void pdp_dec(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc--;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
	cpu->pflag &= ~(PDP_FN | PDP_FC | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (twsrc == 0x7fff) cpu->pflag |= PDP_FC;
}

static cbcpu pdp_0axx_tab[4] = {pdp_clr, pdp_com, pdp_inc, pdp_dec};

void pdp_0axx(CPU* cpu) {
	pdp_0axx_tab[(cpu->com >> 6) & 3](cpu);
}

//0000 1011 00dd dddd	neg		[dd] = -[dd]
//0000 1011 01dd dddd	adc		[dd] = [dd] + C
//0000 1011 10dd dddd	sbc		[dd] = [dd] - C
//0000 1011 11dd dddd	tst
void pdp_neg(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc = 0 - twsrc;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FC | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FC;
	if (twsrc == 0x8000) cpu->pflag |= PDP_FV;
}

void pdp_adc(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	if (cpu->pflag & PDP_FC)
		twsrc++;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (cpu->pflag & PDP_FC) {
		if (twsrc) cpu->pflag &= ~PDP_FC;
		if (twsrc == 0x8000) cpu->pflag |= PDP_FV;
	}
}

void pdp_sbc(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	if (cpu->pflag & PDP_FC)
		twsrc--;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (cpu->pflag & PDP_FC) {
		if (twsrc == 0xffff) cpu->pflag |= PDP_FC; else cpu->pflag &= ~PDP_FC;
		if (twsrc == 0x7fff) cpu->pflag |= PDP_FV;
	}
}

void pdp_tst(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FC | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
}

static cbcpu pdp_0bxx_tab[4] = {pdp_neg, pdp_adc, pdp_sbc, pdp_tst};

void pdp_0bxx(CPU* cpu) {
	pdp_0bxx_tab[(cpu->com >> 6) & 3](cpu);
}

//0000 1100 00dd dddd	ror
void pdp_ror(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 1) cpu->pflag |= PDP_FC;
	twsrc >>= 1;
	if (cpu->tmpw) twsrc |= 0x8000;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

//0000 1100 01dd dddd	rol
void pdp_rol(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x8000) cpu->pflag |= PDP_FC;
	twsrc <<= 1;
	if (cpu->tmpw) twsrc |= 1;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

//0000 1100 10dd dddd	asr
void pdp_asr(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 1) cpu->pflag |= PDP_FC;
	twsrc >>= 1;
	if (twsrc & 0x4000) {
		twsrc |= 0x8000;
		cpu->pflag |= PDP_FN;
	}
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

//0000 1100 11dd dddd	arl
void pdp_asl(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x8000) cpu->pflag |= PDP_FC;
	twsrc <<= 1;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

static cbcpu pdp_0cxx_tab[4] = {pdp_ror, pdp_rol, pdp_asr, pdp_asl};

void pdp_0cxx(CPU* cpu) {
	pdp_0cxx_tab[(cpu->com >> 6) & 3](cpu);
}

//0000 1101 00nn nnnn	mark nn
//0000 1101 01ss ssss	mfpi		push [ss]
//0000 1101 10dd dddd	mtpi		pop [dd]
//0000 1101 11dd dddd	sxt		dst = N ? -1 : 0

void pdp_mark(CPU* cpu) {
	cpu->preg[6] = cpu->preg[7] + (cpu->com & 0x3f) * 2;
	cpu->preg[7] = cpu->preg[5];
	cpu->preg[5] = pdp_pop(cpu);
}

void pdp_mfpi(CPU* cpu) {
	pdp_undef(cpu);
}

void pdp_mtpi(CPU* cpu) {
	pdp_undef(cpu);
}

void pdp_sxt(CPU* cpu) {
	twdst = (cpu->pflag & PDP_FN) ? 0xffff : 0x0000;
	cpu->pflag &= ~(PDP_FZ | PDP_FV);
	if (cpu->pflag & PDP_FN) cpu->pflag |= PDP_FZ;
	pdp_dst(cpu, twdst, cpu->com, 0);
}

static cbcpu pdp_0dxx_tab[4] = {pdp_mark, pdp_mfpi, pdp_mtpi, pdp_sxt};

void pdp_0dxx(CPU* cpu) {
	pdp_0dxx_tab[(cpu->com >> 6) & 3](cpu);
}

static cbcpu pdp_tab_0nxx[16] = {
	pdp_00xx, pdp_01xx, pdp_02xx, pdp_03xx,
	pdp_04xx, pdp_05xx, pdp_06xx, pdp_07xx,
	pdp_jsr, pdp_jsr, pdp_0axx, pdp_0bxx,
	pdp_0cxx, pdp_0dxx, pdp_undef, pdp_undef
};

// 00xxxx
void pdp_0xxx(CPU* cpu) {
	pdp_tab_0nxx[(cpu->com >> 8) & 0x0f](cpu);
}

// bpl
void pdp_80xx(CPU* cpu) {
	if (~cpu->pflag & PDP_FN)
		pdp_01xx(cpu);
}

// bmi
void pdp_81xx(CPU* cpu) {
	if (cpu->pflag & PDP_FN)
		pdp_01xx(cpu);
}

// bhi
void pdp_82xx(CPU* cpu) {
	if (!((cpu->pflag & PDP_FC) || (cpu->pflag & PDP_FZ)))
		pdp_01xx(cpu);
}

// blos
void pdp_83xx(CPU* cpu) {
	if ((cpu->pflag & PDP_FC) || (cpu->pflag & PDP_FZ))
		pdp_01xx(cpu);
}

// bvc
void pdp_84xx(CPU* cpu) {
	if (~cpu->pflag & PDP_FV)
		pdp_01xx(cpu);
}

// bvs
void pdp_85xx(CPU* cpu) {
	if (cpu->pflag & PDP_FV)
		pdp_01xx(cpu);
}

// bcc (bhis)
void pdp_86xx(CPU* cpu) {
	if (~cpu->pflag & PDP_FC)
		pdp_01xx(cpu);
}

// bcs (blo)
void pdp_87xx(CPU* cpu) {
	if (cpu->pflag & PDP_FC)
		pdp_01xx(cpu);
}

// 10xxxx

// emt
void pdp_88xx(CPU* cpu) {
	pdp_trap(cpu, 24);	// 30(8) = 24(10)
}

// trap
void pdp_89xx(CPU* cpu) {
	pdp_trap(cpu, 28);	// 34(8) = 28(10)
}

//1000 1010 00dd dddd	clrb
//1000 1010 01dd dddd	comb
//1000 1010 10dd dddd	incb
//1000 1010 11dd dddd	decb

void pdp_clrb(CPU* cpu) {
	if (cpu->com & 0x38) {
		twsrc = pdp_src(cpu, cpu->com, 1);
		cpu->mwr(cpu->mptr, 0x00, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] &= ~0xff;
	}
	cpu->pflag &= ~(PDP_FN | PDP_FC | PDP_FV);
	cpu->pflag |= PDP_FZ;
}

void pdp_comb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc ^= 0xff;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (!(twsrc & 0xff)) cpu->pflag |= PDP_FZ;
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;		// high byte doesn't changed
	}
}

void pdp_incb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twdst = (twsrc + 1) & 0xff;
	twsrc &= 0xff00;
	twsrc |= twdst;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc == 0x80) cpu->pflag |= PDP_FV;	// 7f->80
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twdst & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

void pdp_decb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twdst = (twsrc - 1) & 0xff;
	twsrc &= 0xff00;
	twsrc |= twdst;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc == 0x7f) cpu->pflag |= PDP_FV;	// 80->7f
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twdst & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

static cbcpu pdp_8axx_tab[4] = {pdp_clrb, pdp_comb, pdp_incb, pdp_decb};

void pdp_8axx(CPU* cpu) {
	pdp_8axx_tab[(cpu->com >> 6) & 3](cpu);
}

//1000 1011 00dd dddd	negb
//1000 1011 01dd dddd	adcb
//1000 1011 10dd dddd	sbcb
//1000 1011 11dd dddd	tstb

void pdp_negb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	twdst = (0 - twsrc) & 0xff;
	twsrc &= 0xff00;
	twsrc |= twdst;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (twsrc == 0x7f) cpu->pflag |= PDP_FV;	// 80->7f
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

void pdp_adcb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	twdst = twsrc;
	if (cpu->pflag & PDP_FC) twdst++;
	twsrc &= 0xff00;
	twsrc |= (twdst & 0xff);
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (cpu->pflag & PDP_FC) {
		if (twsrc) cpu->pflag &= ~PDP_FC;
		if (twsrc == 0x80) cpu->pflag |= PDP_FV;
	}
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

void pdp_sbcb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	twdst = twsrc;
	if (cpu->pflag & PDP_FC) twdst--;
	twsrc &= 0xff00;
	twsrc |= (twdst & 0xff);
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (cpu->pflag & PDP_FC) {
		if (twsrc != 0xff) cpu->pflag &= ~PDP_FC;
		if (twsrc == 0x7f) cpu->pflag |= PDP_FV;
	}
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

void pdp_tstb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0) & 0xff;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
}

static cbcpu pdp_8bxx_tab[4] = {pdp_negb, pdp_adcb, pdp_sbcb, pdp_tstb};

void pdp_8bxx(CPU* cpu) {
	pdp_8bxx_tab[(cpu->com >> 6) & 3](cpu);
}

//1000 1100 00dd dddd	rorb
//1000 1100 01dd dddd	rolb
//1000 1100 10dd dddd	asrb
//1000 1100 11dd dddd	aslb

void pdp_rorb(CPU* cpu) {
	twdst = pdp_src(cpu, cpu->com, 1);
	twsrc = twdst & 0xff;
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 1) cpu->pflag |= PDP_FC;
	twsrc >>= 1;
	if (cpu->tmpw) twsrc |= 0x80;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	twsrc = (twdst & 0xff00) | (twsrc & 0xff);
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] &= 0xff00;
		cpu->preg[cpu->com & 7] |= (twsrc & 0xff);
	}
}

void pdp_rolb(CPU* cpu) {
	twdst = pdp_src(cpu, cpu->com, 1);
	twsrc = twdst & 0xff;
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FC;
	twsrc <<= 1;
	twsrc &= 0xff;
	if (cpu->tmpw) twsrc |= 1;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	twsrc = (twdst & 0xff00) | (twsrc & 0xff);
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] &= 0xff00;
		cpu->preg[cpu->com & 7] |= (twsrc & 0xff);
	}
}

void pdp_asrb(CPU* cpu) {
	twdst = pdp_src(cpu, cpu->com, 1);
	twsrc = twdst & 0xff;
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 1) cpu->pflag |= PDP_FC;
	twsrc >>= 1;
	if (twsrc & 0x40) twsrc |= 0x80;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	twsrc = (twdst & 0xff00) | (twsrc & 0xff);
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] &= 0xff00;
		cpu->preg[cpu->com & 7] |= (twsrc & 0xff);
	}
}

void pdp_aslb(CPU* cpu) {
	twdst = pdp_src(cpu, cpu->com, 1);
	twsrc = twdst & 0xff;
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FC;
	twsrc <<= 1;
	twsrc &= 0xff;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	twsrc = (twdst & 0xff00) | (twsrc & 0xff);
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] &= 0xff00;
		cpu->preg[cpu->com & 7] |= (twsrc & 0xff);
	}
}

static cbcpu pdp_8cxx_tab[4] = {pdp_rorb, pdp_rolb, pdp_asrb, pdp_aslb};

void pdp_8cxx(CPU* cpu) {
	pdp_8cxx_tab[(cpu->com >> 6) & 3](cpu);
}

// 8dxx

void pdp_mtps(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0) & ~0x10;
	twsrc &= 0xff;
	cpu->pflag &= 0xff10;
	cpu->pflag |= twsrc;
}

void pdp_mfps(CPU* cpu) {
	pdp_dst(cpu, cpu->pflag, cpu->com, 1);
}

static cbcpu pdp_8dxx_tab[4] = {pdp_mtps, pdp_undef, pdp_undef, pdp_mfps};

void pdp_8dxx(CPU* cpu) {
	pdp_8dxx_tab[(cpu->com >> 6) & 3](cpu);
}

static cbcpu pdp_8nxx_tab[16] = {
	pdp_80xx, pdp_81xx, pdp_82xx, pdp_83xx,
	pdp_84xx, pdp_85xx, pdp_86xx, pdp_87xx,
	pdp_88xx, pdp_89xx, pdp_8axx, pdp_8bxx,
	pdp_8cxx, pdp_8dxx, pdp_undef, pdp_undef
};

void pdp_8xxx(CPU* cpu) {
	pdp_8nxx_tab[(cpu->com >> 8) & 0x0f](cpu);
}

// 07xxxx

void pdp_mul(CPU* cpu) {
	pdp_undef(cpu);
}

void pdp_div(CPU* cpu) {
	pdp_undef(cpu);
}

void pdp_ash(CPU* cpu) {
	pdp_undef(cpu);
}

void pdp_ashc(CPU* cpu) {
	pdp_undef(cpu);
}

void pdp_xor(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc ^= cpu->preg[(cpu->com >> 6) & 7];
	cpu->pflag &= ~(PDP_FV | PDP_FZ | PDP_FN);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twsrc);
	} else {
		cpu->preg[cpu->com & 7] = twsrc;
	}
}

// 07ruu
void pdp_sob(CPU* cpu) {
	twsrc = (cpu->com >> 6) & 7;
	cpu->preg[twsrc]--;
	if (cpu->preg[twsrc]) {
		cpu->preg[7] -= (cpu->com & 0x3f) * 2;
	}
}

static cbcpu pdp_7nxx_tab[8] = {
	pdp_mul, pdp_div, pdp_ash, pdp_ashc,
	pdp_xor, pdp_undef, pdp_undef, pdp_sob
};

void pdp_7xxx(CPU* cpu) {
	pdp_7nxx_tab[(cpu->com >> 9) & 0x07](cpu);
}

// B1SSDD:mov
// DD = SS
// FZ: src = 0
// FN: src < 0
// FC: not changed
// FV: 0
void pdp_mov(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);
	pdp_dst(cpu, twsrc, cpu->com, 0);
	cpu->pflag &= (PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
}

// movb works as RMW (read-modify-write)
// movb (R1)+, @R3 : R1+=1 !!!
void pdp_movb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 1) & 0xff;
	if (cpu->com & 0x38) {
		twdst = pdp_src(cpu, cpu->com, 0);		// unused: old value
		cpu->mwr(cpu->mptr, twsrc & 0xff, cpu->data);	// write low byte only
	} else {
		if (twsrc & 0x80)
			twsrc |= 0xff00;
		else
			twsrc &= 0xff;
		cpu->preg[cpu->com & 7] = twsrc;
	}
	cpu->pflag &= (PDP_FN | PDP_FV | PDP_FZ);
	if (!(twsrc & 0xff)) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
}

// B2SSDD:cmp
// DD - SS >> /dev/null
// Z: dst = src
// N: dst < src
// C: high byte carry
// V: overflow
void pdp_cmp(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);
	twdst = pdp_src(cpu, cpu->com, 0);
	twres = twsrc + (~twdst + 1);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twres == 0) cpu->pflag |= PDP_FZ;
	if (twres & 0x8000) cpu->pflag |= PDP_FN;
	if (twres & 0x10000) cpu->pflag |= PDP_FC;
	if (!((twsrc ^ twdst) & 0x8000) && (((twres ^ twdst) | (twres ^ twsrc)) & 0x8000)) cpu->pflag |= PDP_FV;
}

void pdp_cmpb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0) & 0xff;
	twdst = pdp_src(cpu, cpu->com, 0) & 0xff;
	twres = twsrc - twdst;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (!(twres & 0xff)) cpu->pflag |= PDP_FZ;
	if (twres & 0x80) cpu->pflag |= PDP_FN;
	if (twres & 0x100) cpu->pflag |= PDP_FC;
	if (!((twsrc ^ twdst) & 0x80) && (((twres ^ twdst) | (twres ^ twsrc)) & 0x80)) cpu->pflag |= PDP_FV;
}

// B3SSDD:bit (and)
// check (DD & SS). don't write it back
// Z: res = 0
// N: res.b15
// C: not affected
// V: 0
void pdp_bit(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);
	twdst = pdp_src(cpu, cpu->com, 0);
	twdst &= twsrc;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twdst == 0) cpu->pflag |= PDP_FZ;
	if (twdst & 0x8000) cpu->pflag |= PDP_FN;
}

void pdp_bitb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0) & 0xff;
	twdst = pdp_src(cpu, cpu->com, 0) & 0xff;
	twdst &= twsrc;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (!twdst) cpu->pflag |= PDP_FZ;
	if (twdst & 0x80) cpu->pflag |= PDP_FN;
}

// B4SSDD:bic (and not)
// DD = DD & ~SS;
// Z: res = 0
// N: res.b15
// C: not affected
// V: 0
void pdp_bic(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);
	twdst = pdp_src(cpu, cpu->com, 0);
	twdst &= ~twsrc;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twdst == 0) cpu->pflag |= PDP_FZ;
	if (twdst & 0x8000) cpu->pflag |= PDP_FN;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twdst);
	} else {
		cpu->preg[cpu->com & 7] = twdst;
	}
}

void pdp_bicb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0) & 0xff;
	twdst = pdp_src(cpu, cpu->com, 0) & 0xff;
	twdst &= ~twsrc;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (!(twdst & 0xff)) cpu->pflag |= PDP_FZ;
	if (twdst & 0x80) cpu->pflag |= PDP_FN;
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twdst & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twdst;
	}
}

// B5SSDD:bis (or)
// DD = DD | SS
// Z: res = 0
// N: res.b15
// C: not affected
// V: 0
void pdp_bis(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);
	twdst = pdp_src(cpu, cpu->com, 0);
	twdst |= twsrc;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twdst == 0) cpu->pflag |= PDP_FZ;
	if (twdst & 0x8000) cpu->pflag |= PDP_FN;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twdst);
	} else {
		cpu->preg[cpu->com & 7] = twdst;
	}
}

void pdp_bisb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0) & 0xff;
	twdst = pdp_src(cpu, cpu->com, 0) & 0xff;
	twdst |= twsrc;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (!(twdst & 0xff)) cpu->pflag |= PDP_FZ;
	if (twdst & 0x80) cpu->pflag |= PDP_FN;
	if (cpu->com & 070) {
		cpu->mwr(cpu->mptr, twdst & 0xff, cpu->data);
	} else {
		cpu->preg[cpu->com & 7] = twdst;
	}
}

// 06SSDD:add
// DD = DD + SS
// Z: result = 0
// N: result < 0
// C: high byte carry
// V: overflow (if both op is same sign, but res is opposite sign)
void pdp_add(CPU* cpu) {				// add r1,r7
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);		// = r1
	twdst = pdp_src(cpu, cpu->com, 0);		// = r7 (next command)
	twres = twsrc + twdst;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twres == 0) cpu->pflag |= PDP_FZ;
	if (twres & 0x8000) cpu->pflag |= PDP_FN;
	if ((twsrc & 0xff) + (twdst & 0xff) > 0xff) cpu->pflag |= PDP_FC;
	if (!((twsrc ^ twdst) & 0x8000) && (((twres ^ twdst) | (twres ^ twsrc)) & 0x8000)) cpu->pflag |= PDP_FV;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twres & 0xffff);
	} else {
		cpu->preg[cpu->com & 7] = twres & 0xffff;
	}
}

// 16SSDD:sub
// DD = DD - SS
// Z: result = 0
// N: result < 0
// C: high byte carry
// V: overflow
void pdp_sub(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);
	twdst = pdp_src(cpu, cpu->com, 0);
	twres = twdst - twsrc;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twres == 0) cpu->pflag |= PDP_FZ;
	if (twres & 0x8000) cpu->pflag |= PDP_FN;
	if ((twdst & 0xff) < (twsrc & 0xff)) cpu->pflag |= PDP_FC;
	if (!((twsrc ^ twdst) & 0x8000) && (((twres ^ twdst) | (twres ^ twsrc)) & 0x8000)) cpu->pflag |= PDP_FV;
	if (cpu->com & 070) {
		pdp_wr(cpu, cpu->mptr, twres & 0xffff);
	} else {
		cpu->preg[cpu->com & 7] = twres & 0xffff;
	}
}

// tables

// xNNN xxxx xxxx xxxx
static cbcpu pdp_tab_a[16] = {
	pdp_0xxx, pdp_mov, pdp_cmp, pdp_bit, pdp_bic, pdp_bis, pdp_add, pdp_7xxx,
	pdp_8xxx, pdp_movb, pdp_cmpb, pdp_bitb, pdp_bicb, pdp_bisb, pdp_sub, pdp_undef
};

int pdp11_exec(CPU* cpu) {
#ifdef ISDEBUG
//	printf("%.4X : %.4X\n", cpu->preg[7], pdp_rd(cpu, cpu->preg[7]));
#endif
	cpu->preg[7] = cpu->pc;
	if (cpu->wait) {
		if (cpu->mcir)
			cpu->wait = 0;
		return 8;
	}
	cpu->t = 0;
	if (cpu->inten & cpu->intrq) {
		cpu->t += pdp11_int(cpu);
	}
	if (cpu->t == 0) {
		cpu->lcom = cpu->mrd(cpu->preg[7]++, 1, cpu->data);
		cpu->hcom = cpu->mrd(cpu->preg[7]++, 1, cpu->data);
		cpu->t += 8;
		// exec 1st tab #Nxxx
		pdp_tab_a[(cpu->com >> 12) & 0x0f](cpu);
		if (cpu->sta) {			// stack goes trough 0x400 : trap 4
			cpu->sta = 0;
			pdp_trap(cpu, 4);
		}
	}
	cpu->pc = cpu->preg[7];
	return cpu->t;
}

// registers

static xRegDsc pdp11RegTab[] = {
	{PDP11_REG0, "R0", 0},
	{PDP11_REG1, "R1", 0},
	{PDP11_REG2, "R2", 0},
	{PDP11_REG3, "R3", 0},
	{PDP11_REG4, "R4", 0},
	{PDP11_REG5, "R5", 0},
	{PDP11_REG6, "SP", 0},
	{PDP11_REG7, "PC", 0},
	{PDP11_REGF, "PSW", 0},
	{REG_NONE, "", 0}
};

void pdp11_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	while (pdp11RegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = pdp11RegTab[idx].id;
		strncpy(bunch->regs[idx].name, pdp11RegTab[idx].name, 7);
		bunch->regs[idx].byte = pdp11RegTab[idx].byte;
		switch(pdp11RegTab[idx].id) {
			case PDP11_REG0: bunch->regs[idx].value = cpu->preg[0]; break;
			case PDP11_REG1: bunch->regs[idx].value = cpu->preg[1]; break;
			case PDP11_REG2: bunch->regs[idx].value = cpu->preg[2]; break;
			case PDP11_REG3: bunch->regs[idx].value = cpu->preg[3]; break;
			case PDP11_REG4: bunch->regs[idx].value = cpu->preg[4]; break;
			case PDP11_REG5: bunch->regs[idx].value = cpu->preg[5]; break;
			case PDP11_REG6: bunch->regs[idx].value = cpu->preg[6]; break;
			case PDP11_REG7: bunch->regs[idx].value = cpu->preg[7]; break;
			case PDP11_REGF: bunch->regs[idx].value = cpu->pflag; break;
		}
		idx++;
	}
	bunch->regs[idx].id = REG_NONE;
	memcpy(bunch->flags, "----NZVC", 8);
	cpu->f = cpu->pflag & 0xff;
}

void pdp11_set_regs(CPU* cpu, xRegBunch bunch) {
	int idx = 0;
	while (bunch.regs[idx].id != REG_NONE) {
		switch (bunch.regs[idx].id) {
			case PDP11_REG0: cpu->preg[0] = bunch.regs[idx].value; break;
			case PDP11_REG1: cpu->preg[1] = bunch.regs[idx].value; break;
			case PDP11_REG2: cpu->preg[2] = bunch.regs[idx].value; break;
			case PDP11_REG3: cpu->preg[3] = bunch.regs[idx].value; break;
			case PDP11_REG4: cpu->preg[4] = bunch.regs[idx].value; break;
			case PDP11_REG5: cpu->preg[5] = bunch.regs[idx].value; break;
			case PDP11_REG6: cpu->preg[6] = bunch.regs[idx].value; break;
			case PDP11_REG7: cpu->preg[7] = bunch.regs[idx].value; cpu->pc = cpu->preg[7]; break;
			case PDP11_REGF: cpu->pflag = bunch.regs[idx].value; break;
		}
		idx++;
	}
}

// disasm

typedef struct {
	unsigned short mask;
	unsigned short code;
	const char* mnem;
} xPdpDasm;

static xPdpDasm pdp11_dasm_tab[] = {
	{0xffff, 0x0000, "halt"},
	{0xffff, 0x0001, "wait"},
	{0xffff, 0x0002, "rti"},
	{0xffff, 0x0003, "bpt"},
	{0xffff, 0x0004, "iot"},
	{0xffff, 0x0005, "reset"},
	{0xffff, 0x0006, "rtt"},
	{0xffc0, 0x0040, "jmp :d"},	// :d lower 6 bits dst(src)
	{0xfff8, 0x0080, "rts r:0"},	// :0 bits 0,1,2 number
	{0xfff0, 0x00a0, "cf :f"},	// :f lower 4 bits flags
	{0xfff0, 0x00b0, "sf :f"},
	{0xffc0, 0x00c0, "swab :d"},
	{0xff00, 0x0100, "br :e"},	// :e relative jump
	{0xff00, 0x0200, "bne :e"},
	{0xff00, 0x0300, "beq :e"},
	{0xff00, 0x0400, "bge :e"},
	{0xff00, 0x0500, "blt :e"},
	{0xff00, 0x0600, "bgt :e"},
	{0xff00, 0x0700, "ble :e"},
	{0xfe00, 0x0800, "jsr r:6, :d"},	// :6 bits 6,7,8 number
	{0xffc0, 0x0a00, "clr :d"},
	{0xffc0, 0x0a40, "com :d"},
	{0xffc0, 0x0a80, "inc :d"},
	{0xffc0, 0x0ac0, "dec :d"},
	{0xffc0, 0x0b00, "neg :d"},
	{0xffc0, 0x0b40, "adc :d"},
	{0xffc0, 0x0b80, "sbc :d"},
	{0xffc0, 0x0bc0, "tst :d"},
	{0xffc0, 0x0c00, "ror :d"},
	{0xffc0, 0x0c40, "rol :d"},
	{0xffc0, 0x0c80, "asr :d"},
	{0xffc0, 0x0cc0, "arl :d"},
	{0xffc0, 0x0d40, "mfpi :d"},
	{0xffc0, 0x0d80, "mtpi :d"},
	{0xffc0, 0x0dc0, "sxt :d"},
	{0xf000, 0x1000, "mov :s, :d"},	// :s = :d from bits 6-11
	{0xf000, 0x2000, "cmp :s, :d"},
	{0xf000, 0x3000, "bit :s, :d"},
	{0xf000, 0x4000, "bic :s, :d"},
	{0xf000, 0x5000, "bis :s, :d"},
	{0xf000, 0x6000, "add :s, :d"},
//	{0xfe00, 0x7000, "mul r:6, :d"},
//	{0xfe00, 0x7200, "div r:6, :d"},
	{0xfe00, 0x7800, "xor r:6, :d"},
	{0xfe00, 0x7e00, "sob r:6, :j"},		// :j = lower 6 bits, back relative adr
	{0xff00, 0x8000, "bpl :e"},
	{0xff00, 0x8100, "bmi :e"},
	{0xff00, 0x8200, "bhi :e"},
	{0xff00, 0x8300, "blos :e"},
	{0xff00, 0x8400, "bvc :e"},
	{0xff00, 0x8500, "bvs :e"},
	{0xff00, 0x8600, "bss :e"},
	{0xff00, 0x8700, "bcs :e"},
	{0xff00, 0x8800, "emt :x"},	// x: lower 8 bits, number
	{0xff00, 0x8900, "trap"},
	{0xffc0, 0x8a00, "clrb :d"},
	{0xffc0, 0x8a40, "comb :d"},
	{0xffc0, 0x8a80, "incb :d"},
	{0xffc0, 0x8ac0, "decb :d"},
	{0xffc0, 0x8b00, "negb :d"},
	{0xffc0, 0x8b40, "adcb :d"},
	{0xffc0, 0x8b80, "sbcb :d"},
	{0xffc0, 0x8bc0, "tstb :d"},
	{0xffc0, 0x8c00, "rorb :d"},
	{0xffc0, 0x8c40, "rolb :d"},
	{0xffc0, 0x8c80, "asrb :d"},
	{0xffc0, 0x8cc0, "aslb :d"},
	{0xffc0, 0x8d00, "mtps :d"},
	{0xffc0, 0x8dc0, "mfps :d"},
	{0xf000, 0x9000, "movb :s, :d"},
	{0xf000, 0xa000, "cmpb :s, :d"},
	{0xf000, 0xb000, "bitb :s, :d"},
	{0xf000, 0xc000, "bicb :s, :d"},
	{0xf000, 0xd000, "bisb :s, :d"},
	{0xf000, 0xe000, "sub :s, :d"},
	{0x0000, 0x0000, "undef"}
};

static char mnbuf[128];
static char num7[8] = "01234567";
static char numF[16] = "0123456789ABCDEF";

char* put_addressation(char* dst, unsigned short type) {
	switch ((type >> 3) & 7) {
		case 1: *(dst++) = '@';
		case 0: *(dst++) = 'R';
			*(dst++) = num7[type & 7];
			break;
		case 3: *(dst++) = '@';
		case 2: if ((type & 7) == 7) {
				*(dst++) = ':';		// :2 will be replaced with next word
				*(dst++) = '2';
			} else {
				*(dst++) = '(';
				*(dst++) = 'R';
				*(dst++) = num7[type & 7];
				*(dst++) = ')';
				*(dst++) = '+';
			}
			break;
		case 5: *(dst++) = '@';
		case 4: *(dst++) = '-';
			*(dst++) = '(';
			*(dst++) = 'R';
			*(dst++) = num7[type & 7];
			*(dst++) = ')';
			break;
		case 7: *(dst++) = '@';
		case 6: *(dst++) = ':';			// :2 will be replaced with next word
			*(dst++) = '2';
			*(dst++) = '(';
			*(dst++) = 'R';
			*(dst++) = num7[type & 7];
			*(dst++) = ')';
			break;
	}
	return dst;
}

xMnem pdp11_mnem(CPU* cpu, unsigned short adr, cbdmr mrd, void* dat) {
	xMnem res;
	res.len = 2;
	int idx = 0;
	unsigned short com = mrd(adr++, dat);
	com |= (mrd(adr++, dat) << 8);
	adr += 2;
	while ((pdp11_dasm_tab[idx].mask != 0) && ((com & pdp11_dasm_tab[idx].mask) != pdp11_dasm_tab[idx].code))
		idx++;
	const char* src = pdp11_dasm_tab[idx].mnem;
	char* dst = mnbuf;
	while (*src != 0) {
		if (*src == ':') {
			src++;
			switch (*src) {
				case '0':
					*dst = num7[com & 7];
					dst++;
					break;
				case '6':
					*dst = num7[(com >> 6) & 7];
					dst++;
					break;
				case 'x':
					*(dst++) = numF[(com & 0xf0) >> 4];
					*(dst++) = numF[com & 0x0f];
					break;
				case 'e':
					*(dst++) = '$';
					*(dst++) = (com & 0x0080) ? '-' : '+';
					*(dst++) = numF[(com & 0x78) >> 3];
					*(dst++) = numF[(com & 0x07) << 1];
					break;
				case 'j':
					*(dst++) = '$';
					*(dst++) = '-';
					*(dst++) = numF[(com & 0x38) >> 3];
					*(dst++) = numF[(com & 7) << 1];
					break;
				case 'f':
					if (com & PDP_FC) *(dst++) = 'C';
					if (com & PDP_FZ) *(dst++) = 'Z';
					if (com & PDP_FV) *(dst++) = 'V';
					if (com & PDP_FN) *(dst++) = 'N';
					if (!(com & 15)) *(dst++) = '0';
					break;
				case 's':
					dst = put_addressation(dst, com >> 6);
					break;
				case 'd':
					dst = put_addressation(dst, com);
					break;
			}
			src++;
		} else {
			*dst = *src;
			src++;
			dst++;
		}
	}
	*dst = 0;
	res.mnem = mnbuf;
	res.flag = 0;
	res.cond = 0;
	res.met = 0;
	res.mem = 0;
	res.mop = 0;
	res.oadr = 0;
	return res;
}

// asm

xAsmScan pdp11_asm(const char* mnm, char* buf) {
	xAsmScan res;
	res.match = 0;

	return res;
}
