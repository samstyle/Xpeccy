#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "i80286.h"

// opcodes D8..DF to x87 (11011xxx)
// format: 11011xxx,mod,[disp,[disp]]

// word int: 2bytes signed	MF=11	short
// short int: 4bytes signed	MF=01	int
// long int: 8bytes signed
// packed bcd: 10bytes, 18 digits x 4bits, MSBit is sign

// short real: 4 bytes float	MF=00	float
// b31:sign
// b22-30:exponent (+7F)
// b0-21:significand

// long real: 8 bytes float	MF=10	double
// b63:sign
// b51-62:exponent (+3FF)
// b0-50:significand

// temp real: 10 bytes float
// b79:sign
// b64-78:exponent (+3FFF)
// b63:binary point = 1
// b0-62:significand

// value = (-1)^s * significand * 2^exponent
// ! significand in [1.0 - 2.0) aka normalized

enum {
	X87_REAL32 = 0,	// 32 bits real (float)		MF=00
	X87_INT32,	// 32 bits int			MF=01
	X87_REAL64,	// 64 bits real (double)	MF=10
	X87_INT16,	// 16 bits int			MF=11
	X87_INT64,	// 64 bits int
	X87_BCD,	// 80bit, 18 digits
	X87_REAL80	// 80bit internal format (long double)
};

// x87 status word flags
#define X87F_IE	1		// invalid op
#define X87F_DE (1<<1)		// denorm operand
#define X87F_ZE (1<<2)		// zero divide
#define X87F_OE (1<<3)		// overflow
#define X87F_UE (1<<4)		// underflow
#define X87F_PE (1<<5)		// precision
#define X87F_SF (1<<6)		// stack flag
#define X87F_ES (1<<7)		// error summary status
#define X87F_C0 (1<<8)		// condition code C0..C3
#define X87F_C1 (1<<9)
#define X87F_C2 (1<<10)
// bit 11-13 = x87top
#define X87F_C3 (1<<14)
// bit 15 = busy

extern void i286_get_ea(CPU*, int);

// if unmasked error occurs, ERROR signal activated. when 286 executed WAIT or ESCAPE, this signal will cause X86_INT_MF
void x87_error(CPU* cpu, int m) {
	m &= 0x3f;
	if (~cpu->x87cr & m) {		// if not masked (1 = masked)
		cpu->x87sr |= m;
		cpu->x87sr |= 0x80;
	}
}

long long x87_rd_cnt(CPU* cpu, int cnt) {
	int x = 0;
	int adr = cpu->ea.adr;
	long res = 0;
	while ((cnt > 0) && (x < (sizeof(long long) * 8))) {
		res |= (i286_mrd(cpu, cpu->ea.seg, 0, adr) << x);
		x += 8;
		adr++;
		cnt--;
	}
	return res;
}

double x87_rd_op(CPU* cpu, int t) {
	long double res = 0;
	unsigned long long tmp = 0;
	long double base;
	double bexp;
	switch (t) {
		case X87_REAL32:		// 32-bit real
			tmp = x87_rd_cnt(cpu, 4);
			base = tmp & (((long)1<<22) - 1);
			while (base > 2.0) {
				base /= 2.0;
			}
			bexp = ((tmp >> 22) & 0xff) - 0x7f;
			res = base * pow(2.0, bexp);
			if ((tmp >> 31) & 1) {
				res = -res;
			}
			break;
		case X87_INT32:		// 32-bit int
			tmp = x87_rd_cnt(cpu, 4);
			res = tmp & (((long long)1 << 31) - 1);
			if ((tmp >> 31) & 1) {
				res = -res;
			}
			break;
		case X87_REAL64:	// 64-bit real
			tmp = x87_rd_cnt(cpu, 8);
			base = tmp & (((long long)1<<51) - 1);
			while (base >= 2.0) {
				base /= 2.0;
			}
			bexp = ((tmp >> 51) & 0x7ff) - 0x3ff;
			res = base * pow(2.0, bexp);
			if ((tmp >> 63) & 1) {
				res = -res;
			}
			break;
		case X87_INT16:		// 16-bit int
			tmp = x87_rd_cnt(cpu, 2);
			res = tmp & 0x7fff;
			if ((tmp >> 15) & 1) {
				res = -res;
			}
			break;
		case X87_BCD:
			tmp = x87_rd_cnt(cpu, 8);
			res = 0;
			for(bexp = 0; bexp < 16; bexp++) {
				res += (tmp & 0x0f) * pow(10, bexp);
				tmp >>= 4;
			}
			cpu->ea.adr += 8;
			tmp = x87_rd_cnt(cpu, 2);
			cpu->ea.adr -= 8;
			res += (tmp & 0x0f) * 1e16;
			res += (tmp & 0xf0) * 1e17;
			if (tmp & 0x8000) {
				res = -res;
			}
			break;
		case X87_REAL80:
			tmp = x87_rd_cnt(cpu, 8);
			base = tmp;
			while (base > 2.0) {
				base /= 2.0;
			}
			cpu->ea.adr += 8;
			tmp = x87_rd_cnt(cpu, 2);
			cpu->ea.adr -= 8;
			bexp = (tmp & ((1<<15)-1)) - 0x3fff;
			res = base * pow(2.0, bexp);
			if (tmp & 0x8000) {
				res = -res;
			}
			break;
		default:
			printf("x87 memory format undefined\n");
			break;
	}
	return res;
}

double x87_round(CPU* cpu, double v) {
	switch((cpu->x87cr >> 10) & 3) {
		case 0:			// to nearest
			v = round(v);
			break;
		case 1:			// down
			v = ceil(v);
			break;
		case 2:			// up
			v = floor(v);
			break;
		case 3:			// chop (toward zero)
			v = trunc(v);
			break;
	}
	return v;
}

void x87_calc_se(CPU* cpu, long double v, int p, long long* s, long long* e) {
	long double t = abs(v);
	*e = 0;
	if (t == 0) {
		*s = 0;
	} else {
		while (t < 1.0) {
			t *= 2.0;
			(*e)--;
		}
		while (t >= 2.0) {
			t /= 2.0;
			(*e)++;
		}
		*s = (long long)(t * pow(2.0, p));
	}
}

void x87_wr_op(CPU* cpu, double v, int t) {
	long long r = 0;
	long long e = 0;
	long long s = 0;
	int c = 0;
	switch(t) {
		case X87_INT16:
			if ((v < INT16_MIN) || (v > INT16_MAX)) {
				x87_error(cpu, X87F_OE);
			} else {
				r = (long long)x87_round(cpu, v);
				c = 2;
			}
			break;
		case X87_INT32:
			if ((v < INT32_MIN) || (v > INT32_MAX)) {
				x87_error(cpu, X87F_OE);
			} else {
				r = (long long)x87_round(cpu, v);
				c = 4;
			}
			break;
		case X87_INT64:
			if ((v < INT64_MIN) || (v > INT64_MAX)) {
				x87_error(cpu, X87F_OE);
			} else {
				r = (long long)x87_round(cpu, v);
				c = 8;
			}
			break;
		case X87_REAL32:
			if ((v < FLT_MIN) || (v > FLT_MAX)) {
				x87_error(cpu, X87F_OE);
			} else {
				x87_calc_se(cpu, v, 24, &s, &e);
				e += 0x7f;
				r = (s & ((1 << 24) - 1)) | ((e & 0x7f) << 24);
				if (v < 0) {r |= (1<<31);}
				c = 4;
			}
			break;
		case X87_REAL64:
			if ((v < DBL_MIN) || (v > DBL_MAX)) {
				x87_error(cpu, X87F_OE);
			} else {
				x87_calc_se(cpu, v, 52, &s, &e);
				e += 0x3ff;
				r = (s & (((long long)1 << 52) - 1)) | ((e & 0x3ff) << 52);
				if (v < 0) {r |= ((long long)1<<63);}
				c = 8;
			}
			break;
		case X87_REAL80:
			x87_calc_se(cpu, v, 63, &s, &e);
			e += 0x3fff;
			r = (s | ((long long)1 << 63));
			e &= 0x7fff;
			if (v < 0) e |= 0x8000;
			c = 10;
			break;
	}
	int adr = cpu->ea.adr;
	int i = 0;
	while (i < c) {
		i286_mwr(cpu, cpu->ea.seg, 0, adr, r & 0xff);
		i++;
		adr++;
		r >>= 8;
		if (i == 8) r = e;		// extending long-long if nesessary
	}
}

// tag for SR(n)
// 00:valid, 01:zero, 10:Nan,Infinity,Denormal,Undefined, 11:empty
int x87_get_tag(CPU* cpu, int n) {
	n = (n & 7) << 1;
	return (cpu->x87tw >> n) & 3;
}

void x87_set_tag(CPU* cpu, int n, int t) {
	n = (n & 7) << 1;
	unsigned short msk = ~(3 << n);
	t = (t & 3) << n;
	cpu->x87tw = (cpu->x87tw & msk) | t;
}

// TODO: NaN, infinity
void x87_set_reg(CPU* cpu, int n, long double v) {
	n = (cpu->x87top + n) & 7;
	cpu->x87reg[n] = v;
	x87_set_tag(cpu, n, (v == 0) ? 1 : 0);
}

#define x87_pop cpu->x87top++
#define x87_push cpu->x87top--

#define X87ST0		cpu->x87reg[cpu->x87top]
#define X87ST(_n)	cpu->x87reg[(cpu->x87top + (_n)) & 7]

// FLD/FILD

void x87_fld(CPU* cpu) {x87_push; X87ST0 = x87_rd_op(cpu, (cpu->hcom >> 1) & 3);}
void x87_fld_r80(CPU* cpu) {x87_push; X87ST0 = x87_rd_op(cpu, X87_REAL80);}
void x87_fld_i64(CPU* cpu) {x87_push; X87ST0 = x87_rd_op(cpu, X87_INT64);}
void x87_fld_bcd(CPU* cpu) {x87_push; X87ST0 = x87_rd_op(cpu, X87_BCD);}
void x87_fld_st(CPU* cpu) {cpu->x87reg[8] = X87ST(cpu->com); x87_push; X87ST0 = cpu->x87reg[8];}

void x87_fld1(CPU* cpu) {x87_push; X87ST0 = 1.0;}
void x87_fldl2t(CPU* cpu) {x87_push; X87ST0 = log2(10);}
void x87_fldl2e(CPU* cpu) {x87_push; X87ST0 = M_LOG2E;}
void x87_fldpi(CPU* cpu) {x87_push; X87ST0 = M_PI;}
void x87_fldlg2(CPU* cpu) {x87_push; X87ST0 = log10(2);}
void x87_fldln2(CPU* cpu) {x87_push; X87ST0 = M_LN2;}
void x87_fldz(CPU* cpu) {x87_push; X87ST0 = 0.0;}

// FST/FIST/FSTP

void x87_fst_sti(CPU* cpu) {X87ST(cpu->com) = X87ST0;}
void x87_fstp_sti(CPU* cpu) {X87ST(cpu->com) = X87ST0; x87_pop;}
void x87_fst(CPU* cpu) {x87_wr_op(cpu, X87ST0, (cpu->hcom >> 1) & 3);}
void x87_fstp(CPU* cpu) {x87_wr_op(cpu, X87ST0, (cpu->hcom >> 1) & 3); x87_pop;}
void x87_fstp_r80(CPU* cpu) {x87_wr_op(cpu, X87ST0, X87_REAL80); x87_pop;}
void x87_fstp_i64(CPU* cpu) {x87_wr_op(cpu, X87ST0, X87_INT64); x87_pop;}
void x87_fstp_bcd(CPU* cpu) {x87_wr_op(cpu, X87ST0, X87_BCD); x87_pop;}

// FXCH
void x87_fxch(CPU* cpu) {
	long double t = X87ST0;
	X87ST0 = X87ST(cpu->com & 7);
	X87ST(cpu->com & 7) = t;
}

// st0 = -st0
void x87_fchs(CPU* cpu) {X87ST0 = -X87ST0;}
// st0 = |st0|
void x87_fabs(CPU* cpu) {if (X87ST0 < 0) {X87ST0 = -X87ST0;}}
// st0 = 2^st0 - 1
void x87_f2xm1(CPU* cpu) {X87ST0 = pow(2.0, X87ST0) - 1;}
// st1 = st1 * log2(st0)
void x87_fyl2x(CPU* cpu) {
	X87ST(1) = X87ST(1) * log2(X87ST0);
	x87_pop;
}
// st1 = st1 * log2(st0 + 1)
void x87_fyl2xp1(CPU* cpu) {
	X87ST(1) = X87ST(1) * log2(X87ST0 + 1);
	x87_pop;
}
// st0 = sqrt(st0)
void x87_fsqrt(CPU* cpu) {
	if (X87ST0 < 0) {
		x87_error(cpu, X87F_IE);
	} else {
		X87ST0 = sqrt(X87ST0);
	}
}
// st0 = round(st0)
void x87_frndint(CPU* cpu) {
	X87ST0 = x87_round(cpu, X87ST0);
}
// st0 = st0 * (2 ^ trunc(st1))
void x87_fscale(CPU* cpu) {
	X87ST0 = X87ST0 * pow(2, trunc(X87ST(1)));
}

/*
// st0 = cos(st0)
void x87_fcos(CPU* cpu) {
	if (X87ST0 < pow(2, 63)) {
		cpu->x87sr &= ~X87F_C2;
		X87ST0 = cos(X87ST0);
	} else {
		cpu->x87sr |= X87F_C2;
	}
}
// st0 = sin(st0)
void x87_fsin(CPU* cpu) {
	if (X87ST0 < pow(2, 63)) {
		cpu->x87sr &= ~X87F_C2;
		X87ST0 = sin(X87ST0);
	} else {
		cpu->x87sr |= X87F_C2;
	}
}
*/
// st0 = tan(st0)
void x87_fptan(CPU* cpu) {
	long double v = X87ST0;
	if (v < pow(2.0,63)) {
		cpu->x87sr &= ~X87F_C2;
		X87ST0 = tan(v);
		x87_push;
		X87ST0 = 1.0;
	} else {
		cpu->x87sr |= X87F_C2;
	}
}

void x87_fpatan(CPU* cpu) {
	if (X87ST0 == 0) {
		if (X87ST(1) < 0) {
			X87ST(1) = -M_PI;
		} else {
			X87ST(1) = M_PI;
		}
	} else {
		X87ST(1) = atan(X87ST(1) / X87ST0);
	}
	x87_pop;
}

// FADD/FIADD/FADDP

double x87_fadd_x(CPU* cpu, int n1, int n2) {
	// TODO: check ST(n1),ST(n2) for exceptions, errors...
	return X87ST(n1) + X87ST(n2);
}

void x87_fadd_sti(CPU* cpu) {
	X87ST0 = x87_fadd_x(cpu, 0, cpu->com & 7);
}

void x87_fadd_st0(CPU* cpu) {
	X87ST(cpu->com) = x87_fadd_x(cpu, 0, cpu->com & 7);
}

void x87_faddp(CPU* cpu) {
	x87_fadd_st0(cpu);
	x87_pop;
}

void x87_fadd_m(CPU* cpu) {
	X87ST(8) = x87_rd_op(cpu, (cpu->hcom >> 1) & 3);
	X87ST0 = x87_fadd_x(cpu, 0, 8);
}

// FMUL/FIMUL/FMULP

double x87_fmul_x(CPU* cpu, int n1, int n2) {return X87ST(n1) * X87ST(n2);}
void x87_fmul_sti(CPU* cpu) {X87ST0 = x87_fmul_x(cpu, 0, cpu->com & 7);}
void x87_fmul_st0(CPU* cpu) {X87ST(cpu->com) = x87_fmul_x(cpu, 0, cpu->com & 7);}
void x87_fmulp(CPU* cpu) {x87_fmul_st0(cpu); x87_pop;}
void x87_fmul_m(CPU* cpu) {
	X87ST(8) = x87_rd_op(cpu, (cpu->hcom >> 1) & 3);
	X87ST0 = x87_fmul_x(cpu, 0, 8);
}

// FCOM/FCOMP/FCOMPP

void x87_comp_8(CPU* cpu) {
	cpu->x87sr &= ~X87F_C1;			// c1 = 0
	if (X87ST0 > X87ST(8)) {
		cpu->x87sr &= ~(X87F_C0 | X87F_C2 | X87F_C3);	// 000
	} else if (X87ST0 < X87ST(8)) {
		cpu->x87sr &= ~(X87F_C2 | X87F_C3);		// 001
		cpu->x87sr |= X87F_C0;
	} else {
		cpu->x87sr &= ~(X87F_C0 | X87F_C2);		// 100
		cpu->x87sr |= X87F_C3;
	}
}

void x87_fcom(CPU* cpu) {
	if ((cpu->com & 0xc0) == 0xc0) {	// registers
		X87ST(8) = X87ST(cpu->com & 7);
	} else {
		if (cpu->com & 0x400) {		// mem:real64
			X87ST(8) = x87_rd_op(cpu, X87_REAL64);
		} else {			// mem:real32
			X87ST(8) = x87_rd_op(cpu, X87_REAL32);
		}
	}
	x87_comp_8(cpu);
	if (cpu->com & 8) {		// pop
		x87_pop;
	}
}

void x87_fcompp(CPU* cpu) {
	X87ST(8) = X87ST(1);
	x87_comp_8(cpu);
	x87_pop;
	x87_pop;
}

void x87_ftst(CPU* cpu) {
	X87ST(8) = 0;
	x87_comp_8(cpu);
}

void x87_fxam(CPU* cpu) {
	double v = X87ST0;
	cpu->x87sr &= (X87F_C0 | X87F_C1 | X87F_C2 | X87F_C3);
	if (v < 0) {
		cpu->x87sr |= X87F_C1;
	}
	switch(x87_get_tag(cpu, cpu->x87top)) {		// get tag of ST0
		case 0:
		case 1: cpu->x87sr |= v ? X87F_C2 : X87F_C3;	// C2 (normal), C3 (zero)
			break;
		case 2:						// Nan,infinity
			cpu->x87sr |= X87F_C0;
			// set C2 if infinity
			break;
		case 3: cpu->x87sr |= (X87F_C0 | X87F_C3);	// empty
			break;
	}
}

// FSUB/FISUB/FSUBP

double x87_fsub_x(CPU* cpu, int n1, int n2) {
	return X87ST(n1) - X87ST(n2);
}

void x87_fsub_sti(CPU* cpu) {X87ST0 = x87_fsub_x(cpu, 0, cpu->com & 7);}
void x87_fsub_st0(CPU* cpu) {X87ST(cpu->com) = x87_fsub_x(cpu, cpu->com & 7, 0);}
void x87_fsubp_sti(CPU* cpu) {x87_fsub_st0(cpu); x87_pop;}
void x87_fsubrp_st0(CPU* cpu) {x87_fsub_sti(cpu); x87_pop;}
void x87_fsub_m(CPU* cpu) {
	X87ST(8) = x87_rd_op(cpu, (cpu->hcom >> 1) & 3);
	X87ST0 = x87_fsub_x(cpu, 0, 8);
}
void x87_fsubr_m(CPU* cpu) {
	X87ST(8) = x87_rd_op(cpu, (cpu->hcom >> 1) & 3);
	X87ST0 = x87_fsub_x(cpu, 8, 0);
}

// FDIV/FIDIV/FDIVP
double x87_fdiv_x(CPU* cpu, int n1, int n2) {
	double res = 0;
	if (X87ST(n2) == 0) {
		x87_error(cpu, X87F_DE);
	} else {
		res = X87ST(n1) / X87ST(n2);
	}
	return res;
}

void x87_fdiv_sti(CPU* cpu) {X87ST0 = x87_fdiv_x(cpu, 0, cpu->com & 7);}
void x87_fdiv_st0(CPU* cpu) {X87ST(cpu->com) = x87_fdiv_x(cpu, cpu->com & 7, 0);}
void x87_fdivp_st0(CPU* cpu) {x87_fdiv_st0(cpu); x87_pop;}
void x87_fdivrp_sti(CPU* cpu) {x87_fdiv_sti(cpu); x87_pop;}
void x87_fdiv_m(CPU* cpu) {
	X87ST(8) = x87_rd_op(cpu, (cpu->hcom >> 1) & 3);
	X87ST0 = x87_fdiv_x(cpu, 0, 8);
}
void x87_fdivr_m(CPU* cpu) {
	X87ST(8) = x87_rd_op(cpu, (cpu->hcom >> 1) & 3);
	X87ST0 = x87_fdiv_x(cpu, 8, 0);
}

void x87_fprem(CPU* cpu) {
	long double q = floor(X87ST0 / X87ST(1));
	X87ST0 -= q * X87ST(1);
	// C0-3
}

// system

void x87_finit(CPU* cpu) {
	cpu->x87cr = 0x37f;
	cpu->x87sr = 0;
	cpu->x87tw = 0xffff;
}

void x87_wr_word(CPU* cpu, xSegPtr seg, unsigned short adr, int v) {
	i286_mwr(cpu, seg, 0, adr, v & 0xff);
	i286_mwr(cpu, seg, 0, adr+1, (v >> 1) & 0xff);
}

void x87_fstenv(CPU* cpu) {
	int adr = cpu->ea.adr;
	cpu->x87sr &= ~(7 << 11);
	cpu->x87sr |= ((cpu->x87top & 7) << 11);
	x87_wr_word(cpu, cpu->ea.seg, adr, cpu->x87cr);
	x87_wr_word(cpu, cpu->ea.seg, adr+2, cpu->x87sr);
	x87_wr_word(cpu, cpu->ea.seg, adr+4, cpu->x87tw);
	x87_wr_word(cpu, cpu->ea.seg, adr+6, cpu->regIP);
	x87_wr_word(cpu, cpu->ea.seg, adr+8, cpu->cs.idx);
	x87_wr_word(cpu, cpu->ea.seg, adr+10, cpu->ea.adr);
	x87_wr_word(cpu, cpu->ea.seg, adr+12, cpu->ea.seg.idx);
}

void x87_fldenv(CPU* cpu) {
	int adr = cpu->ea.adr;
	cpu->x87cr = x87_rd_cnt(cpu, 2) & 0xffff; cpu->ea.adr += 2;
	cpu->x87sr = x87_rd_cnt(cpu, 2) & 0xffff; cpu->ea.adr += 2;
	cpu->x87tw = x87_rd_cnt(cpu, 2) & 0xffff; cpu->ea.adr += 2;
	cpu->tmpw = x87_rd_cnt(cpu, 2) & 0xffff; cpu->ea.adr += 2;
	cpu->tmpw = x87_rd_cnt(cpu, 2) & 0xffff; cpu->ea.adr += 2;
	cpu->tmpw = x87_rd_cnt(cpu, 2) & 0xffff;
	cpu->x87top = (cpu->x87sr >> 11) & 7;
	cpu->ea.adr = adr;
}

void x87_fsave(CPU* cpu) {
	int adr = cpu->ea.adr;
	x87_fstenv(cpu);
	cpu->ea.adr += 14;
	for (int i = 0; i < 8; i++) {
		x87_wr_op(cpu, X87ST(i), X87_REAL80);
		cpu->ea.adr += 10;
	}
	cpu->ea.adr = adr;
}

void x87_frstr(CPU* cpu) {
	int adr = cpu->ea.adr;
	x87_fldenv(cpu);
	cpu->ea.adr += 14;
	for (int i = 0; i < 8; i++) {
		X87ST(i) = x87_rd_op(cpu, X87_REAL80);
		cpu->ea.adr += 10;
	}
	cpu->ea.adr = adr;
}

void x87_fldcw(CPU* cpu) {
	cpu->x87cr = x87_rd_cnt(cpu, 2) & 0xffff;
}

void x87_fstcw(CPU* cpu) {
	x87_wr_word(cpu, cpu->ea.seg, cpu->ea.adr, cpu->x87cr);
}

void x87_fstsw_ax(CPU* cpu) {
	cpu->x87sr &= ~(7 << 11);
	cpu->x87sr |= ((cpu->x87top & 7) << 11);
	cpu->regAX = cpu->x87sr;
}

// clear exceptions
void x87_fclex(CPU* cpu) {
	cpu->x87sr &= ~0x80ff;	// b15, b7:0 = 0
}

void x87_fdecstp(CPU* cpu) {cpu->x87top--;}
void x87_fincstp(CPU* cpu) {cpu->x87top++;}

void x87_fxtract(CPU* cpu) {
	long long s = 0;
	long long e = 0;
	x87_calc_se(cpu, X87ST0, 64, &s, &e);
	X87ST0 = e;
	x87_pop;
	X87ST0 = s;
}

void x87_ffree(CPU* cpu) {
	x87_set_tag(cpu, cpu->com & 7, 3);
}

void x87_fnop(CPU* cpu) {}

void x87_undef(CPU* cpu) {
	x87_error(cpu, X87F_IE);		// invalid operation
}

typedef struct {
	int mask;
	int val;
	cbcpu exec;
	const char* mnem;
} x87opCode;

x87opCode x87tab[] = {
	// ... commands with ea is reg

	{0x7ff,0x1d0,x87_fnop,"fnop"},
	{0x7ff,0x1e0,x87_fchs,"fchs"},
	{0x7ff,0x1e1,x87_fabs,"fabs"},
	{0x7ff,0x1e4,x87_ftst,"ftst"},
	{0x7ff,0x1e5,x87_fxam,"fxam"},
	{0x7ff,0x1e8,x87_fld1,"fld1"},
	{0x7ff,0x1e9,x87_fldl2t,"fldl2t"},
	{0x7ff,0x1ea,x87_fldl2e,"fldl2e"},
	{0x7ff,0x1eb,x87_fldpi,"fldpi"},
	{0x7ff,0x1ec,x87_fldlg2,"fldlg2"},
	{0x7ff,0x1ed,x87_fldln2,"fldln2"},
	{0x7ff,0x1ee,x87_fldz,"fldz"},
	{0x7ff,0x1f0,x87_f2xm1,"f2xm1"},
	{0x7ff,0x1f1,x87_fyl2x,"fyl2x"},
	{0x7ff,0x1f2,x87_fptan,"fptan"},
	{0x7ff,0x1f2,x87_fpatan,"fpatan"},
	{0x7ff,0x1f4,x87_fxtract,"fxtract"},
	{0x7ff,0x1f6,x87_fdecstp,"fdecstp"},
	{0x7ff,0x1f7,x87_fincstp,"fincstp"},
	{0x7ff,0x1f8,x87_fprem,"fprem"},
	{0x7ff,0x1f9,x87_fyl2xp1,"fyl2xp1"},
	{0x7ff,0x1fa,x87_fsqrt,"fsqrt"},
	{0x7ff,0x1fc,x87_frndint,"frndint"},
	{0x7ff,0x1fd,x87_fscale,"fscale"},
	{0x7ff,0x3e2,x87_fclex, "fclex"},
	{0x7ff,0x3e3,x87_finit,"finit"},
	{0x7ff,0x7e0,x87_fstsw_ax,"fstsw ax"},

	{0x7f8,0x0c0,x87_fadd_sti,"fadd st0,st:7"},
	{0x7f8,0x4c0,x87_fadd_st0,"fadd st:7,st0"},
	{0x7f8,0x6c0,x87_faddp,"faddp st:7,st0"},

	{0x7f8,0x0c8,x87_fmul_sti,"fmul st0,st:7"},
	{0x7f8,0x4c8,x87_fmul_st0,"fmul st:7,st:0"},
	{0x7f8,0x6c8,x87_fmulp,"fmulp st:7,st0"},

	{0x7f8,0x0d0,x87_fcom,"fcom st:7"},
	{0x7f8,0x0d8,x87_fcom,"fcomp st:7"},
	{0x7ff,0x6d9,x87_fcompp,"fcompp"},

	{0x7f8,0x0e0,x87_fsub_sti,"fsub st0,st:7"},
	{0x7f8,0x4e8,x87_fsub_st0,"fsub st:7,st0"},
	{0x7f8,0x6e8,x87_fsubp_sti,"fsubp st:7,st0"},
	{0x7f8,0x0e8,x87_fsub_st0,"fsubr st0,st:7"},
	{0x7f8,0x4e0,x87_fsub_sti,"fsubr st:7,st0"},
	{0x7f8,0x6e0,x87_fsubrp_st0,"fsubrp st:7,st0"},

	{0x7f8,0x0f0,x87_fdiv_sti,"fdiv st0,st:7"},
	{0x7f8,0x4f8,x87_fdiv_st0,"fdiv st:7,st0"},
	{0x7f8,0x6f8,x87_fdivp_st0,"fdivp st:7,st0"},
	{0x7f8,0x0f8,x87_fdiv_st0,"fdivr st0,st:7"},
	{0x7f8,0x4f0,x87_fdiv_sti,"fdivr st:7,st0"},
	{0x7f8,0x6f0,x87_fdivrp_sti,"fdivrp st:7,st0"},

	{0x7f8,0x1c0,x87_fld_st,"fld st:7"},

	{0x7f8,0x1c8,x87_fxch,"fxch st:7"},
	{0x7f8,0x5c0,x87_ffree,"ffree st:7"},
	{0x7f8,0x5d0,x87_fst_sti,"fst st:7"},
	{0x7f8,0x5d8,x87_fstp_sti,"fstp st:7"},

	{0x0c0,0x0c0,x87_undef,"* x87 undef"},

	// commands with ea is mem

	{0x738,0x000,x87_fadd_m,"fadd float :e"},
	{0x738,0x200,x87_fadd_m,"fiadd dword :e"},
	{0x738,0x400,x87_fadd_m,"fadd double :e"},
	{0x738,0x600,x87_fadd_m,"fiadd word :e"},

	{0x738,0x008,x87_fmul_m, "fmul float :e"},
	{0x738,0x208,x87_fmul_m, "fimul dword :e"},
	{0x738,0x408,x87_fmul_m, "fmul double :e"},
	{0x738,0x608,x87_fmul_m, "fimul word :e"},

	{0x738,0x010,x87_fcom,"fcom float :e"},
	{0x738,0x210,x87_fcom,"ficom dword :e"},
	{0x738,0x418,x87_fcom,"fcomp double :e"},
	{0x738,0x618,x87_fcom,"ficomp word :e"},

	{0x738,0x020,x87_fsub_m,"fsub float :e"},
	{0x738,0x220,x87_fsub_m,"fisub dword :e"},
	{0x738,0x420,x87_fsub_m,"fsub double :e"},
	{0x738,0x620,x87_fsub_m,"fisub word :e"},
	{0x738,0x028,x87_fsubr_m,"fsubr float :e"},
	{0x738,0x228,x87_fsubr_m,"fisubr dword :e"},
	{0x738,0x428,x87_fsubr_m,"fsubr double :e"},
	{0x738,0x628,x87_fsubr_m,"fisubr word :e"},

	{0x738,0x030,x87_fdiv_m,"fdiv float :e"},
	{0x738,0x030,x87_fdiv_m,"fdiv dword :e"},
	{0x738,0x030,x87_fdiv_m,"fdiv double :e"},
	{0x738,0x030,x87_fdiv_m,"fdiv word :e"},
	{0x738,0x038,x87_fdivr_m,"fdivr float :e"},
	{0x738,0x038,x87_fdivr_m,"fdivr dword :e"},
	{0x738,0x038,x87_fdivr_m,"fdivr double :e"},
	{0x738,0x038,x87_fdivr_m,"fdivr word :e"},

	{0x738,0x100,x87_fld,"fld float :e"},
	{0x738,0x300,x87_fld,"fild dword :e"},
	{0x738,0x500,x87_fld,"fld double :e"},
	{0x738,0x700,x87_fld,"fild word :e"},
	{0x738,0x328,x87_fld_r80,"fld real :e"},
	{0x738,0x720,x87_fld_bcd,"fbld :e"},
	{0x738,0x728,x87_fld_i64,"fld qword :e"},

	{0x738,0x110,x87_fst,"fst float :e"},
	{0x738,0x310,x87_fst,"fist dword :e"},
	{0x738,0x510,x87_fst,"fst double :e"},
	{0x738,0x710,x87_fst,"fist word :e"},
	{0x738,0x118,x87_fstp,"fstp float :e"},
	{0x738,0x318,x87_fstp,"fistp dword :e"},
	{0x738,0x518,x87_fstp,"fstp double :e"},
	{0x738,0x718,x87_fstp,"fistp word :e"},
	{0x738,0x338,x87_fstp_r80,"fstp real :e"},
	{0x738,0x730,x87_fstp_bcd,"fbstp :e"},
	{0x738,0x738,x87_fstp_i64,"fstp qword :e"},

	{0x738,0x120,x87_fldenv,"fldenv :e"},
	{0x738,0x128,x87_fldcw,"fldcw :e"},
	{0x738,0x130,x87_fstenv,"fstenv :e"},
	{0x738,0x520,x87_frstr,"frstor :e"},
	{0x738,0x530,x87_fsave,"fsave :e"},
	{0x738,0x538,x87_fstcw,"fstcw :e"},

	{0x000,0x000,x87_undef,"* x87 undef"}
};

// cpu->com = D8..DF
void x87_exec(CPU* cpu) {
	cpu->hcom = cpu->com & 7;
	cpu->lcom = cpu->mod;
	int idx = -1;
	int work = 1;
	do {
		idx++;
		if (((cpu->com ^ x87tab[idx].val) & x87tab[idx].mask) == 0) {
			if (x87tab[idx].exec) {
				x87tab[idx].exec(cpu);
			} else {
				x87_undef(cpu);
			}
			work = 0;
		}
	} while (work && x87tab[idx].mask);
}

const char* x87_get_mnem(CPU* cpu, int com) {
	int i = 0;
	int work = 1;
	const char* ptr = NULL;
	do {
		if (((com ^ x87tab[i].val) & x87tab[i].mask) == 0) {
			ptr = x87tab[i].mnem;
			work = 0;
		} else {
			i++;
		}
	} while (work);
	return ptr;
}
