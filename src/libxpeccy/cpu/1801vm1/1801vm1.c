#include "1801vm1.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// reset
// R7 = (0xffce + Ncpu * 16) & 0xff00;
// F = 0xe0
void pdp11_reset(CPU* cpu) {
	cpu->hptr = cpu->mrd(0xffcf, 0, cpu->data);
	cpu->lptr = 0;
	cpu->preg[7] = cpu->mptr;
	cpu->pflag = 0xe0;
	cpu->sta = 0;			// reset "stack overfill" flag
}

void pdp_wr(CPU* cpu, unsigned short adr, unsigned short val) {
	cpu->mwr(adr++, val & 0xff, cpu->data);
	cpu->mwr(adr, (val >> 8) & 0xff, cpu->data);
}

unsigned short pdp_rd(CPU* cpu, unsigned short adr) {
	unsigned short res = cpu->mrd(adr++, 0, cpu->data) & 0xff;
	res |= (cpu->mrd(adr, 0, cpu->data) << 8);
	return res;
}

void pdp_push(CPU* cpu, unsigned short val) {
	cpu->tmpw = cpu->preg[6];
	cpu->mwr(--cpu->preg[6], (val >> 8) & 0xff, cpu->data);
	cpu->mwr(--cpu->preg[6], val & 0xff, cpu->data);
	if ((cpu->preg[6] < 0x400) && (cpu->tmpw > 0x3ff)) {
		cpu->sta = 1;
	}
}

unsigned short pdp_pop(CPU* cpu) {
	cpu->tmpw = cpu->preg[6];
	unsigned short res = cpu->mrd(cpu->preg[6]++, 0, cpu->data);
	res |= cpu->mrd(cpu->preg[6]++, 0, cpu->data) << 8;
	if ((cpu->preg[6] > 0x3ff) && (cpu->tmpw < 0x400)) {
		cpu->sta = 1;
	}
	return res;
}

void pdp_trap(CPU* cpu, unsigned short adr) {
	pdp_push(cpu, cpu->pflag);
	pdp_push(cpu, cpu->preg[7]);
	cpu->lptr = cpu->mrd(adr++, 0, cpu->data);
	cpu->hptr = cpu->mrd(adr++, 0, cpu->data);
	cpu->preg[7] = cpu->mptr;
	cpu->lptr = cpu->mrd(adr++, 0, cpu->data);
	cpu->hptr = cpu->mrd(adr, 0, cpu->data);
	cpu->pflag = cpu->mptr;
}

int pdp11_int(CPU* cpu) {
	if ((cpu->intrq & PDP_INT_IRQ1) && !(cpu->pflag & (PDP_F10 | PDP_F11))) {
		cpu->intrq &= ~PDP_INT_IRQ1;
		pdp_wr(cpu, 0xffbc, cpu->preg[7]);
		pdp_wr(cpu, 0xffbe, cpu->pflag);
		cpu->preg[7] = pdp_rd(cpu, 0xe002);	// #e002 = 160002(8)
		cpu->pflag = pdp_rd(cpu, 0xe004);
	} else if ((cpu->intrq & PDP_INT_IRQ2) && !(cpu->pflag & (PDP_F10 | PDP_F7))) {
		cpu->intrq &= PDP_INT_IRQ2;
		pdp_trap(cpu, 0x40);			// #40 = 100(8)
	} else if ((cpu->intrq & PDP_INT_IRQ3) && !(cpu->pflag & (PDP_F10 | PDP_F7))) {
		cpu->intrq &= PDP_INT_IRQ3;
		pdp_trap(cpu, 0x00b8);			// #b0 = 270(8)
	} else if (cpu->intrq & PDP_INT_VIRQ) {
		cpu->intrq &= PDP_INT_VIRQ;
		// read external vector
		// trap on this vector
	} else {
		return 0;
	}
	return 10;
}

// addressation

unsigned short pdp_src(CPU* cpu, int type, int b) {
	unsigned short res;
	switch(type & 0x30) {
		case 0x00: res = cpu->preg[type & 3];			// Rn
			break;
		case 0x10: res = cpu->preg[type & 3];			// (Rn)+
			cpu->lptr = cpu->mrd(res++, 0, cpu->data);
			cpu->hptr = cpu->mrd(res, 0, cpu->data);
			cpu->preg[type & 3] += b ? 1 : 2;
			res = cpu->mptr;
			break;
		case 0x20: cpu->preg[type & 7] -= b ? 1 : 2;		// -(Rn)
			res = cpu->preg[type & 7];
			cpu->lptr = cpu->mrd(res++, 0, cpu->data);
			cpu->hptr = cpu->mrd(res, 0, cpu->data);
			res = cpu->mptr;
			break;
		default: cpu->lptr = cpu->mrd(cpu->preg[7]++, 0, cpu->data);	// N(Rn)
			cpu->hptr = cpu->mrd(cpu->preg[7]++, 0, cpu->data);
			res = cpu->preg[type & 7] + cpu->mptr;
			cpu->lptr = cpu->mrd(res++, 0, cpu->data);
			cpu->hptr = cpu->mrd(res, 0, cpu->data);
			res = cpu->mptr;
			break;
	}
	if (type & 8) {
		cpu->lptr = cpu->mrd(res++, 0, cpu->data);
		cpu->hptr = cpu->mrd(res, 0, cpu->data);
		res = cpu->mptr;
	}
	if (b) res &= 0x00ff;
	return res;
}

void pdp_dst(CPU* cpu, unsigned short wrd, int type, int b) {
	switch (type & 0x38) {
		case 0x00: cpu->preg[type & 7] = wrd;
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
			cpu->preg[type & 7] += b ? 1 : 2;
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x20: cpu->preg[type & 7] -= b ? 1 : 2;
			cpu->mptr = cpu->preg[type & 7];
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x28: cpu->preg[type & 7] -= b ? 1 : 2;
			cpu->mptr = cpu->preg[type & 7];
			cpu->tmp = cpu->mrd(cpu->mptr++, 0, cpu->data);
			cpu->hptr = cpu->mrd(cpu->mptr, 0, cpu->data);
			cpu->lptr = cpu->tmp;
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x30: cpu->lptr = cpu->mrd(cpu->preg[7]++, 0, cpu->data);
			cpu->hptr = cpu->mrd(cpu->preg[7]++, 0, cpu->data);
			cpu->mptr += cpu->preg[type & 7];
			cpu->mwr(cpu->mptr++, wrd & 0xff, cpu->data);
			if (!b) cpu->mwr(cpu->mptr, (wrd >> 8) & 0xff, cpu->data);
			break;
		case 0x38:cpu->lptr = cpu->mrd(cpu->preg[7]++, 0, cpu->data);
			cpu->hptr = cpu->mrd(cpu->preg[7]++, 0, cpu->data);
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
static unsigned int twres;

void pdp_undef(CPU* cpu) {
	printf("undef command %.4X\n", cpu->com);
	assert(0);
	pdp_trap(cpu, 16);
}

// 000x

// 0000:halt
void pdp_halt(CPU* cpu) {
}

// 0001:wait
void pdp_wait(CPU* cpu) {

}

// 0002:rti
void pdp_rti(CPU* cpu) {
	cpu->preg[7] = pdp_pop(cpu);
	cpu->pflag = pdp_pop(cpu);
}

// 0003:bpt
void pdp_bpt(CPU* cpu) {
	pdp_trap(cpu, 12);		// 14(8) = 12(10)
}

// 0004:iot
void pdp_iot(CPU* cpu) {
	pdp_trap(cpu, 16);		// 20(8) = 16(10)
}

// 0005:reset
void pdp_res(CPU* cpu) {
}

// 0006:rtt
void pdp_rtt(CPU* cpu) {
	cpu->preg[7] = pdp_pop(cpu);
	cpu->pflag = pdp_pop(cpu) & 0xff;		// reset high byte
}

// 0007..000A : start

void pdp_start(CPU* cpu) {

}

// 000B..000F : step

void pdp_step(CPU* cpu) {

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
	cpu->preg[7] = pdp_src(cpu, cpu->com, 0);
}

//0000 0000 1000 0rrr	rts		r7=reg:pop reg
//0000 0000 1000 1xxx	?
void pdp_008x(CPU* cpu) {
	if (cpu->com & 8) {
		pdp_undef(cpu);
	} else {
		cpu->preg[7] = cpu->preg[cpu->com & 7];
		cpu->preg[cpu->com & 7] = pdp_pop(cpu);
	}
}

//0000 0000 1001 xxxx	?

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
	twsrc = pdp_src(cpu, cpu->com, 0);
	twdst = ((twsrc << 8) & 0xff00) | ((twsrc >> 8) & 0xff);
	pdp_dst(cpu, twdst, cpu->com, 0);
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
	cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bne
void pdp_02xx(CPU* cpu) {
	if (~cpu->pflag & PDP_FZ)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// beq
void pdp_03xx(CPU* cpu) {
	if (cpu->pflag & PDP_FZ)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bge
void pdp_04xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (!twres)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// blt
void pdp_05xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (twres)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bgt
void pdp_06xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (cpu->pflag & PDP_FZ) twres |= 1;
	if (!twres)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// ble
void pdp_07xx(CPU* cpu) {
	twres = (cpu->pflag & PDP_FN) ? 1 : 0;
	if (cpu->pflag & PDP_FV) twres ^= 1;
	if (cpu->pflag & PDP_FZ) twres |= 1;
	if (twres)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// 0000 100r rrdd dddd	jsr		push reg:reg=r7:r7=[dd]
// !!! if addressation method = 0, exception (4)
void pdp_jsr(CPU* cpu) {
	if ((cpu->com & 0x38) == 0) {
		pdp_trap(cpu, 4);
	} else {
		twsrc = (cpu->com >> 6) & 7;
		twdst = cpu->preg[twsrc];
		cpu->mwr(cpu->preg[6]--, (twdst >> 8) & 0xff, cpu->data);	// push Rr
		cpu->mwr(cpu->preg[6]--, twdst & 0xff, cpu->data);
		cpu->preg[twsrc] = cpu->preg[7];
		cpu->preg[7] = pdp_src(cpu, cpu->com, 0);
	}
}

//0000 1010 00dd dddd	clr		dd = 0
void pdp_clr(CPU* cpu) {
	pdp_dst(cpu, 0, cpu->com, 0);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV);
	cpu->pflag |= PDP_FZ;
}

//0000 1010 01dd dddd	com		invert all bits (cpl)
void pdp_com(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc = ~twsrc;
	pdp_dst(cpu, twsrc, cpu->com, 0);
	cpu->pflag &= ~(PDP_FN | PDP_FC | PDP_FZ | PDP_FV);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	cpu->pflag |= PDP_FC;
}

//0000 1010 10dd dddd	inc
void pdp_inc(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc++;
	pdp_dst(cpu, twsrc, cpu->com, 0);
	cpu->pflag &= ~(PDP_FN | PDP_FC | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
	if (twsrc == 0x8000) cpu->pflag |= PDP_FC;
}

//0000 1010 11dd dddd	dec
void pdp_dec(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	twsrc--;
	pdp_dst(cpu, twsrc, cpu->com, 0);
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
	pdp_dst(cpu, twsrc, cpu->com, 0);
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
	pdp_dst(cpu, twsrc, cpu->com, 0);
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
	pdp_dst(cpu, twsrc, cpu->com, 0);
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
	pdp_dst(cpu, twsrc, cpu->com, 0);
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
	pdp_dst(cpu, twsrc, cpu->com, 0);
}

//0000 1100 10dd dddd	asr
void pdp_asr(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 0);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 1) cpu->pflag |= PDP_FC;
	twsrc >>= 1;
	if (twsrc & 4000) {
		twsrc |= 0x8000;
		cpu->pflag |= PDP_FN;
	}
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	pdp_dst(cpu, twsrc, cpu->com, 0);
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
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bmi
void pdp_81xx(CPU* cpu) {
	if (cpu->pflag & PDP_FN)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bhi
void pdp_82xx(CPU* cpu) {
	if (!((cpu->pflag & PDP_FC) || (cpu->pflag & PDP_FZ)))
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// blos
void pdp_83xx(CPU* cpu) {
	if ((cpu->pflag & PDP_FC) || (cpu->pflag & PDP_FZ))
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bvc
void pdp_84xx(CPU* cpu) {
	if (~cpu->pflag & PDP_FV)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bvs
void pdp_85xx(CPU* cpu) {
	if (cpu->pflag & PDP_FV)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bcc (bhis)
void pdp_86xx(CPU* cpu) {
	if (~cpu->pflag & PDP_FC)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
}

// bcs (blo)
void pdp_87xx(CPU* cpu) {
	if (cpu->pflag & PDP_FC)
		cpu->preg[7] += 2 * (signed)cpu->lcom;
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
	pdp_dst(cpu, 0, cpu->com, 1);
	cpu->pflag &= ~(PDP_FN | PDP_FC | PDP_FV);
	cpu->pflag |= PDP_FZ;
}

void pdp_comb(CPU* cpu) {
	twsrc = ~pdp_src(cpu, cpu->com, 1);
	pdp_dst(cpu, twsrc, cpu->com, 1);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (!(twsrc & 0xff)) cpu->pflag |= PDP_FZ;
}

void pdp_incb(CPU* cpu) {
	twsrc = (pdp_src(cpu, cpu->com, 1) + 1) & 0xff;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc == 0x80) cpu->pflag |= PDP_FV;	// 7f->80
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

void pdp_decb(CPU* cpu) {
	twsrc = (pdp_src(cpu, cpu->com, 1) - 1) & 0xff;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc == 0x7f) cpu->pflag |= PDP_FV;	// 80->7f
	pdp_dst(cpu, twsrc, cpu->com, 1);
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
	twsrc = (~pdp_src(cpu, cpu->com, 1) + 1) & 0xff;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (twsrc == 0x7f) cpu->pflag |= PDP_FV;	// 80->7f
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

void pdp_adcb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	if (cpu->pflag & PDP_FC) twsrc++;
	twsrc &= 0xff;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (cpu->pflag & PDP_FC) {
		if (twsrc) cpu->pflag &= ~PDP_FC;
		if (twsrc == 0x80) cpu->pflag |= PDP_FV;
	}
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

void pdp_sbcb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	if (cpu->pflag & PDP_FC) twsrc--;
	twsrc &= 0xff;
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (cpu->pflag & PDP_FC) {
		if (twsrc != 0xff) cpu->pflag &= ~PDP_FC;
		if (twsrc == 0x7f) cpu->pflag |= PDP_FV;
	}
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

void pdp_tstb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
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
	twsrc = pdp_src(cpu, cpu->com, 1);
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 1) cpu->pflag |= PDP_FC;
	twsrc >>= 1;
	if (cpu->tmpw) twsrc |= 0x80;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

void pdp_rolb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FC;
	twsrc <<= 1;
	if (cpu->tmpw) twsrc |= 1;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

void pdp_asrb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 1) cpu->pflag |= PDP_FC;
	twsrc >>= 1;
	if (twsrc & 0x40) twsrc |= 0x80;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

void pdp_aslb(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1);
	cpu->tmpw = cpu->pflag & PDP_FC;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc & 0x80) cpu->pflag |= PDP_FC;
	twsrc <<= 1;
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x80) cpu->pflag |= PDP_FN;
	if (((cpu->pflag & PDP_FC) ? 1 : 0) ^ ((cpu->pflag & PDP_FN) ? 1 : 0))
		cpu->pflag |= PDP_FV;
	pdp_dst(cpu, twsrc, cpu->com, 1);
}

static cbcpu pdp_8cxx_tab[4] = {pdp_rorb, pdp_rolb, pdp_asrb, pdp_aslb};

void pdp_8cxx(CPU* cpu) {
	pdp_8cxx_tab[(cpu->com >> 6) & 3](cpu);
}

// 8dxx

void pdp_mtps(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com, 1) & ~0x10;
	cpu->pflag &= 0x20;
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
	pdp_dst(cpu, twsrc, cpu->com, 0);
}

// 07ruu
void pdp_sob(CPU* cpu) {
	twsrc = (cpu->com >> 6) & 7;
	cpu->preg[twsrc]--;
	if (cpu->preg[twsrc])
		cpu->preg[7] -= (cpu->com & 0x3f) * 2;
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
	twsrc = pdp_src(cpu, cpu->com >> 6, cpu->com & 0x8000);
	if ((cpu->com & 0x8000) && ((cpu->com & 0x38) == 0)) {		// byte -> reg
		if (twsrc & 0x80)
			twsrc |= 0xff00;
		cpu->preg[cpu->com & 7] = twsrc;
	} else {
		pdp_dst(cpu, twsrc, cpu->com, cpu->com & 0x8000);	// all other variants
	}
	cpu->pflag &= (PDP_FN | PDP_FV | PDP_FZ);
	if (twsrc == 0) cpu->pflag |= PDP_FZ;
	if (twsrc & 0x8000) cpu->pflag |= PDP_FN;
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
	twres = twdst - twsrc;
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twres == 0) cpu->pflag |= PDP_FZ;
	if (twres & 0x8000) cpu->pflag |= PDP_FN;
	if ((twdst & 0xff) < (twsrc & 0xff)) cpu->pflag |= PDP_FC;
	if (!((twsrc ^ twdst) & 0x8000) && (((twres ^ twdst) | (twres ^ twsrc)) & 0x8000)) cpu->pflag |= PDP_FV;
}

// B3SSDD:bit (and)
// DD = DD & SS
// Z: res = 0
// N: res.b15
// C: not affected
// V: 0
void pdp_bit(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, cpu->com & 0x8000);
	twdst = pdp_src(cpu, cpu->com, cpu->com & 0x8000);
	twdst &= twsrc;
	pdp_dst(cpu, twdst, cpu->com, cpu->com & 0x8000);
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twdst == 0) cpu->pflag |= PDP_FZ;
	if (twdst & 0x8000) cpu->pflag |= PDP_FN;
}

// B4SSDD:bic (and not)
// DD = DD & ~SS;
// Z: res = 0
// N: res.b15
// C: not affected
// V: 0
void pdp_bic(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, cpu->com & 0x8000);
	twdst = pdp_src(cpu, cpu->com, cpu->com & 0x8000);
	twdst &= ~twsrc;
	pdp_dst(cpu, twdst, cpu->com, cpu->com & 0x8000);
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twdst == 0) cpu->pflag |= PDP_FZ;
	if (twdst & 0x8000) cpu->pflag |= PDP_FN;
}

// B5SSDD:bis (or)
// DD = DD | SS
// Z: res = 0
// N: res.b15
// C: not affected
// V: 0
void pdp_bis(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, cpu->com & 0x8000);
	twdst = pdp_src(cpu, cpu->com, cpu->com & 0x8000);
	twdst |= twsrc;
	pdp_dst(cpu, twdst, cpu->com, cpu->com & 0x8000);
	cpu->pflag &= ~(PDP_FN | PDP_FV | PDP_FZ);
	if (twdst == 0) cpu->pflag |= PDP_FZ;
	if (twdst & 0x8000) cpu->pflag |= PDP_FN;
}

// 06SSDD:add
// DD = DD + SS
// Z: result = 0
// N: result < 0
// C: high byte carry
// V: overflow (if both op is same sign, but res is opposite sign)
void pdp_add(CPU* cpu) {
	twsrc = pdp_src(cpu, cpu->com >> 6, 0);
	twdst = pdp_src(cpu, cpu->com, 0);
	twres = twsrc + twdst;
	pdp_dst(cpu, twres & 0xffff, cpu->com, 0);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twres == 0) cpu->pflag |= PDP_FZ;
	if (twres & 0x8000) cpu->pflag |= PDP_FN;
	if ((twsrc & 0xff) + (twdst & 0xff) > 0xff) cpu->pflag |= PDP_FC;
	if (!((twsrc ^ twdst) & 0x8000) && (((twres ^ twdst) | (twres ^ twsrc)) & 0x8000)) cpu->pflag |= PDP_FV;
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
	pdp_dst(cpu, twres & 0xffff, cpu->com, 0);
	cpu->pflag &= ~(PDP_FC | PDP_FN | PDP_FV | PDP_FZ);
	if (twres == 0) cpu->pflag |= PDP_FZ;
	if (twres & 0x8000) cpu->pflag |= PDP_FN;
	if ((twdst & 0xff) < (twsrc & 0xff)) cpu->pflag |= PDP_FC;
	if (!((twsrc ^ twdst) & 0x8000) && (((twres ^ twdst) | (twres ^ twsrc)) & 0x8000)) cpu->pflag |= PDP_FV;
}

// tables

// xNNN xxxx xxxx xxxx
static cbcpu pdp_tab_a[16] = {
	pdp_0xxx, pdp_mov, pdp_cmp, pdp_bit, pdp_bic, pdp_bis, pdp_add, pdp_7xxx,
	pdp_8xxx, pdp_mov, pdp_cmp, pdp_bit, pdp_bic, pdp_bis, pdp_sub, pdp_undef
};

int pdp11_exec(CPU* cpu) {
	cpu->lcom = cpu->mrd(cpu->pc++, 1, cpu->data);
	cpu->hcom = cpu->mrd(cpu->pc++, 1, cpu->data);
	// exec 1st tab #Nxxx
	pdp_tab_a[(cpu->com >> 12) & 0x0f](cpu);
	if (cpu->sta) {			// stack goes trough 0x400 : trap 4
		cpu->sta = 0;
		pdp_trap(cpu, 4);
	}
	return 1;
}

// registers

static xRegDsc pdp11RegTab[] = {
	{PDP11_REG0, "R0", 0},
	{PDP11_REG1, "R1", 0},
	{PDP11_REG2, "R2", 0},
	{PDP11_REG3, "R3", 0},
	{PDP11_REG4, "R4", 0},
	{PDP11_REG5, "R5", 0},
	{PDP11_REG6, "R6", 0},
	{PDP11_REG7, "R7", 0},
	{PDP11_REGF, "F", 0},
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
	}
}

void pdp11_set_regs(CPU* cpu, xRegBunch bunch) {

}

// asm/disasm

xMnem pdp11_mnem(CPU* cpu, unsigned short adr, cbdmr crd, void* dat) {
	xMnem res;
	res.len = 1;

	return res;
}

xAsmScan pdp11_asm(const char* mnm, char* buf) {
	xAsmScan res;
	res.match = 0;

	return res;
}
