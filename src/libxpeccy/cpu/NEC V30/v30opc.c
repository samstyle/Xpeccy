#include "v30.h"
#include "v30regs.h"

#include <stdio.h>

extern opCode v30_0f_tab[256];

void v30_exception(CPU*, int, int);

// get/set reg8 or r16 by index in mod byte
size_t v30_off_r8[8] = {
	offsetof(CPU, regAL), offsetof(CPU, regCL), offsetof(CPU, regDL), offsetof(CPU, regBL),
	offsetof(CPU, regAH), offsetof(CPU, regCH), offsetof(CPU, regDH), offsetof(CPU, regBH)
};

size_t v30_off_r16[8] = {
	offsetof(CPU, regAW), offsetof(CPU, regCW), offsetof(CPU, regDW), offsetof(CPU, regBW),
	offsetof(CPU, regSP), offsetof(CPU, regBP), offsetof(CPU, regIX), offsetof(CPU, regIY)
};

int v30_get_reg8(CPU* cpu, int idx) {
	size_t off = v30_off_r8[idx & 7];
	xbyte* ptr = ((void*)cpu) + off;
	return (*ptr) & 0xff;
}

void v30_set_reg8(CPU* cpu, int idx, int val) {
	size_t off = v30_off_r8[idx & 7];
	xbyte* ptr = ((void*)cpu) + off;
	*ptr = (val & 0xff);
}

int v30_get_reg16(CPU* cpu, int idx) {
	size_t off = v30_off_r16[idx & 7];
	xword* ptr = ((void*)cpu) + off;
	return (*ptr) & 0xffff;
}

void v30_set_reg16(CPU* cpu, int idx, int val) {
	size_t off = v30_off_r16[idx & 7];
	xword* ptr = ((void*)cpu) + off;
	*ptr = (val & 0xffff);
}

#define V30GETREG8	v30_get_reg8(cpu, (cpu->regMOD >> 3) & 7)
#define V30GETREG16	v30_get_reg16(cpu, (cpu->regMOD >> 3) & 7)
#define V30SETREG8(_n)	v30_set_reg8(cpu, (cpu->regMOD >> 3) & 7, _n)
#define V30SETREG16(_n)	v30_set_reg16(cpu, (cpu->regMOD >> 3) & 7, _n)

// need set cs.base for deBUGa
void v30_set_ps(CPU*, int);

// mrd/mwr byte or word
int v30_mrdb(CPU* cpu, int seg, int adr) {
	return cpu_mrd(cpu, (seg << 4) + adr);
}

int v30_mrdw(CPU* cpu, int seg, int adr) {
	xreg16 r;
	r.l = cpu_mrd(cpu, (seg << 4) + adr);
	adr++;
	r.h = cpu_mrd(cpu, (seg << 4) + adr);
	return r.w;
}

void v30_mwrb(CPU* cpu, int seg, int adr, int d) {
	cpu_mwr(cpu, (seg << 4) + adr, d);
}

void v30_mwrw(CPU* cpu, int seg, int adr, int val) {
	xreg16 r;
	r.w = val & 0xffff;
	cpu_mwr(cpu, (seg << 4) + adr, r.l);
	adr++;
	cpu_mwr(cpu, (seg << 4) + adr, r.h);
}

// mrd imm byte or word
int v30_immb(CPU* cpu) {
	int r = v30_mrdb(cpu, cpu->regPS, cpu->regPC);
	cpu->regPC++;
	return r;
}

int v30_immw(CPU* cpu) {
	xreg16 r;
	r.l = v30_immb(cpu);
	r.h = v30_immb(cpu);
	return r.w;
}

// push/pop word
int v30_pop(CPU* cpu) {
	xword rw;
	rw = v30_mrdw(cpu, cpu->regSS, cpu->regSP);
	cpu->regSP += 2;
	return rw;
}

void v30_push(CPU* cpu, int d) {
	cpu->regSP -= 2;
	v30_mwrw(cpu, cpu->regSS, cpu->regSP, d & 0xffff);
}

// read mod byte and calculate ea or reg
void v30_get_ea(CPU* cpu, int w) {
	cpu->regMOD = v30_immb(cpu);
	// dst
	if (w) {
		cpu->twrd = v30_get_reg16(cpu, (cpu->regMOD >> 3) & 7);
	} else {
		cpu->lwr = v30_get_reg8(cpu, (cpu->regMOD >> 3) & 7);
		cpu->hwr = 0;
	}
	// src
	if ((cpu->regMOD & 0xc0) == 0xc0) {	// reg-reg
		cpu->ea.reg = 1;
		if (w) {
			cpu->tmpw = v30_get_reg16(cpu, cpu->regMOD & 7);
		} else {
			cpu->ltw = v30_get_reg8(cpu, cpu->regMOD & 7);
			cpu->htw = 0;
		}
	} else {
		cpu->ea.reg = 0;
		xreg16 disp;
		switch (cpu->regMOD & 0xc0) {
			case 0x40: disp.l = v30_immb(cpu); disp.h = 0; break;
			case 0x80: disp.w = v30_immw(cpu); break;
			default: disp.w = 0; break;
		}
		switch(cpu->regMOD & 7) {
			case 0: cpu->ea.seg.idx = cpu->regDS0;
				cpu->ea.adr = cpu->regBW + cpu->regIX + disp.w;
				break;
			case 1: cpu->ea.seg.idx = cpu->regDS0;
				cpu->ea.adr = cpu->regBW + cpu->regIY + disp.w;
				break;
			case 2: cpu->ea.seg.idx = cpu->regSS;
				cpu->ea.adr = cpu->regBP + cpu->regIX + disp.w;
				break;
			case 3: cpu->ea.seg.idx = cpu->regSS;
				cpu->ea.adr = cpu->regBP + cpu->regIY + disp.w;
				break;
			case 4: cpu->ea.seg.idx = cpu->regDS0;
				cpu->ea.adr = cpu->regIX + disp.w;
				break;
			case 5: cpu->ea.seg.idx = cpu->regDS0;
				cpu->ea.adr = cpu->regIY + disp.w;
				break;
			case 6: if (cpu->regMOD & 0xc0) {
					cpu->ea.seg.idx = cpu->regSS;
					cpu->ea.adr = cpu->regBP + disp.w;
				} else {
					cpu->ea.seg.idx = cpu->regDS0;
					cpu->ea.adr = v30_immw(cpu);
				}
				break;
			case 7: cpu->ea.seg.idx = cpu->regDS0;
				cpu->ea.adr = cpu->regBW + disp.w;
				break;
		}
		if (cpu->flgSOVR) cpu->ea.seg.idx = cpu->regSEG;	// if segment overwrite prefix
	}
}

int v30_rd_ea(CPU* cpu, int w) {
	v30_get_ea(cpu, w);
	if (!cpu->ea.reg) {
		if (w) {
			cpu->tmpw = v30_mrdw(cpu, cpu->ea.seg.idx, cpu->ea.adr);
		} else {
			cpu->ltw = v30_mrdb(cpu, cpu->ea.seg.idx, cpu->ea.adr);
			cpu->htw = 0;
		}
	}
	return cpu->tmpw;
}

void v30_wr_ea(CPU* cpu, int v, int w) {
	// ea is already set by v30_get_ea
	if (cpu->ea.reg) {
		if (w) {
			v30_set_reg16(cpu, cpu->regMOD & 7, v & 0xffff);
		} else {
			v30_set_reg8(cpu, cpu->regMOD & 7, v & 0xff);
		}
	} else {
		if (w) {
			v30_mwrw(cpu, cpu->ea.seg.idx, cpu->ea.adr, v & 0xffff);
		} else {
			v30_mwrb(cpu, cpu->ea.seg.idx, cpu->ea.adr, v & 0xff);
		}
	}
}

void v30_undef(CPU* cpu) {
	printf("v30: undef opcode @ %X:%X\n", cpu->regPS, cpu->oldpc);
	cpu_irq(cpu, IRQ_PANIC);
//	THROW(V30_INT_UD);		// no #UD exception before 286
}

// alu

static const int v30_add_FO[8] = {0, 0, 0, 1, 1, 0, 0, 0};

unsigned char v30_add8(CPU* cpu, unsigned char p1, unsigned char p2, int cf) {
	int r1 = p1 & 0xff;
	int r2 = (p2 & 0xff) + (cf ? 1 : 0);
	int res = r1 + r2;
	cpu->flgV = !!(((p1 ^ p2 ^ 0x80) & (res ^ p2)) & 0x80);
	cpu->flgS = !!(res & 0x80);
	cpu->flgZ = !(res & 0xff);
	cpu->flgAC = !!((p1 & 15) + (p2 & 15) > 15);
	cpu->flgP = parity(res & 0xff);
	cpu->flgCY = !!(res & ~0xff);
	return res & 0xff;
}

unsigned short v30_add16(CPU* cpu, unsigned short p1, unsigned short p2, int cf) {
	int r1 = p1 & 0xffff;
	int r2 = (p2 & 0xffff) + (cf ? 1 : 0);
	int res = r1 + r2;
	cpu->flgV = !!(((p1 ^ p2 ^ 0x8000) & (res ^ p2)) & 0x8000);
	cpu->flgS = !!(res & 0x8000);
	cpu->flgZ = !(res & 0xffff);
	cpu->flgAC = ((p1 & 0xfff) + (p2 & 0xfff) > 0xfff);
	cpu->flgP = parity(res & 0xff);
	cpu->flgCY = !!(res & ~0xffff);
	return res & 0xffff;
}

unsigned char v30_sub8(CPU* cpu, unsigned char p1, unsigned char p2, int cf) {
	int r1 = p1 & 0xff;
	int r2 = (p2 & 0xff) + (cf ? 1 : 0);
	int res = r1 - r2;
	cpu->flgV = !!(((p1 ^ p2) & (p1 ^ res)) & 0x80);
	cpu->flgS = !!(res & 0x80);
	cpu->flgZ = !(res & 0xff);
	cpu->flgP = parity(res & 0xff);
	cpu->flgCY = !!(res & ~0xff);
	cpu->flgAC = !!((p1 & 0x0f) < (p2 & 0x0f));
	return res & 0xff;
}

unsigned short v30_sub16(CPU* cpu, unsigned short p1, unsigned short p2, int cf) {
	int r1 = p1 & 0xffff;
	int r2 = (p2 & 0xffff) + (cf ? 1 : 0);
	int res = r1 - r2;
	cpu->flgV = !!(((p1 ^ p2) & (p1 ^ res)) & 0x8000);
	cpu->flgS = !!(res & 0x8000);
	cpu->flgZ = !(res & 0xffff);
	cpu->flgP = parity(res & 0xff);
	cpu->flgCY = !!(res & ~0xffff);
	cpu->flgAC = !!((p1 & 0x0fff) < (p2 & 0x0fff));
	return res & 0xffff;
}

// mul
int v30_smul(CPU* cpu, signed short p1, signed short p2) {
	int res = p1 * p2;
	cpu->flgCY = !!((p1 & 0x7fff) * (p2 & 0x7fff) > 0xffff);
	cpu->tmp = ((p1 & 0x8000) >> 15) | ((p2 & 0x8000) >> 14) | ((res & 0x8000) >> 13);
	cpu->flgV = !!v30_add_FO[cpu->tmp & 7];
	return res;
}

// logic
unsigned char v30_and8(CPU* cpu, unsigned char p1, unsigned char p2) {
	p1 &= p2;
	cpu->flgS = !!(p1 & 0x80);
	cpu->flgZ = !p1;
	cpu->flgP = parity(p1 & 0xff);
	cpu->flgV = 0;
	cpu->flgCY = 0;
	return p1;
}

unsigned short v30_and16(CPU* cpu, unsigned short p1, unsigned short p2) {
	p1 &= p2;
	cpu->flgS = !!(p1 & 0x8000);
	cpu->flgZ = !p1;
	cpu->flgP = parity(p1 & 0xff);
	cpu->flgV = 0;
	cpu->flgCY = 0;
	return p1;
}

unsigned char v30_or8(CPU* cpu, unsigned char p1, unsigned char p2) {
	unsigned char res =  p1 | p2;
	cpu->flgS = !!(res & 0x80);
	cpu->flgZ = !res;
	cpu->flgP = parity(res & 0xff);
	cpu->flgV = 0;
	cpu->flgCY = 0;
	return res;
}

unsigned short v30_or16(CPU* cpu, unsigned short p1, unsigned short p2) {
	unsigned short res =  p1 | p2;
	cpu->flgS = !!(res & 0x8000);
	cpu->flgZ = !res;
	cpu->flgP = parity(res & 0xff);
	cpu->flgV = 0;
	cpu->flgCY = 0;
	return res;
}

unsigned char v30_xor8(CPU* cpu, unsigned char p1, unsigned char p2) {
	p1 ^= p2;
	cpu->flgS = !!(p1 & 0x80);
	cpu->flgZ = !p1;
	cpu->flgP = parity(p1 & 0xff);
	cpu->flgV = 0;
	cpu->flgCY = 0;
	return p1;
}

unsigned short v30_xor16(CPU* cpu, unsigned short p1, unsigned short p2) {
	p1 ^= p2;
	cpu->flgS = !!(p1 & 0x8000);
	cpu->flgZ = !p1;
	cpu->flgP = parity(p1 & 0xff);
	cpu->flgV = 0;
	cpu->flgCY = 0;
	return p1;
}

// rotate

// rol: CF<-b7...b0<-b7
unsigned char v30_rol8(CPU* cpu, unsigned char p) {
	p = (p << 1) | ((p & 0x80) ? 1 : 0);
	cpu->flgCY = p & 1;
	cpu->flgV = !!(!cpu->flgCY != !(p & 0x80));
	return p;
}

unsigned short v30_rol16(CPU* cpu, unsigned short p) {
	p = (p << 1) | ((p & 0x8000) ? 1 : 0);
	cpu->flgCY = p & 1;
	cpu->flgV = (!cpu->flgCY != !(p & 0x8000));
	return p;
}

// ror: b0->b7...b0->CF
unsigned char v30_ror8(CPU* cpu, unsigned char p) {
	p = (p >> 1) | ((p & 1) ? 0x80 : 0);
	cpu->flgCY = !!(p & 0x80);
	cpu->flgV = !!(!(p & 0x80) != !(p & 0x40));
	return p;
}

unsigned short v30_ror16(CPU* cpu, unsigned short p) {
	p = (p >> 1) | ((p & 1) ? 0x8000 : 0);
	cpu->flgCY = !!(p & 0x8000);
	cpu->flgV = !!(!(p & 0x8000) != !(p & 0x4000));
	return p;
}

// rcl: CF<-b7..b0<-CF
unsigned char v30_rcl8(CPU* cpu, unsigned char p) {
	cpu->tmp = cpu->flgCY;
	cpu->flgCY = !!(p & 0x80);
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	cpu->flgV = !!(!cpu->flgCY != !(p & 0x80));
	return p;
}

unsigned short v30_rcl16(CPU* cpu, unsigned short p) {
	cpu->tmp = cpu->flgCY;
	cpu->flgCY = !!(p & 0x8000);
	p = (p << 1) | (cpu->tmp ? 1 : 0);
	cpu->flgV = !!(!cpu->flgCY != !(p & 0x8000));
	return p;
}

// rcr: CF->b7..b0->CF
unsigned char v30_rcr8(CPU* cpu, unsigned char p) {
	cpu->tmp = cpu->flgCY;
	cpu->flgCY = p & 1;
	p >>= 1;
	if (cpu->tmp) p |= 0x80;
	cpu->flgV = !!(!(p & 0x80) != !(p & 0x40));
	return p;
}

unsigned short v30_rcr16(CPU* cpu, unsigned short p) {
	cpu->tmp = cpu->flgCY;
	cpu->flgCY = p & 1;
	p >>= 1;
	if (cpu->tmp) p |= 0x8000;
	cpu->flgV = !!(!(p & 0x8000) != !(p & 0x4000));
	return p;
}

// sal: CF<-b7..b0<-0
unsigned char v30_sal8(CPU* cpu, unsigned char p) {
	cpu->flgCY = !!(p & 0x80);
	p <<= 1;
	cpu->flgV = (!cpu->flgCY != !(p & 0x80));
	cpu->flgZ = !p;
	cpu->flgP = parity(p & 0xff);
	cpu->flgS = !!(p & 0x80);
	return p;
}

unsigned short v30_sal16(CPU* cpu, unsigned short p) {
	cpu->flgCY = !!(p & 0x8000);
	p <<= 1;
	cpu->flgV = !!(!cpu->flgCY != !(p & 0x8000));
	cpu->flgZ = !p;
	cpu->flgP = parity(p & 0xff);
	cpu->flgS = !!(p & 0x8000);
	return p;
}

// shr 0->b7..b0->CF
unsigned char v30_shr8(CPU* cpu, unsigned char p) {
	cpu->flgCY = p & 1;
	cpu->flgV = !!(p & 0x80);
	p >>= 1;
	cpu->flgZ = !p;
	cpu->flgP = parity(p & 0xff);
	cpu->flgS = !!(p & 0x80);
	return p;
}

unsigned short v30_shr16(CPU* cpu, unsigned short p) {
	cpu->flgCY = p & 1;
	cpu->flgV = !!(p & 0x8000);
	p >>= 1;
	cpu->flgZ = !p;
	cpu->flgP = parity(p & 0xff);
	cpu->flgS = !!(p & 0x8000);
	return p;
}

// sar b7->b7..b0->CF
unsigned char v30_sar8(CPU* cpu, unsigned char p) {
	cpu->flgCY = p & 1;
	p = (p >> 1) | (p & 0x80);
	cpu->flgZ = !p;
	cpu->flgP = parity(p & 0xff);
	cpu->flgS = !!(p & 0x80);
	cpu->flgV = 0;
	return p;
}

unsigned short v30_sar16(CPU* cpu, unsigned short p) {
	cpu->flgCY = p & 1;
	p = (p >> 1) | (p & 0x8000);
	cpu->flgZ = !p;
	cpu->flgP = parity(p & 0xff);
	cpu->flgS = !!(p & 0x8000);
	cpu->flgV = 0;
	return p;
}

// opcodes

// 00,mod: add eb,rb	EA.byte += reg.byte
void v30_op00(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_add8(cpu, cpu->ltw, cpu->lwr, 0);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// 01,mod: add ew,rw	EA.word += reg.word
void v30_op01(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_add16(cpu, cpu->tmpw, cpu->twrd, 0);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 02,mod: add rb,eb	reg.byte += EA.byte
void v30_op02(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_add8(cpu, cpu->ltw, cpu->lwr, 0);
	V30SETREG8(cpu->ltw);
}

// 03,mod: add rw,ew	reg.word += EA.word
void v30_op03(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_add16(cpu, cpu->tmpw, cpu->twrd, 0);
	V30SETREG16(cpu->tmpw);
}

// 04,db: add AL,db	AL += db
void v30_op04(CPU* cpu) {
	cpu->lwr = v30_immb(cpu);
	cpu->regAL = v30_add8(cpu, cpu->regAL, cpu->lwr, 0);
}

// 05,dw: add AW,dw	AW += dw
void v30_op05(CPU* cpu) {
	cpu->twrd = v30_immw(cpu);
	cpu->regAW = v30_add16(cpu, cpu->regAW, cpu->twrd, 0);
}

// 06: push ds1
void v30_op06(CPU* cpu) {
	v30_push(cpu, cpu->regDS1);
}

// 07: pop es
void v30_op07(CPU* cpu) {
	cpu->regDS1 = v30_pop(cpu);
}

// or

// 08,mod: or eb,rb
void v30_op08(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_or8(cpu, cpu->ltw, cpu->lwr);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// 09,mod: or ew,rw
void v30_op09(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_or16(cpu, cpu->tmpw, cpu->twrd);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 0a,mod: or rb,eb
void v30_op0A(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_or8(cpu, cpu->ltw, cpu->lwr);
	V30SETREG8(cpu->ltw);
}

// 0b,mod: or rw,ew
void v30_op0B(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_or16(cpu, cpu->tmpw, cpu->twrd);
	V30SETREG16(cpu->tmpw);
}

// 0c,db: or al,db
void v30_op0C(CPU* cpu) {
	cpu->tmp = v30_immb(cpu);
	cpu->regAL = v30_or8(cpu, cpu->regAL, cpu->tmp);
}

// 0d,dw: or aw,dw
void v30_op0D(CPU* cpu) {
	cpu->twrd = v30_immw(cpu);
	cpu->regAW = v30_or16(cpu, cpu->regAW, cpu->twrd);
}

// 0e: push cs
void v30_op0E(CPU* cpu) {
	v30_push(cpu, cpu->regPS);
}

// 0f: prefix
void v30_op0F(CPU* cpu) {
	cpu->opTab = v30_0f_tab;
}

// 10,mod: addc eb,rb
void v30_op10(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_add8(cpu, cpu->ltw, cpu->lwr, cpu->flgCY);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// 11,mod: addc ew,rw
void v30_op11(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_add16(cpu, cpu->tmpw, cpu->twrd, cpu->flgCY);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 12,mod: addc rb,eb
void v30_op12(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_add8(cpu, cpu->ltw, cpu->lwr, cpu->flgCY);
	V30SETREG8(cpu->ltw);
}

// 13,mod: addc rw,ew
void v30_op13(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_add16(cpu, cpu->tmpw, cpu->twrd, cpu->flgCY);
	V30SETREG16(cpu->tmpw);
}

// 14,db: addc al,db
void v30_op14(CPU* cpu) {
	cpu->lwr = v30_immb(cpu);
	cpu->regAL = v30_add8(cpu, cpu->regAL, cpu->lwr, cpu->flgCY);
}

// 15,dw: addc aw,dw
void v30_op15(CPU* cpu) {
	cpu->twrd = v30_immw(cpu);
	cpu->regAW = v30_add16(cpu, cpu->regAW, cpu->twrd, cpu->flgCY);
}

// 16: push ss
void v30_op16(CPU* cpu) {
	v30_push(cpu, cpu->regSS);
}

// 17: pop ss
void v30_op17(CPU* cpu) {
	cpu->regSS = v30_pop(cpu);
}

// sub/subc

// 18,mod: subc eb,rb	NOTE: subc
void v30_op18(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_sub8(cpu, cpu->ltw, cpu->lwr, cpu->flgCY);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// 19,mod: subc ew,rw
void v30_op19(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_sub16(cpu, cpu->tmpw, cpu->twrd, cpu->flgCY);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 1a,mod: subc rb,eb
void v30_op1A(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_sub8(cpu, cpu->lwr, cpu->ltw, cpu->flgCY);
	V30SETREG8(cpu->ltw);
}

// 1b,mod: subc rw,ew
void v30_op1B(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_sub16(cpu, cpu->twrd, cpu->tmpw, cpu->flgCY);
	V30SETREG16(cpu->tmpw);
}

// 1c,db: subc al,db
void v30_op1C(CPU* cpu) {
	cpu->lwr = v30_immb(cpu);
	cpu->regAL = v30_sub8(cpu, cpu->regAL, cpu->lwr, cpu->flgCY);
}

// 1d,dw: subc aw,dw
void v30_op1D(CPU* cpu) {
	cpu->twrd = v30_immw(cpu);
	cpu->regAW = v30_sub16(cpu, cpu->regAW, cpu->twrd, cpu->flgCY);
}

// 1e: push ds0
void v30_op1E(CPU* cpu) {
	v30_push(cpu, cpu->regDS0);
}

// 1f: pop ds0
void v30_op1F(CPU* cpu) {
	cpu->regDS0 = v30_pop(cpu);
}

// and

// 20,mod: and eb,rb
void v30_op20(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_and8(cpu, cpu->ltw, cpu->lwr);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// 21,mod: and ew,rw
void v30_op21(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_and16(cpu, cpu->tmpw, cpu->twrd);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 22,mod: and rb,eb
void v30_op22(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_and8(cpu, cpu->ltw, cpu->lwr);
	V30SETREG8(cpu->ltw);
}

// 23,mod: and rw,ew
void v30_op23(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_and16(cpu, cpu->tmpw, cpu->twrd);
	V30SETREG16(cpu->tmpw);
}

// 24,db: and al,db
void v30_op24(CPU* cpu) {
	cpu->ltw = v30_immb(cpu);
	cpu->regAL = v30_and8(cpu, cpu->regAL, cpu->ltw);
}

// 25,dw: and aw,dw
void v30_op25(CPU* cpu) {
	cpu->twrd = v30_immw(cpu);
	cpu->regAW = v30_and16(cpu, cpu->regAW, cpu->twrd);
}

// 26: ES (DS1) segment override prefix
void v30_op26(CPU* cpu) {
	cpu->regSEG = cpu->regDS1;
	cpu->flgSOVR = 1;
}

// 27: daa
void v30_op27(CPU* cpu) {
	if (((cpu->regAL & 0x0f) > 9) || cpu->flgAC) {
		cpu->regAL += 6;
		cpu->flgAC = 1;
	} else {
		cpu->flgAC = 0;
	}
	if ((cpu->regAL > 0x9f) || cpu->flgCY) {
		cpu->regAL += 0x60;
		cpu->flgCY = 1;
	} else {
		cpu->flgCY = 0;
	}
	cpu->flgS = !!(cpu->regAL & 0x80);
	cpu->flgZ = !cpu->regAL;
	cpu->flgP = parity(cpu->regAL);
}

// 28,mod: sub eb,rb
void v30_op28(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_sub8(cpu, cpu->ltw, cpu->lwr, 0);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// 29,mod: sub ew,rw
void v30_op29(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_sub16(cpu, cpu->tmpw, cpu->twrd, 0);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 2a,mod: sub rb,eb
void v30_op2A(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_sub8(cpu, cpu->lwr, cpu->ltw, 0);
	V30SETREG8(cpu->ltw);
}

// 2b,mod: sub rw,ew
void v30_op2B(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_sub16(cpu, cpu->twrd, cpu->tmpw, 0);
	V30SETREG16(cpu->tmpw);
}

// 2c,db: sub al,db
void v30_op2C(CPU* cpu) {
	cpu->ltw = v30_immb(cpu);
	cpu->regAL =  v30_sub8(cpu, cpu->regAL, cpu->ltw, 0);
}

// 2d,dw: sub aw,dw
void v30_op2D(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	cpu->regAW =  v30_sub16(cpu, cpu->regAW, cpu->tmpw, 0);
}

// 2e: CS segment override prefix
void v30_op2E(CPU* cpu) {
	cpu->regSEG = cpu->regPS;
	cpu->flgSOVR = 1;
}

// 2f: das
void v30_op2F(CPU* cpu) {
	if (((cpu->regAL & 15) > 9) || cpu->flgAC) {
		cpu->regAL -= 9;
		cpu->flgAC = 1;
	} else {
		cpu->flgAC = 0;
	}
	if ((cpu->regAL > 0x9f) || cpu->flgCY) {
		cpu->regAL -= 0x60;
		cpu->flgCY = 1;
	} else {
		cpu->flgCY = 0;
	}
	cpu->flgCY = !!(cpu->regAL & 0x80);
	cpu->flgZ = !cpu->regAL;
	cpu->flgP = parity(cpu->regAL);
}

// xor

// 30,mod: xor eb,rb
void v30_op30(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_xor8(cpu, cpu->ltw, cpu->lwr);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// 31,mod: xor ew,rw
void v30_op31(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_xor16(cpu, cpu->tmpw, cpu->twrd);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 32,mod: xor rb,eb
void v30_op32(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->ltw = v30_xor8(cpu, cpu->ltw, cpu->lwr);
	V30SETREG8(cpu->ltw);
}

// 33,mod: xor rw,ew
void v30_op33(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_xor16(cpu, cpu->tmpw, cpu->twrd);
	V30SETREG16(cpu->tmpw);
}

// 34,db: xor al,db
void v30_op34(CPU* cpu) {
	cpu->ltw = v30_immb(cpu);
	cpu->regAL = v30_xor8(cpu, cpu->regAL, cpu->ltw);
}

// 35,dw: xor aw,dw
void v30_op35(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	cpu->regAW = v30_xor16(cpu, cpu->regAW, cpu->tmpw);
}

// 36: SS segment override prefix
void v30_op36(CPU* cpu) {
	cpu->regSEG = cpu->regSS;
	cpu->flgSOVR = 1;
}

// 37: aaa
void v30_op37(CPU* cpu) {
	if (((cpu->regAL & 0x0f) > 0x09) || cpu->flgAC) {
		cpu->regAL += 6;
		cpu->regAH++;
		cpu->flgAC = 1;
		cpu->flgCY = 1;
	} else {
		cpu->flgAC = 0;
		cpu->flgCY = 0;
	}
}

// 38,mod: cmp eb,rb
void v30_op38(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->tmp = v30_sub8(cpu, cpu->ltw, cpu->lwr, 0);
}

// 39,mod: cmp ew,rw
void v30_op39(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_sub16(cpu, cpu->tmpw, cpu->twrd, 0);
}

// 3a,mod: cmp rb,eb
void v30_op3A(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->tmp = v30_sub8(cpu, cpu->lwr, cpu->ltw, 0);
}

// 3b,mod: cmp rw,ew
void v30_op3B(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_sub16(cpu, cpu->twrd, cpu->tmpw, 0);
}

// 3c,db: cmp al,db
void v30_op3C(CPU* cpu) {
	cpu->ltw = v30_immb(cpu);
	cpu->ltw = v30_sub8(cpu, cpu->regAL, cpu->ltw, 0);
}

// 3d,dw: cmp aw,dw
void v30_op3D(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	cpu->tmpw = v30_sub16(cpu, cpu->regAW, cpu->tmpw, 0);
}

// 3e: DS0 segment override prefix
void v30_op3E(CPU* cpu) {
	cpu->regSEG = cpu->regDS0;
	cpu->flgSOVR = 1;
}

// 3f: adjbs
void v30_op3F(CPU* cpu) {
	if (((cpu->regAL & 15) > 9) | cpu->flgAC) {
		cpu->regAL -= 6;
		cpu->regAH--;
		cpu->flgAC = 1;
		cpu->flgCY = 1;
	} else {
		cpu->flgAC = 0;
		cpu->flgCY = 0;
	}
	cpu->regAL &= 15;
}

// inc

unsigned char v30_inc8(CPU* cpu, unsigned char r) {
	r++;
	cpu->flgV = !!(r == 0x80);
	cpu->flgS = !!(r & 0x80);
	cpu->flgZ = !r;
	cpu->flgAC = !!((r & 15) == 0);	// ? 0fff
	cpu->flgP = parity(r & 0xff);
	return r;
}

unsigned short v30_inc16(CPU* cpu, unsigned short r) {
	r++;
	cpu->flgV = !!(r == 0x8000);
	cpu->flgS = !!(r & 0x8000);
	cpu->flgZ = !r;
	cpu->flgAC = !!((r & 15) == 0);	// ? 0fff
	cpu->flgP = parity(r & 0xff);
	return r;
}

// 40: inc aw
void v30_op40(CPU* cpu) {
	cpu->regAW = v30_inc16(cpu, cpu->regAW);
}

// 41: inc cw
void v30_op41(CPU* cpu) {
	cpu->regCW = v30_inc16(cpu, cpu->regCW);
}

// 42: inc dw
void v30_op42(CPU* cpu) {
	cpu->regDW = v30_inc16(cpu, cpu->regDW);
}

// 43: inc bw
void v30_op43(CPU* cpu) {
	cpu->regBW = v30_inc16(cpu, cpu->regBW);
}

// 44: inc sp
void v30_op44(CPU* cpu) {
	cpu->regSP = v30_inc16(cpu, cpu->regSP);
}

// 45: inc bp
void v30_op45(CPU* cpu) {
	cpu->regBP = v30_inc16(cpu, cpu->regBP);
}

// 46: inc ix
void v30_op46(CPU* cpu) {
	cpu->regIX = v30_inc16(cpu, cpu->regIX);
}

// 47: inc iy
void v30_op47(CPU* cpu) {
	cpu->regIY = v30_inc16(cpu, cpu->regIY);
}

// dec

unsigned char v30_dec8(CPU* cpu, unsigned char r) {
	r--;
	cpu->flgV = !!(r == 0x7f);
	cpu->flgS = !!(r & 0x80);
	cpu->flgZ = !r;
	cpu->flgAC = !!((r & 15) == 15);
	cpu->flgP = parity(r & 0xff);
	return r;
}

unsigned short v30_dec16(CPU* cpu, unsigned short r) {
	r--;
	cpu->flgV = !!(r == 0x7fff);
	cpu->flgS = !!(r & 0x8000);
	cpu->flgZ = !r;
	cpu->flgAC = !!((r & 15) == 15);
	cpu->flgP = parity(r & 0xff);
	return r;
}

// 48: dec aw
void v30_op48(CPU* cpu) {
	cpu->regAW = v30_dec16(cpu, cpu->regAW);
}

// 49: dec cw
void v30_op49(CPU* cpu) {
	cpu->regCW = v30_dec16(cpu, cpu->regCW);
}

// 4a: dec dw
void v30_op4A(CPU* cpu) {
	cpu->regDW = v30_dec16(cpu, cpu->regDW);
}

// 4b: dec bw
void v30_op4B(CPU* cpu) {
	cpu->regBW = v30_dec16(cpu, cpu->regBW);
}

// 4c: dec sp
void v30_op4C(CPU* cpu) {
	cpu->regSP = v30_dec16(cpu, cpu->regSP);
}

// 4d: dec bp
void v30_op4D(CPU* cpu) {
	cpu->regBP = v30_dec16(cpu, cpu->regBP);
}

// 4e: dec ix
void v30_op4E(CPU* cpu) {
	cpu->regIX = v30_dec16(cpu, cpu->regIX);
}

// 4f: dec iy
void v30_op4F(CPU* cpu) {
	cpu->regIY = v30_dec16(cpu, cpu->regIY);
}

// 50: push aw
void v30_op50(CPU* cpu) {
	v30_push(cpu, cpu->regAW);
}

// 51: push cw
void v30_op51(CPU* cpu) {
	v30_push(cpu, cpu->regCW);
}

// 52: push dw
void v30_op52(CPU* cpu) {
	v30_push(cpu, cpu->regDW);
}

// 53: push bw
void v30_op53(CPU* cpu) {
	v30_push(cpu, cpu->regBW);
}

// 54: push sp
void v30_op54(CPU* cpu) {
	v30_push(cpu, cpu->regSP);
}

// 55: push bp
void v30_op55(CPU* cpu) {
	v30_push(cpu, cpu->regBP);
}

// 56: push ix
void v30_op56(CPU* cpu) {
	v30_push(cpu, cpu->regIX);
}

// 57: push iy
void v30_op57(CPU* cpu) {
	v30_push(cpu, cpu->regIY);
}

// 58: pop aw
void v30_op58(CPU* cpu) {
	cpu->regAW = v30_pop(cpu);
}

// 59: pop cw
void v30_op59(CPU* cpu) {
	cpu->regCW = v30_pop(cpu);
}

// 5a: pop dw
void v30_op5A(CPU* cpu) {
	cpu->regDW = v30_pop(cpu);
}

// 5b: pop bw
void v30_op5B(CPU* cpu) {
	cpu->regBW = v30_pop(cpu);
}

// 5c: pop sp
void v30_op5C(CPU* cpu) {
	cpu->regSP = v30_pop(cpu);
}

// 5d: pop bp
void v30_op5D(CPU* cpu) {
	cpu->regBP = v30_pop(cpu);
}

// 5e: pop ix
void v30_op5E(CPU* cpu) {
	cpu->regIX = v30_pop(cpu);
}

// 5f: pop iy
void v30_op5F(CPU* cpu) {
	cpu->regIY = v30_pop(cpu);
}

// 60: pusha	(push aw,cw,dw,bw,orig.sp,bp,ix,iy)
void v30_op60(CPU* cpu) {
	cpu->tmpw = cpu->regSP;
	v30_push(cpu, cpu->regAW);
	v30_push(cpu, cpu->regCW);
	v30_push(cpu, cpu->regDW);
	v30_push(cpu, cpu->regBW);
	v30_push(cpu, cpu->tmpw);
	v30_push(cpu, cpu->regBP);
	v30_push(cpu, cpu->regIX);
	v30_push(cpu, cpu->regIY);
}

// 61: popa	(pop iy,ix,bp,(ignore sp),bw,dw,cw,aw)
void v30_op61(CPU* cpu) {
	cpu->regIY = v30_pop(cpu);
	cpu->regIX = v30_pop(cpu);
	cpu->regBP = v30_pop(cpu);
	cpu->tmpw = v30_pop(cpu);
	cpu->regBW = v30_pop(cpu);
	cpu->regDW = v30_pop(cpu);
	cpu->regCW = v30_pop(cpu);
	cpu->regAW = v30_pop(cpu);
}

// 62,mod: bound rw,md		@eff.addr (md): 2words = min,maw. check if min<=rw<=maw, INT5 if not
void v30_op62(CPU* cpu) {
	v30_rd_ea(cpu, 1);	// twrd=rw, tmpw=min
	if (cpu->ea.reg) {	// interrupts. TODO: fix for protected mode
		v30_undef(cpu);
	} else if ((signed short)cpu->twrd < (signed short)cpu->tmpw) {	// not in bounds: INT5
		v30_exception(cpu, V30_INT_BR, 0);
	} else {
		int seg = cpu->ea.seg.idx;
		if (cpu->flgSOVR) seg = cpu->regSEG;
		cpu->tmpw = v30_mrdw(cpu, seg, cpu->ea.adr + 2);
		// cpu->htw = v30_mrdb(cpu, seg, cpu->ea.adr + 3);
		if ((signed short)cpu->twrd > (signed short)cpu->tmpw) {
			v30_exception(cpu, V30_INT_BR, 0);
		}
	}
}

// 63-67 is 286+
#if 0

// 63,mod: arpl ew,rw		adjust RPL of EW not less than RPL of RW
void i286_op63(CPU* cpu) {
	v30_exception(cpu, V30_INT_UD, 0);	// real mode
	// TODO: protected mode
}
#endif

// 64: repeat next cmps/scas cw times or cf=1
void v30_op64(CPU* cpu) {
	cpu->regREP = V30_REPNC;
}

// 65: repeat next cmps/scas cw times or cf=0
void v30_op65(CPU* cpu) {
	cpu->regREP = V30_REPC;
}

#if 0
// 66: operand size override prefix
void i286_op66(CPU* cpu) {}

// 67: address size override prefix
void i286_op67(CPU* cpu) {}

#endif

// 68: push wrd
void v30_op68(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	v30_push(cpu, cpu->tmpw);
}

// 69,mod,dw: imul rw,ea,dw: rw = ea.w * wrd
void v30_op69(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->twrd = v30_immw(cpu);
	cpu->tmpi = v30_smul(cpu, cpu->tmpw, cpu->twrd);
	V30SETREG16(cpu->tmpi & 0xffff);
}

// 6a,db: push byte (sign extended to word)
void v30_op6A(CPU* cpu) {
	cpu->ltw = v30_immb(cpu);
	cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
	v30_push(cpu, cpu->tmpw);
}

// 6b,mod,db: imul rw,ea,db: rw = ea.w * db
void v30_op6B(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->lwr = v30_immb(cpu);
	cpu->hwr = (cpu->lwr & 0x80) ? 0xff : 0x00;
	cpu->tmpi = v30_smul(cpu, cpu->tmpw, cpu->twrd);
	V30SETREG16(cpu->tmpi & 0xffff);
}

// rep opcode template
// note: repz/repnz both working as rep

void v30_rep(CPU* cpu, cbcpu foo) {
	if (cpu->regREP == V30_REP_NONE) {
		foo(cpu);
	} else {
		if (cpu->regCW) {
			foo(cpu);
			cpu->regCW--;
			if (cpu->regCW)			// don't do last dummy cycle (cw=0)
				cpu->regPC = cpu->oldpc;
		}
	}
}

// 6c: insb: [es:di] = in dx;
void v30_6c_cb(CPU* cpu) {
	cpu->tmp = cpu_ird(cpu, cpu->regDW);
	v30_mwrb(cpu, cpu->regDS1, cpu->regIY, cpu->tmp);	// no segment override
	cpu->regIY += cpu->flgDIR ? -1 : 1;
}
void v30_op6C(CPU* cpu) {v30_rep(cpu, v30_6c_cb);}

// 6d: insw: word [es:di] = in dx;
void v30_6d_cb(CPU* cpu) {
	cpu->htw = cpu_ird(cpu, cpu->regDW + 1);
	cpu->ltw = cpu_ird(cpu, cpu->regDW);
	v30_mwrw(cpu, cpu->regDS1, cpu->regIY, cpu->tmpw);
	cpu->regIY += cpu->flgDIR ? -2 : 2;
}
void v30_op6D(CPU* cpu) {
	v30_rep(cpu, v30_6d_cb);
}

// 6e: outsb: out (dx),word [ds:si]
void v30_6e_cb(CPU* cpu) {
	int seg = cpu->regDS0;
	if (cpu->flgSOVR) seg = cpu->regSEG;
	cpu->tmp = v30_mrdb(cpu, seg, cpu->regIX);
	cpu_iwr(cpu, cpu->regDW, cpu->tmp); //, 0);
	cpu->regIX += cpu->flgDIR ? -1 : 1;
}
void v30_op6E(CPU* cpu) {v30_rep(cpu, v30_6e_cb);}

// 6f: outs dx,wrd
void v30_6f_cb(CPU* cpu) {
	int seg = cpu->regDS0;
	if (cpu->flgSOVR) seg = cpu->regSEG;
	cpu->tmpw = v30_mrdw(cpu, seg, cpu->regIX);
	cpu_iwr(cpu, cpu->regDW + 1, cpu->htw);
	cpu_iwr(cpu, cpu->regDW, cpu->ltw);
	cpu->regIX += cpu->flgDIR ? -2 : 2;
}
void v30_op6F(CPU* cpu) {
	if (cpu->regIX == 0xffff) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		v30_rep(cpu, v30_6f_cb);
	}
}

// cond jump
void v30_jr(CPU* cpu, int cnd) {
	cpu->ltw = v30_immb(cpu);
	cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
	if (cnd) {
		cpu->regPC += cpu->tmpw;
		cpu->t += 4;
	}
}

// 70: jo cb
void v30_op70(CPU* cpu) {v30_jr(cpu, cpu->flgV);}
// 71: jno cb
void v30_op71(CPU* cpu) {v30_jr(cpu, !cpu->flgV);}
// 72: jc cb (aka jb,jnae)
void v30_op72(CPU* cpu) {v30_jr(cpu, cpu->flgCY);}
// 73: jnc cb (aka jnb,jae)
void v30_op73(CPU* cpu) {v30_jr(cpu, !cpu->flgCY);}
// 74: jz cb (aka je)
void v30_op74(CPU* cpu) {v30_jr(cpu, cpu->flgZ);}
// 75: jnz cb (aka jne)
void v30_op75(CPU* cpu) {v30_jr(cpu, !cpu->flgZ);}
// 76: jbe cb (aka jna): CF=1 || Z=1
void v30_op76(CPU* cpu) {v30_jr(cpu, cpu->flgCY || cpu->flgZ);}
// 77: ja cb (aka jnbe): CF=0 && Z=0
void v30_op77(CPU* cpu) {v30_jr(cpu, !cpu->flgCY && !cpu->flgZ);}
// 78: js cb
void v30_op78(CPU* cpu) {v30_jr(cpu, cpu->flgS);}
// 79: jns cb
void v30_op79(CPU* cpu) {v30_jr(cpu, !cpu->flgS);}
// 7a: jp cb (aka jpe)
void v30_op7A(CPU* cpu) {v30_jr(cpu, cpu->flgP);}
// 7b: jnp cb (aka jpo)
void v30_op7B(CPU* cpu) {v30_jr(cpu, !cpu->flgP);}
// 7c: jl cb (aka jngl) FS!=FO
void v30_op7C(CPU* cpu) {v30_jr(cpu, cpu->flgS ^ cpu->flgV);}
// 7d: jnl cb (aka jgl) FS==FO
void v30_op7D(CPU* cpu) {v30_jr(cpu, !(cpu->flgS ^ cpu->flgV));}
// 7e: jle cb (aka jng) (FZ=1)||(FS!=FO)
void v30_op7E(CPU* cpu) {v30_jr(cpu, cpu->flgZ || (cpu->flgS ^ cpu->flgV));}
// 7f: jnle cb (aka jg) (FZ=0)&&(FS=FO)
void v30_op7F(CPU* cpu) {v30_jr(cpu, !cpu->flgZ && !(cpu->flgS ^ cpu->flgV));}

// 80: ALU eb,byte
void v30_op80(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->tmpb = v30_immb(cpu);
	switch((cpu->regMOD >> 3) & 7) {
		case 0: cpu->tmpb = v30_add8(cpu, cpu->ltw, cpu->tmpb, 0); break;		// add
		case 1: cpu->tmpb = v30_or8(cpu, cpu->ltw, cpu->tmpb); break;			// or
		case 2: cpu->tmpb = v30_add8(cpu, cpu->ltw, cpu->tmpb, cpu->flgCY);		// addc
			break;
		case 3: cpu->tmpb = v30_sub8(cpu, cpu->ltw, cpu->tmpb, cpu->flgCY);		// subc
			break;
		case 4: cpu->tmpb = v30_and8(cpu, cpu->ltw, cpu->tmpb); break;			// and
		case 5:										// sub
		case 7: cpu->tmpb = v30_sub8(cpu, cpu->ltw, cpu->tmpb, 0); break;		// cmp
		case 6: cpu->tmpb = v30_xor8(cpu, cpu->ltw, cpu->tmpb); break;			// xor
	}
	if ((cpu->regMOD & 0x38) != 0x38)		// CMP drop result of SUB
		v30_wr_ea(cpu, cpu->tmpb, 0);
}

void v30_alu16(CPU* cpu) {
	switch((cpu->regMOD >> 3) & 7) {
		case 0: cpu->twrd = v30_add16(cpu, cpu->tmpw, cpu->twrd, 0); break;
		case 1: cpu->twrd = v30_or16(cpu, cpu->tmpw, cpu->twrd); break;
		case 2: cpu->twrd = v30_add16(cpu, cpu->tmpw, cpu->twrd, cpu->flgCY);
			break;
		case 3: cpu->twrd = v30_sub16(cpu, cpu->tmpw, cpu->twrd, cpu->flgCY);
			break;
		case 4: cpu->twrd = v30_and16(cpu, cpu->tmpw, cpu->twrd); break;
		case 5:
		case 7: cpu->twrd = v30_sub16(cpu, cpu->tmpw, cpu->twrd, 0); break;
		case 6: cpu->twrd = v30_xor16(cpu, cpu->tmpw, cpu->twrd); break;
	}
	if ((cpu->regMOD & 0x38) != 0x38)		// cmp
		v30_wr_ea(cpu, cpu->twrd, 1);
}

// 81: ALU ew,word
void v30_op81(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->twrd = v30_immw(cpu);
	v30_alu16(cpu);
}

// 82: ALU eb,byte (==80)
void v30_op82(CPU* cpu) {
	v30_op80(cpu);
}

// 83: ALU ew,signed.byte
void v30_op83(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->lwr = v30_immb(cpu);
	cpu->hwr = (cpu->lwr & 0x80) ? 0xff : 0x00;
	v30_alu16(cpu);
}

// 84,mod: test eb,rb = and w/o storing result
void v30_op84(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->tmpb = v30_and8(cpu, cpu->ltw, cpu->lwr);
}

// 85,mod: test ew,rw
void v30_op85(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpw = v30_and16(cpu, cpu->tmpw, cpu->twrd);
}

// 86,mod: xchg eb,rb = swap values
void v30_op86(CPU* cpu) {
	v30_rd_ea(cpu, 0);	// tmpw=ea, twrd=reg
	v30_wr_ea(cpu, cpu->lwr, 0);
	V30SETREG8(cpu->ltw);
}

// 87,mod: xchg ew,rw
void v30_op87(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	v30_wr_ea(cpu, cpu->twrd, 1);
	V30SETREG16(cpu->tmpw);
}

// 88,mod: mov eb,rb
void v30_op88(CPU* cpu) {
	v30_get_ea(cpu, 0);
	cpu->lwr = v30_get_reg8(cpu, (cpu->regMOD >> 3) & 7);
	v30_wr_ea(cpu, cpu->lwr, 0);
}

// 89,mod: mov ew,rw
void v30_op89(CPU* cpu) {
	v30_get_ea(cpu, 1);
	cpu->twrd = v30_get_reg16(cpu, (cpu->regMOD >> 3) & 7);
	v30_wr_ea(cpu, cpu->twrd, 1);
}

// 8a,mod: mov rb,eb
void v30_op8A(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	V30SETREG8(cpu->ltw);
}

// 8b,mod: mov rw,ew
void v30_op8B(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	V30SETREG16(cpu->tmpw);
}

// 8c,mod: mov ew,[es,cs,ss,ds]	TODO: ignore N.bit2?
void v30_op8C(CPU* cpu) {
	v30_get_ea(cpu, 1);
	switch((cpu->regMOD & 0x18) >> 3) {
		case 0: cpu->twrd = cpu->regDS1; break;
		case 1: cpu->twrd = cpu->regPS; break;
		case 2: cpu->twrd = cpu->regSS; break;
		case 3: cpu->twrd = cpu->regDS0; break;
	}
	v30_wr_ea(cpu, cpu->twrd, 1);
}

// 8d : lea rw,ea	rw = ea.offset
void v30_op8D(CPU* cpu) {
	v30_get_ea(cpu, 0);
	if (cpu->ea.reg) {	// 2nd operand is register
		v30_undef(cpu);
	} else {
		V30SETREG16(cpu->ea.adr);
	}
}

// 8e,mod: mov [es,not cs,ss,ds],ew
void v30_op8E(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	switch((cpu->regMOD & 0x38) >> 3) {
		case 0:	cpu->regDS1 = cpu->tmpw; break;
		case 1: cpu->regPS = cpu->tmpw; break;		// valid for V30
		case 2: cpu->regSS = cpu->tmpw; break;
		case 3: cpu->regDS0 = cpu->tmpw; break;
	}
}

// 8f,mod: pop ew
// not supported by V30
void v30_op8F(CPU* cpu) {
	v30_get_ea(cpu, 0);
	cpu->tmpw = v30_pop(cpu);
	if (cpu->ea.reg) {
		V30SETREG16(cpu->tmpw);
	} else {
		v30_wr_ea(cpu, cpu->tmpw, 1);
	}
}

// 90: nop = xchg aw,aw
void v30_op90(CPU* cpu) {}

// 91: xchg aw,cw
void v30_op91(CPU* cpu) {
	cpu->tmpw = cpu->regAW;
	cpu->regAW = cpu->regCW;
	cpu->regCW = cpu->tmpw;
}

// 92: xchg aw,dw
void v30_op92(CPU* cpu) {
	cpu->tmpw = cpu->regAW;
	cpu->regAW = cpu->regDW;
	cpu->regDW = cpu->tmpw;
}

// 93: xchg aw,bw
void v30_op93(CPU* cpu) {
	cpu->tmpw = cpu->regAW;
	cpu->regAW = cpu->regBW;
	cpu->regBW = cpu->tmpw;
}

// 94: xchg aw,sp
void v30_op94(CPU* cpu) {
	cpu->tmpw = cpu->regAW;
	cpu->regAW = cpu->regSP;
	cpu->regSP = cpu->tmpw;
}

// 95:xchg aw,bp
void v30_op95(CPU* cpu) {
	cpu->tmpw = cpu->regAW;
	cpu->regAW = cpu->regBP;
	cpu->regBP = cpu->tmpw;
}

// 96:xchg aw,ix
void v30_op96(CPU* cpu) {
	cpu->tmpw = cpu->regAW;
	cpu->regAW = cpu->regIX;
	cpu->regIX = cpu->tmpw;
}

// 97:xchg aw,iy
void v30_op97(CPU* cpu) {
	cpu->tmpw = cpu->regAW;
	cpu->regAW = cpu->regIY;
	cpu->regIY = cpu->tmpw;
}

// 98:cbw : sign extend AL to aw
void v30_op98(CPU* cpu) {
	cpu->regAH = (cpu->regAL & 0x80) ? 0xff : 0x00;
}

// 99:cwd : sign extend aw to DX:aw
void v30_op99(CPU* cpu) {
	cpu->regDW = (cpu->regAH & 0x80) ? 0xffff : 0x0000;
}

// callf to ncs:nip
void v30_callf(CPU* cpu, int nip, int ncs) {
	v30_push(cpu, cpu->cs.idx);
	v30_push(cpu, cpu->regPC);
	v30_set_ps(cpu, ncs);
	cpu->regPC = nip;
	cpu->t = 41;
}

// 9a: callf cd (cd=SEG:ADR)
void v30_op9A(CPU* cpu) {
	unsigned short nip = v30_immw(cpu);		// offset
	unsigned short ncs = v30_immw(cpu);		// segment
	v30_callf(cpu, nip, ncs);
}

// 9b: wait
void v30_op9B(CPU* cpu) {
	// wait for busy=0
}

// 9c: push psw
void v30_op9C(CPU* cpu) {
	v30_push(cpu, v30_getflag(cpu));
}

// 9d: pop psw
void v30_op9D(CPU* cpu) {
	cpu->tmpw = v30_pop(cpu);
	cpu->tmpw |= 0xf000;
	v30_setflag(cpu, cpu->tmpw);
}

// 9e: mov psw,ah
void v30_op9E(CPU* cpu) {
	int f = v30_getflag(cpu);
	f &= ~0xd5; //(I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
	f |= cpu->regAH & 0xd5; //(I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
	v30_setflag(cpu, f);
}

// 9f: lahf
void v30_op9F(CPU* cpu) {
	int f = v30_getflag(cpu);
	cpu->regAH &= ~0xd5; //(I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
	cpu->regAH |= f & 0xd5; //(I286_FS | I286_FZ | I286_FA | I286_FP | I286_FC);
}

// a0,iw: mov al,[*ds:iw]
void v30_opA0(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	cpu->regAL = v30_mrdb(cpu, seg, cpu->tmpw);
}

// a1,iw: mov aw,[*ds:iw]
void v30_opA1(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	if (cpu->tmpw == 0xffff) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
		cpu->regAW = v30_mrdw(cpu, seg, cpu->tmpw);
	}
}

// a2,xb: mov [iw],al
void v30_opA2(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	v30_mwrb(cpu, seg, cpu->tmpw, cpu->regAL);
}

// a3,xw: mov [iw],aw
void v30_opA3(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	if (cpu->tmpw == 0xffff) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
		v30_mwrw(cpu, seg, cpu->tmpw, cpu->regAL);
	}
}

// a4: movsb: [*ds:si]->[es:di], si,di ++ or --
void v30_a4_cb(CPU* cpu) {
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	cpu->tmp = v30_mrdb(cpu, seg, cpu->regIX);
	v30_mwrb(cpu, cpu->regDS1, cpu->regIY, cpu->tmp);
	if (cpu->flgDIR) {
		cpu->regIX--;
		cpu->regIY--;
	} else {
		cpu->regIX++;
		cpu->regIY++;
	}
}
void v30_opA4(CPU* cpu) {v30_rep(cpu, v30_a4_cb);}

// a5: movsw [*ds:si]->[es:di] by word, si,di +/-= 2
void v30_a5_cb(CPU* cpu) {
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	cpu->tmpw = v30_mrdw(cpu, seg, cpu->regIX);
	v30_mwrw(cpu, cpu->regDS1, cpu->regIY, cpu->tmpw);
	if (cpu->flgDIR) {
		cpu->regIX -= 2;
		cpu->regIY -= 2;
	} else {
		cpu->regIX += 2;
		cpu->regIY += 2;
	}
}
void v30_opA5(CPU* cpu) {
	if ((cpu->regIX == 0xffff) || (cpu->regIY == 0xffff)) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		v30_rep(cpu, v30_a5_cb);
	}
}

// check condition for repz/repnz/repc/repnc for cmps/scas opcodes
void v30_rep_fz(CPU* cpu, cbcpu foo) {
	if (cpu->regREP == V30_REP_NONE) {
		foo(cpu);
	} else if (cpu->regCW) {
		cpu->regCW--;
		foo(cpu);
		int cond = 0;
		switch (cpu->regREP) {
			case V30_REPZ: cond = (cpu->flgZ && cpu->regCW); break;
			case V30_REPNZ: cond = (!cpu->flgZ && cpu->regCW); break;
			case V30_REPC: cond = (cpu->flgCY && cpu->regCW); break;
			case V30_REPNC: cond = (!cpu->flgCY && cpu->regCW); break;
		}
		if (cond) {
			cpu->regPC = cpu->oldpc;
		}
	}
}

// a6: cmpsb: cmp [*ds:si]-[es:di], adv si,di
void v30_a6_cb(CPU* cpu) {
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	cpu->ltw = v30_mrdb(cpu, seg, cpu->regIX);
	cpu->lwr = v30_mrdb(cpu, cpu->regDS1, cpu->regIY);
	cpu->htw = v30_sub8(cpu, cpu->ltw, cpu->lwr, 0);
	if (cpu->flgDIR) {
		cpu->regIX--;
		cpu->regIY--;
	} else {
		cpu->regIX++;
		cpu->regIY++;
	}
}
void v30_opA6(CPU* cpu) {
	v30_rep_fz(cpu, v30_a6_cb);
}

// a7: cmpsw
void v30_a7_cb(CPU* cpu) {
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	cpu->tmpw = v30_mrdw(cpu, seg, cpu->regIX);
	cpu->twrd = v30_mrdw(cpu, cpu->regDS1, cpu->regIY);
	cpu->tmpw = v30_sub16(cpu, cpu->tmpw, cpu->twrd, 0);
	if (cpu->flgDIR) {
		cpu->regIX -= 2;
		cpu->regIY -= 2;
	} else {
		cpu->regIX += 2;
		cpu->regIY += 2;
	}
}
void v30_opA7(CPU* cpu) {
	if ((cpu->regIX == 0xffff) || (cpu->regIY == 0xffff)) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		v30_rep_fz(cpu, v30_a7_cb);
	}
}

// a8,byte: test al,byte
void v30_opA8(CPU* cpu) {
	cpu->ltw = v30_immb(cpu);
	cpu->lwr = v30_and8(cpu, cpu->regAL, cpu->ltw);
}

// a9,wrd: test aw,wrd
void v30_opA9(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	cpu->twrd = v30_and16(cpu, cpu->regAW, cpu->tmpw);
}

// aa: stosb  al->[es:di], adv di
void v30_aa_cb(CPU* cpu) {
	v30_mwrb(cpu, cpu->regDS1, cpu->regIY, cpu->regAL);
	cpu->regIY += cpu->flgDIR ? -1 : 1;
}
void v30_opAA(CPU* cpu) {v30_rep(cpu, v30_aa_cb);}

// ab: stosw: aw->[es:di], adv di
void v30_ab_cb(CPU* cpu) {
	v30_mwrw(cpu, cpu->regDS1, cpu->regIY, cpu->regAW);
	cpu->regIY += cpu->flgDIR ? -2 : 2;
}
void v30_opAB(CPU* cpu) {
	if (cpu->regIY == 0xffff) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		v30_rep(cpu, v30_ab_cb);
	}
}

// ac: lodsb: [*ds:si]->al, adv si
void v30_opAC(CPU* cpu) {
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	cpu->regAL = v30_mrdb(cpu, seg, cpu->regIX);
	cpu->regIX += cpu->flgDIR ? -1 : 1;
}

// ad: lodsw [*ds:si]->aw, adv si
void v30_opAD(CPU* cpu) {
	if (cpu->regIX == 0xffff) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
		cpu->regAW = v30_mrdw(cpu, seg, cpu->regIX);
		cpu->regIX += cpu->flgDIR ? -2 : 2;
	}
}

// ae: scasb	cmp al,[ds1:iy]
void v30_ae_cb(CPU* cpu) {
	cpu->ltw = v30_mrdb(cpu, cpu->regDS1, cpu->regIY);
	cpu->lwr = v30_sub8(cpu, cpu->regAL, cpu->ltw, 0);
	cpu->regIY += cpu->flgDIR ? -1 : 1;
}
void v30_opAE(CPU* cpu) {
	v30_rep_fz(cpu, v30_ae_cb);
}

// af: scasw	cmp aw,[es:di]
void v30_af_cb(CPU* cpu) {
	cpu->tmpw = v30_mrdw(cpu, cpu->regDS1, cpu->regIY);
	cpu->twrd = v30_sub16(cpu, cpu->regAW, cpu->tmpw, 0);
	cpu->regIX += cpu->flgDIR ? -2 : 2;
}
void v30_opAF(CPU* cpu) {
	if (cpu->regIY == 0xffff) {
		v30_push(cpu, 0);
		v30_exception(cpu, V30_INT_GP, 0);
	} else {
		v30_rep_fz(cpu, v30_af_cb);
	}
}

// b0..b7,ib: mov rb,ib
void v30_opB0(CPU* cpu) {cpu->regAL = v30_immb(cpu);}
void v30_opB1(CPU* cpu) {cpu->regCL = v30_immb(cpu);}
void v30_opB2(CPU* cpu) {cpu->regDL = v30_immb(cpu);}
void v30_opB3(CPU* cpu) {cpu->regBL = v30_immb(cpu);}
void v30_opB4(CPU* cpu) {cpu->regAH = v30_immb(cpu);}
void v30_opB5(CPU* cpu) {cpu->regCH = v30_immb(cpu);}
void v30_opB6(CPU* cpu) {cpu->regDH = v30_immb(cpu);}
void v30_opB7(CPU* cpu) {cpu->regBH = v30_immb(cpu);}

// b8..bf,iw: mov rw,iw
void v30_opB8(CPU* cpu) {cpu->regAW = v30_immw(cpu);}
void v30_opB9(CPU* cpu) {cpu->regCW = v30_immw(cpu);}
void v30_opBA(CPU* cpu) {cpu->regDW = v30_immw(cpu);}
void v30_opBB(CPU* cpu) {cpu->regBW = v30_immw(cpu);}
void v30_opBC(CPU* cpu) {cpu->regSP = v30_immw(cpu);}
void v30_opBD(CPU* cpu) {cpu->regBP = v30_immw(cpu);}
void v30_opBE(CPU* cpu) {cpu->regIX = v30_immw(cpu);}
void v30_opBF(CPU* cpu) {cpu->regIY = v30_immw(cpu);}

// rotate/shift

typedef unsigned char(*cbv30rot8)(CPU*, unsigned char);
typedef unsigned short(*cbv30rot16)(CPU*, unsigned short);

// shl (/4) = sal (/6)
static cbv30rot8 v30_rot8_tab[8] = {
	v30_rol8, v30_ror8, v30_rcl8, v30_rcr8,
	v30_sal8, v30_shr8, v30_sal8, v30_sar8
};

static cbv30rot16 v30_rot16_tab[8] = {
	v30_rol16, v30_ror16, v30_rcl16, v30_rcr16,
	v30_sal16, v30_shr16, v30_sal16, v30_sar16
};

void v30_rotsh8(CPU* cpu, int cnt) {
	// v30_rd_ea already called, cpu->tmpb is number of repeats
	// cpu->ltw = ea.byte, result should be back in cpu->ltw. flags is setted
	cnt &= 0x1f;		// only 5 bits is counted
	cpu->t += cnt;		// 1T for each iteration
	cbv30rot8 foo = v30_rot8_tab[(cpu->regMOD >> 3) & 7];
	if (foo) {
		while (cnt) {
			cpu->ltw = foo(cpu, cpu->ltw);
			cnt--;
		}
	}
}

void v30_rotsh16(CPU* cpu, int cnt) {
	cnt &= 0x1f;
	cpu->t += cpu->tmpb;
	cbv30rot16 foo = v30_rot16_tab[(cpu->regMOD >> 3) & 7];
	if (foo) {
		while (cnt) {
			cpu->tmpw = foo(cpu, cpu->tmpw);
			cnt--;
		}
	}
}

// c0,mod,db: rotate/shift ea byte db times
void v30_opC0(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	cpu->tmpb = v30_immb(cpu);
	v30_rotsh8(cpu, cpu->tmpb);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// c1,mod,db: rotate/shift ea word db times
void v30_opC1(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	cpu->tmpb = v30_immb(cpu);
	v30_rotsh16(cpu, cpu->tmpb);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// c2,iw: ret iw	pop ip, pop iw bytes
void v30_opC2(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	cpu->regPC = v30_pop(cpu);
	cpu->regSP += cpu->tmpw;
}

// c3: ret
void v30_opC3(CPU* cpu) {
	cpu->regPC = v30_pop(cpu);
}

// c4,mod: les rw,ed
void v30_opC4(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->ea.seg.idx;
	cpu->tmpw = v30_mrdw(cpu, seg, cpu->ea.adr + 0);		// address
	cpu->regDS1 = v30_mrdw(cpu, seg, cpu->ea.adr + 2);		// segment
	V30SETREG16(cpu->tmpw);
}

// c5,mod: lds rw,ed (same c4 with ds)
void v30_opC5(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->ea.seg.idx;
	cpu->tmpw = v30_mrdw(cpu, seg, cpu->ea.adr + 0);		// address
	cpu->regDS0 = v30_mrdw(cpu, seg, cpu->ea.adr + 2);		// segment
	V30SETREG16(cpu->tmpw);
}

// c6,mod,ib: mov ea,ib
// TODO:c6/0 only
void v30_opC6(CPU* cpu) {
	v30_get_ea(cpu, 0);
	if (cpu->regMOD & 0x38) {
		v30_undef(cpu);
	} else {
		cpu->tmp = v30_immb(cpu);
		v30_wr_ea(cpu, cpu->tmp, 0);
	}
}

// c7,mod,iw: mov ea,iw
// TODO:c7/0 only
void v30_opC7(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	if (cpu->regMOD & 0x38) {
		v30_undef(cpu);
	} else {
		cpu->twrd = v30_immw(cpu);
		v30_wr_ea(cpu, cpu->twrd, 1);
	}
}

// c8,iw,ib: enter iw,ib
void v30_opC8(CPU* cpu) {
	cpu->t = 11;
	cpu->tmpw = v30_immw(cpu);
	cpu->tmpb = v30_immb(cpu) & 0x1f;
	v30_push(cpu, cpu->regBP);
	cpu->ea.adr = cpu->regSP;
	if (cpu->tmpb > 0) {
		while(--cpu->tmpb) {
			cpu->regBP -= 2;
			int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
			cpu->twrd = v30_mrdw(cpu, seg, cpu->regBP);		// +1T
			v30_push(cpu, cpu->twrd);				// +2T
		}
		v30_push(cpu, cpu->ea.adr);					// +2T (1?)
	}
	cpu->regBP = cpu->ea.adr;
	cpu->regSP -= cpu->tmpw;
}

// c9: leave
void v30_opC9(CPU* cpu) {
	cpu->regSP = cpu->regBP;
	cpu->regBP = v30_pop(cpu);
}

// ca,iw: retf iw	pop ip,cs,iw bytes	15/25/55T
void v30_opCA(CPU* cpu) {
	cpu->twrd = v30_immw(cpu);
	cpu->regPC = v30_pop(cpu);
	v30_set_ps(cpu, v30_pop(cpu));
	cpu->regSP += cpu->twrd;
	cpu->t += cpu->twrd;
}

// cb: retf	pop ip,cs
void v30_opCB(CPU* cpu) {
	cpu->regPC = v30_pop(cpu);
	v30_set_ps(cpu, v30_pop(cpu));
}

void v30_int(CPU*, int);

// cc: int 3
void v30_opCC(CPU* cpu) {
	v30_int(cpu, 3);
}

// cd,ib: int ib
void v30_opCD(CPU* cpu) {
	cpu->tmp = v30_immb(cpu);
	v30_int(cpu, cpu->tmp);
}

// ce: into	int 4 if FO=1
void v30_opCE(CPU* cpu) {
	if (cpu->flgV)
		v30_int(cpu, 4); // I286_INT_OF);
}

// cf: reti	pop pc,ps,psw (MD flag optionaly)
void v30_opCF(CPU* cpu) {
	cpu->regPC = v30_pop(cpu);	// pc
	v30_set_ps(cpu, v30_pop(cpu));
	v30_setflag(cpu, v30_pop(cpu));
	cpu->flgBNMI = 0;
}

// d0,mod: rot/shift ea.byte 1 time
void v30_opD0(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	v30_rotsh8(cpu, 1);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// d1,mod: rot/shift ea.word 1 time
void v30_opD1(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	v30_rotsh16(cpu, 1);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// d2,mod: rot/shift ea.byte CL times
void v30_opD2(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	v30_rotsh8(cpu, cpu->regCL);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

// d3,mod: rot/shift ea.word CL times
void v30_opD3(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	v30_rotsh16(cpu, cpu->regCL);
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// d4 0a: aam ib
void v30_opD4(CPU* cpu) {
	cpu->tmpb = v30_immb(cpu);
	if (cpu->tmpb == 0) {
		v30_exception(cpu, V30_INT_DE, 0);
	} else {
		cpu->regAH = cpu->regAL / cpu->tmpb;
		cpu->regAL = cpu->regAL % cpu->tmpb;
		cpu->flgS = !!(cpu->regAH & 0x80);
		cpu->flgZ = !cpu->regAW;
		cpu->flgP = parity(cpu->regAL);
	}
}

// d5 n: aad ib
// NOTE: v30 ignore n, allways use base 10
void v30_opD5(CPU* cpu) {
	cpu->tmpb = v30_immb(cpu);
	cpu->regAL = cpu->regAH * 10 + cpu->regAL;
	cpu->regAH = 0;
	cpu->flgS = !!(cpu->regAL & 0x80);
	cpu->flgZ = !cpu->regAL;
	cpu->flgP = parity(cpu->regAL);
}

// d6: salc	al = (flag C) ? 0xff : 0x00
void v30_opD6(CPU* cpu) {
	cpu->regAL = cpu->flgCY ? 0xff : 0x00;
}

// d7: xlatb	al = [ds:bw+al]		// segment replacement must work
void v30_opD7(CPU* cpu) {
	cpu->tmpw = cpu->regBW + cpu->regAL;
	int seg = cpu->flgSOVR ? cpu->regSEG : cpu->regDS0;
	cpu->regAL = v30_mrdb(cpu, seg, cpu->tmpw);
}

// 80287 template
// opcodes: D8-DF (11011xxx),mod,[data,[data]]

void v30_fpo1(CPU* cpu) {
	v30_get_ea(cpu, 1);
	v30_exception(cpu, V30_INT_NM, 0);
}

void v30_fpo2(CPU* cpu) {
	v30_get_ea(cpu, 1);
	v30_exception(cpu, V30_INT_NM, 0);
}

// e0,cb: loopnz cb:	cw--,jump short if (cw!=0)&&(fz=0)
void v30_opE0(CPU* cpu) {
	cpu->regCW--;
	v30_jr(cpu, cpu->regCW && !cpu->flgZ);
}

// e1,cb: loopz cb
void v30_opE1(CPU* cpu) {
	cpu->regCW--;
	v30_jr(cpu, cpu->regCW && cpu->flgZ);
}

// e2,cb: loop cb	check only cw
void v30_opE2(CPU* cpu) {
	cpu->regCW--;
	v30_jr(cpu, cpu->regCW);
}

// e3: jcxz cb
void v30_opE3(CPU* cpu) {
	v30_jr(cpu, !cpu->regCW);
}

// e4,ib: in al,ib	al = in(ib)
void v30_opE4(CPU* cpu) {
	cpu->tmpb = v30_immb(cpu);
	cpu->regAL = cpu_ird(cpu, cpu->tmpb);
}

// e5,ib: in aw,ib	aw = in(ib)[x2]
void v30_opE5(CPU* cpu) {
	cpu->tmpb = v30_immb(cpu);
	cpu->regAH = cpu_ird(cpu, cpu->tmpb+1);
	cpu->regAL = cpu_ird(cpu, cpu->tmpb);
}

// e6,ib: out ib,al	out(ib),al
void v30_opE6(CPU* cpu) {
	cpu->tmpb = v30_immb(cpu);
	cpu_iwr(cpu, cpu->tmpb, cpu->regAL);
}

// e7,ib: out ib,aw	out(ib),aw
void v30_opE7(CPU* cpu) {
	cpu->tmpb = v30_immb(cpu);
	cpu_iwr(cpu, cpu->tmpb + 1, cpu->regAH);
	cpu_iwr(cpu, cpu->tmpb, cpu->regAL);
}

// e8,cw: call cw	(relative call)
void v30_opE8(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	v30_push(cpu, cpu->regPC);
	cpu->regPC += cpu->tmpw;
}

// e9,cw: jmp cw	(relative jump)
void v30_opE9(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);
	cpu->regPC += cpu->tmpw;
}

// ea,ow,sw: jmpf ow:sw	(far jump)
void v30_opEA(CPU* cpu) {
	cpu->tmpw = v30_immw(cpu);	// pc
	cpu->twrd = v30_immw(cpu);	// ps
	v30_set_ps(cpu, cpu->twrd);
	cpu->regPC = cpu->tmpw;
}

// eb,cb: jump cb	(short jump)
void v30_opEB(CPU* cpu) {
	cpu->ltw = v30_immb(cpu);
	cpu->htw = (cpu->ltw & 0x80) ? 0xff : 0x00;
	cpu->regPC += cpu->tmpw;
}

// ec: in al,dw
void v30_opEC(CPU* cpu) {
	cpu->regAL = cpu_ird(cpu, cpu->regDW);
}

// ed: in aw,dw
void v30_opED(CPU* cpu) {
	cpu->regAH = cpu_ird(cpu, cpu->regDW+1);
	cpu->regAL = cpu_ird(cpu, cpu->regDW);
}

// ee: out dw,al
void v30_opEE(CPU* cpu) {
	cpu_iwr(cpu, cpu->regDW, cpu->regAL);
}

// ef: out dw,aw
void v30_opEF(CPU* cpu) {
	cpu_iwr(cpu, cpu->regDW + 1, cpu->regAH);
	cpu_iwr(cpu, cpu->regDW, cpu->regAL);
}

// f0: lock prefix (for multi-CPU)
void v30_opF0(CPU* cpu) {
	// cpu->flgLOCK = 1;
}

// f1: undef, doesn't cause interrupt
void v30_opF1(CPU* cpu) {}

// f2: REPNZ prefix for scas/cmps: repeat until Z=1
void v30_opF2(CPU* cpu) {
	cpu->regREP = V30_REPNZ;
}

// f3: REPZ prefix for scas/cmps: repeat until Z=0
// f3: REP prefix for ins/movs/outs/stos: cw--,repeat if cw!=0
void v30_opF3(CPU* cpu) {
	cpu->regREP = V30_REPZ;
}

// f4: hlt	halt until interrupt
void v30_opF4(CPU* cpu) {
	if (!((cpu->intrq & cpu->inten) && cpu->flgIE)) {
		cpu->flgHALT = 1;
		cpu->regPC = cpu->oldpc;
	} else {
		cpu->flgHALT = 0;
	}
}

// f5:cmc
void v30_opF5(CPU* cpu) {
	cpu->flgCY ^= 1;
}

// f6,mod:
void v30_opF60(CPU* cpu) {		// /0:test eb,ib
	cpu->tmpb = v30_immb(cpu);
	cpu->tmpb = v30_and8(cpu, cpu->ltw, cpu->tmpb);
}

void v30_opF62(CPU* cpu) {		// /2:not eb
	v30_wr_ea(cpu, cpu->ltw ^ 0xff, 0);
}

void v30_opF63(CPU* cpu) {		// /3:neg eb
	cpu->ltw = v30_sub8(cpu, 0, cpu->ltw, 0);
	v30_wr_ea(cpu, cpu->ltw, 0);
}

void v30_opF64(CPU* cpu) {		// /4:mulu eb
	cpu->regAW = cpu->ltw * cpu->regAL;
	cpu->flgCY = !!cpu->regAH;
	cpu->flgV = cpu->flgCY;
}

void v30_opF65(CPU* cpu) {		// /5:imul eb
	cpu->regAW = (signed char)cpu->ltw * (signed char)cpu->regAL;
	cpu->flgCY = !!(cpu->regAH != ((cpu->regAL & 0x80) ? 0xff : 0x00));
	cpu->flgV = cpu->flgCY;
}

void v30_opF66(CPU* cpu) {		// /6:div eb
	if (cpu->ltw == 0) {				// div by zero
		v30_exception(cpu, V30_INT_DE, 0);
	} else {
		if (cpu->regAW / cpu->ltw > 0xff) {	// cpu->ah >= cpu->ltw ?
			v30_exception(cpu, V30_INT_DE, 0);
		} else {
			cpu->twrd = cpu->regAW % cpu->ltw;
			cpu->tmpw = cpu->regAW / cpu->ltw;
			cpu->regAL = cpu->ltw;
			cpu->regAH = cpu->lwr;
		}
	}
}

void v30_opF67(CPU* cpu) {		// /7:idiv eb
	if (cpu->ltw == 0) {
		v30_exception(cpu, V30_INT_DE, 0);
	} else {
		// TODO: int0 if quo>0xff
		if (cpu->regAW / cpu->ltw > 0xff) {	// cpu->ah >= cpu->ltw
			v30_exception(cpu, V30_INT_DE, 0);
		} else {
			cpu->twrd = (signed short)cpu->regAW % (signed char)cpu->ltw;
			cpu->tmpw = (signed short)cpu->regAW / (signed char)cpu->ltw;
			cpu->regAL = cpu->ltw;
			cpu->regAH = cpu->lwr;
		}
	}
}

cbcpu v30_F6_tab[8] = {
	v30_opF60, v30_opF60, v30_opF62, v30_opF63,
	v30_opF64, v30_opF65, v30_opF66, v30_opF67
};

void v30_opF6(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	v30_F6_tab[(cpu->regMOD >> 3) & 7](cpu);
}

// f7,mod:

void v30_opF70(CPU* cpu) {		// /0:test ew,iw
	cpu->twrd = v30_immw(cpu);
	cpu->twrd = v30_and16(cpu, cpu->tmpw, cpu->twrd);
}

void v30_opF72(CPU* cpu) {		// /2:not ew
	v30_wr_ea(cpu, cpu->tmpw ^ 0xffff, 1);
}

void v30_opF73(CPU* cpu) {		// /3:neg ew
	cpu->twrd = v30_sub16(cpu, 0, cpu->tmpw, 0);
	v30_wr_ea(cpu, cpu->twrd, 1);
}

void v30_opF74(CPU* cpu) {		// /4:mulu ew
	cpu->tmpi = cpu->tmpw * cpu->regAW;
	cpu->regAW = cpu->tmpi & 0xffff;
	cpu->regDW = (cpu->tmpi >> 16) & 0xffff;
	cpu->flgCY = !!cpu->regDW;
	cpu->flgV = cpu->flgCY;
}

void v30_opF75(CPU* cpu) {		// /5:mul ew
	cpu->tmpi = (signed short)cpu->tmpw * (signed short)cpu->regAW;
	cpu->regAW = cpu->tmpi & 0xffff;
	cpu->regDW = (cpu->tmpi >> 16) & 0xffff;
	cpu->flgCY = !!(cpu->regDW != ((cpu->regAH & 0x80) ? 0xff : 0x00));
	cpu->flgV = cpu->flgCY;
}

void v30_opF76(CPU* cpu) {		// /6:div ew
	if (cpu->tmpw == 0) {				// div by zero
		v30_exception(cpu, V30_INT_DE, 0);
	} else {
		cpu->tmpi = (cpu->regDW << 16) | cpu->regAW;
		if (cpu->tmpi / cpu->tmpw > 0xffff) {		// cpu->dw >= cpu->tmpw
			v30_exception(cpu, V30_INT_DE, 0);
		} else {
			cpu->regAW = cpu->tmpi / cpu->tmpw;
			cpu->regDW = cpu->tmpi % cpu->tmpw;
		}
	}
}

void v30_opF77(CPU* cpu) {		// /7:idiv ew
	if (cpu->tmpw == 0) {
		v30_exception(cpu, V30_INT_DE, 0);
	} else {
		cpu->tmpi = (cpu->regDW << 16) | cpu->regAW;
		if ((signed int)cpu->tmpi / (signed short)cpu->tmpw > 0xffff) {		// cpu->dw >= cpu->tmpw ?
			v30_exception(cpu, V30_INT_DE, 0);
		} else {
			cpu->regAW = (signed int)cpu->tmpi / (signed short)cpu->tmpw;
			cpu->regDW = (signed int)cpu->tmpi % (signed short)cpu->tmpw;
		}
	}
}

cbcpu v30_F7_tab[8] = {
	v30_opF70, v30_opF70, v30_opF72, v30_opF73,
	v30_opF74, v30_opF75, v30_opF76, v30_opF77
};

void v30_opF7(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	v30_F7_tab[(cpu->regMOD >> 3) & 7](cpu);
}

// f8: clc
void v30_opF8(CPU* cpu) {
	cpu->flgCY = 0;
}

// f9: stc
void v30_opF9(CPU* cpu) {
	cpu->flgCY = 1;
}

// fa: cli
void v30_opFA(CPU* cpu) {
	cpu->flgIE = 0;
}

// fb: sti
void v30_opFB(CPU* cpu) {
	cpu->flgIE = 1;
}

// fc: cld
void v30_opFC(CPU* cpu) {
	cpu->flgDIR = 0;;
}

// fd: std
void v30_opFD(CPU* cpu) {
	cpu->flgDIR = 1;
}

// fe: inc/dec ea.byte
void v30_opFE(CPU* cpu) {
	v30_rd_ea(cpu, 0);
	switch((cpu->regMOD >> 3) & 7) {
		case 0: cpu->ltw = v30_inc8(cpu, cpu->ltw);
			v30_wr_ea(cpu, cpu->ltw, 0);
			break;
		case 1: cpu->ltw = v30_dec8(cpu, cpu->ltw);
			v30_wr_ea(cpu, cpu->ltw, 0);
			break;
		default:
			v30_undef(cpu);
			break;
	}
}

// ff: extend ops ea.word
void v30_opFF(CPU* cpu) {
	v30_rd_ea(cpu, 1);
	switch((cpu->regMOD >> 3) & 7) {
		case 0: cpu->tmpw = v30_inc16(cpu, cpu->tmpw);
			v30_wr_ea(cpu, cpu->tmpw, 1);
			break;	// inc ew
		case 1: cpu->tmpw = v30_dec16(cpu, cpu->tmpw);
			v30_wr_ea(cpu, cpu->tmpw, 1);
			break;	// dec ew
		case 2:	v30_push(cpu, cpu->regPC);
			cpu->regPC = cpu->tmpw;
			break; // call ew
		case 3:	cpu->twrd = v30_mrdw(cpu, cpu->flgSOVR ? cpu->regSEG : cpu->ea.seg.idx, cpu->ea.adr + 2);	// twrd = segment
			v30_callf(cpu, cpu->tmpw, cpu->twrd);
			break; // callf ed
		case 4: cpu->regPC = cpu->tmpw;
			break; // jmp ew
		case 5:	v30_set_ps(cpu, v30_mrdw(cpu, cpu->flgSOVR ? cpu->regSEG : cpu->ea.seg.idx, cpu->ea.adr + 2));
			cpu->regPC = cpu->tmpw;
			break; // jmpf ed
		case 6: v30_push(cpu, cpu->tmpw);
			break; // push ew
		case 7:
			break;	// ???
	}
}

// opcode tab
// TODO: NEC mnemonics ?
opCode v30_tab[256] = {
	{OF_MODRM, 1, v30_op00, 0, "add :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op01, 0, "add :e,:r"},
	{OF_MODRM, 1, v30_op02, 0, "add :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op03, 0, "add :r,:e"},
	{0, 1, v30_op04, 0, "add al,:1"},
	{0, 1, v30_op05, 0, "add aw,:2"},
	{0, 1, v30_op06, 0, "push es"},
	{0, 1, v30_op07, 0, "pop es"},
	// 08
	{OF_MODRM, 1, v30_op08, 0, "or :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op09, 0, "or :e,:r"},
	{OF_MODRM, 1, v30_op0A, 0, "or :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op0B, 0, "or :r,:e"},
	{0, 1, v30_op0C, 0, "or al,:1"},
	{0, 1, v30_op0D, 0, "or aw,:2"},
	{0, 1, v30_op0E, 0, "push cs"},
	{OF_PREFIX, 1, v30_op0F, NULL, "prefix 0F"},		// v30: 0F prefix (80186 doesn't have it)
	// 10
	{OF_MODRM, 1, v30_op10, 0, "addc :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op11, 0, "addc :e,:r"},
	{OF_MODRM, 1, v30_op12, 0, "addc :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op13, 0, "addc :r,:e"},
	{0, 1, v30_op14, 0, "addc al,:1"},
	{0, 1, v30_op15, 0, "addc aw,:2"},
	{0, 1, v30_op16, 0, "push ss"},
	{0, 1, v30_op17, 0, "pop ss"},
	// 18
	{OF_MODRM, 1, v30_op18, 0, "subc :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op19, 0, "subc :e,:r"},
	{OF_MODRM, 1, v30_op1A, 0, "subc :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op1B, 0, "subc :r,:e"},
	{0, 1, v30_op1C, 0, "subc al,:1"},
	{0, 1, v30_op1D, 0, "subc aw,:2"},
	{0, 1, v30_op1E, 0, "push ds"},
	{0, 1, v30_op1F, 0, "pop ds"},
	// 20
	{OF_MODRM, 1, v30_op20, 0, "and :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op21, 0, "and :e,:r"},
	{OF_MODRM, 1, v30_op22, 0, "and :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op23, 0, "and :r,:e"},
	{0, 1, v30_op24, 0, "and al,:1"},
	{0, 1, v30_op25, 0, "and aw,:2"},
	{OF_PREFIX, 1, v30_op26, 0, "segment DS1"},
	{0, 1, v30_op27, 0, "adj4a"},
	// 28
	{OF_MODRM, 1, v30_op28, 0, "sub :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op29, 0, "sub :e,:r"},
	{OF_MODRM, 1, v30_op2A, 0, "sub :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op2B, 0, "sub :r,:e"},
	{0, 1, v30_op2C, 0, "sub al,:1"},
	{0, 1, v30_op2D, 0, "sub aw,:2"},
	{OF_PREFIX, 1, v30_op2E, 0, "segment PS"},
	{0, 1, v30_op2F, 0, "adj4s"},
	// 30
	{OF_MODRM, 1, v30_op30, 0, "xor :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op31, 0, "xor :e,:r"},
	{OF_MODRM, 1, v30_op32, 0, "xor :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op33, 0, "xor :r,:e"},
	{0, 1, v30_op34, 0, "xor al,:1"},
	{0, 1, v30_op35, 0, "xor aw,:2"},
	{OF_PREFIX, 1, v30_op36, 0, "segment SS"},
	{0, 1, v30_op37, 0, "adjba"},
	// 38
	{OF_MODRM, 1, v30_op38, 0, "cmp :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op39, 0, "cmp :e,:r"},
	{OF_MODRM, 1, v30_op3A, 0, "cmp :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op3B, 0, "cmp :r,:e"},
	{0, 1, v30_op3C, 0, "cmp al,:1"},
	{0, 1, v30_op3D, 0, "cmp aw,:2"},
	{OF_PREFIX, 1, v30_op3E, 0, "segment DS0"},
	{0, 1, v30_op3F, 0, "adjbs"},
	// 40
	{0, 1, v30_op40, 0, "inc aw"},
	{0, 1, v30_op41, 0, "inc cw"},
	{0, 1, v30_op42, 0, "inc dw"},
	{0, 1, v30_op43, 0, "inc bw"},
	{0, 1, v30_op44, 0, "inc sp"},
	{0, 1, v30_op45, 0, "inc bp"},
	{0, 1, v30_op46, 0, "inc ix"},
	{0, 1, v30_op47, 0, "inc iy"},
	// 48
	{0, 1, v30_op48, 0, "dec aw"},
	{0, 1, v30_op49, 0, "dec cw"},
	{0, 1, v30_op4A, 0, "dec dw"},
	{0, 1, v30_op4B, 0, "dec bw"},
	{0, 1, v30_op4C, 0, "dec sp"},
	{0, 1, v30_op4D, 0, "dec bp"},
	{0, 1, v30_op4E, 0, "dec ix"},
	{0, 1, v30_op4F, 0, "dec iy"},
	// 50
	{0, 1, v30_op50, 0, "push aw"},
	{0, 1, v30_op51, 0, "push cw"},
	{0, 1, v30_op52, 0, "push dw"},
	{0, 1, v30_op53, 0, "push bw"},
	{0, 1, v30_op54, 0, "push sp"},
	{0, 1, v30_op55, 0, "push bp"},
	{0, 1, v30_op56, 0, "push ix"},
	{0, 1, v30_op57, 0, "push iy"},
	// 58
	{0, 1, v30_op58, 0, "pop aw"},
	{0, 1, v30_op59, 0, "pop cw"},
	{0, 1, v30_op5A, 0, "pop dw"},
	{0, 1, v30_op5B, 0, "pop bw"},
	{0, 1, v30_op5C, 0, "pop sp"},
	{0, 1, v30_op5D, 0, "pop bp"},
	{0, 1, v30_op5E, 0, "pop ix"},
	{0, 1, v30_op5F, 0, "pop iy"},
	// 60
	{0, 1, v30_op60, NULL, "pusha"},			// 1+
	{0, 1, v30_op61, NULL, "pop r"},			// 1+
	{OF_MODRM | OF_WORD, 1, v30_op62, NULL, "chkind :r,:e"},	// 1+
	{OF_MODRM | OF_WORD, 1, v30_undef, NULL, "undef"},
	{OF_PREFIX, 1, v30_op64, 0, "repnc"},		// repnc
	{OF_PREFIX, 1, v30_op65, 0, "repc"},		// repc
	{OF_MODRM, 1, v30_fpo2, 0, "FPO2 * :z"},		// v30: 66,67: FPO2 commands (extension of x87 commands, uPD72191)
	{OF_MODRM, 1, v30_fpo2, 0, "FPO2 * :z"},
	// 68
	{0, 1, v30_op68, NULL, "push :2"},			// 1+
	{OF_MODRM | OF_WORD, 1, v30_op69, NULL, "mul :r,:e,:2"},	// 1+
	{0, 1, v30_op6A, NULL, "push :1"},			// 1+
	{OF_MODRM | OF_WORD, 1, v30_op6B, NULL, "mul :r,:e,:1"},	// 1+
	{OF_SKIPABLE, 1, v30_op6C, NULL, ":Linmb [ds1::iy]"},	// 1+
	{OF_SKIPABLE, 1, v30_op6D, NULL, ":Linmw [ds1::iy]"},	// 1+
	{OF_SKIPABLE, 1, v30_op6E, NULL, ":Loutmb [:D::ix]"},	// 1+
	{OF_SKIPABLE, 1, v30_op6F, NULL, ":Loutmw [:D::ix]"},	// 1+
	// 70
	{0, 1, v30_op70, 0, "bv :3"},
	{0, 1, v30_op71, 0, "bnv :3"},
	{0, 1, v30_op72, 0, "bc :3"},		// jb, jnae
	{0, 1, v30_op73, 0, "bnc :3"},		// jnb, jae
	{0, 1, v30_op74, 0, "bz :3"},
	{0, 1, v30_op75, 0, "bnz :3"},
	{0, 1, v30_op76, 0, "bnh :3"},		// jna
	{0, 1, v30_op77, 0, "bh :3"},	// ja
	// 78
	{0, 1, v30_op78, 0, "bn :3"},
	{0, 1, v30_op79, 0, "bp :3"},
	{0, 1, v30_op7A, 0, "bpe :3"},
	{0, 1, v30_op7B, 0, "bpo :3"},
	{0, 1, v30_op7C, 0, "blt :3"},
	{0, 1, v30_op7D, 0, "bge :3"},
	{0, 1, v30_op7E, 0, "ble :3"},		// jng
	{0, 1, v30_op7F, 0, "bgt :3"},	// jg
	// 80
	{OF_MODRM, 1, v30_op80, 0, ":A :e,:1"},	// :A = mod:N (add,or,addc,sbb,and,sub,xor,cmp)
	{OF_MODRM | OF_WORD, 1, v30_op81, 0, ":A :e,:2"},
	{OF_MODRM, 1, v30_op82, 0, ":A :e,:1"},
	{OF_MODRM | OF_WORD, 1, v30_op83, 0, ":A :e,:1"},
	{OF_MODRM, 1, v30_op84, 0, "test :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op85, 0, "test :e,:r"},
	{OF_MODRM, 1, v30_op86, 0, "xch :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op87, 0, "xch :e,:r"},
	// 88
	{OF_MODRM, 1, v30_op88, 0, "mov :e,:r"},
	{OF_MODRM | OF_WORD, 1, v30_op89, 0, "mov :e,:r"},
	{OF_MODRM, 2, v30_op8A, 0, "mov :r,:e"},
	{OF_MODRM | OF_WORD, 9, v30_op8B, 0, "mov :r,:e"},	// v20:13T, v30:9T[+4T if odd address]
	{OF_MODRM | OF_WORD, 1, v30_op8C, 0, "mov :e,:s"},	// :s segment register from mod:N
	{OF_MODRM | OF_WORD, 1, v30_op8D, 0, "ldea :r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_op8E, 0, "mov :s,:e"},
	{OF_MODRM | OF_WORD, 1, v30_undef, 0, "undef"}, // "pop :e"},	// !!! /0 push, /1../7 nodef
	// 90
	{0, 1, v30_op90, 0, "nop"},	// xch aw,aw
	{0, 1, v30_op91, 0, "xch aw,cw"},
	{0, 1, v30_op92, 0, "xch aw,dw"},
	{0, 1, v30_op93, 0, "xch aw,bw"},
	{0, 1, v30_op94, 0, "xch aw,sp"},
	{0, 1, v30_op95, 0, "xch aw,bp"},
	{0, 1, v30_op96, 0, "xch aw,ix"},
	{0, 1, v30_op97, 0, "xch aw,iy"},
	// 98
	{0, 1, v30_op98, 0, "cvtbw"},
	{0, 1, v30_op99, 0, "cvtwl"},
	{OF_SKIPABLE, 1, v30_op9A, 0, "callf :p"},
	{0, 1, v30_op9B, 0, "poll"},
	{0, 1, v30_op9C, 0, "pushf"},
	{0, 1, v30_op9D, 0, "popf"},
	{0, 1, v30_op9E, 0, "mov psw,ah"},
	{0, 1, v30_op9F, 0, "mov ah,psw"},
	// a0
	{0, 1, v30_opA0, 0, "mov al,[:D:::2]"},
	{OF_WORD, 1, v30_opA1, 0, "mov aw,[:D:::2]"},
	{0, 1, v30_opA2, 0, "mov [:D:::2],al"},
	{OF_WORD, 1, v30_opA3, 0, "mov [:D:::2],aw"},
	{OF_SKIPABLE, 1, v30_opA4, 0, ":Lmovbkb [:D::ix],[ds1::iy]"},
	{OF_SKIPABLE | OF_WORD, 1, v30_opA5, 0, ":Lmovbkw [:D::ix],[ds1::iy]"},
	{OF_SKIPABLE, 1, v30_opA6, 0, ":Lcmpbkb [:D::ix],[ds1::iy]"},
	{OF_SKIPABLE | OF_WORD, 1, v30_opA7, 0, ":Lcmpbkw [:D::ix],[ds1::iy]"},
	// a8
	{0, 1, v30_opA8, 0, "test al,:1"},
	{OF_WORD, 1, v30_opA9, 0, "test aw,:2"},
	{OF_SKIPABLE, 1, v30_opAA, 0, ":Lstmb [ds1::iy],al"},
	{OF_SKIPABLE | OF_WORD, 1, v30_opAB, 0, ":Lstmw [ds1::iy],aw"},
	{0, 1, v30_opAC, 0, ":Lldmb al,[:D::ix]"},
	{OF_WORD, 1, v30_opAD, 0, ":Lldmw aw,[:D::ix]"},
	{OF_SKIPABLE, 1, v30_opAE, 0, ":Lcmpmb al,[ds1::iy]"},
	{OF_SKIPABLE | OF_WORD, 1, v30_opAF, 0, ":Lcmpmw aw,[ds1::iy]"},
	// b0
	{0, 1, v30_opB0, 0, "mov al,:1"},
	{0, 1, v30_opB1, 0, "mov cl,:1"},
	{0, 1, v30_opB2, 0, "mov dl,:1"},
	{0, 1, v30_opB3, 0, "mov bl,:1"},
	{0, 1, v30_opB4, 0, "mov ah,:1"},
	{0, 1, v30_opB5, 0, "mov ch,:1"},
	{0, 1, v30_opB6, 0, "mov dh,:1"},
	{0, 1, v30_opB7, 0, "mov bh,:1"},
	// b8
	{OF_WORD, 1, v30_opB8, 0, "mov aw,:2"},
	{OF_WORD, 1, v30_opB9, 0, "mov cw,:2"},
	{OF_WORD, 1, v30_opBA, 0, "mov dw,:2"},
	{OF_WORD, 1, v30_opBB, 0, "mov bw,:2"},
	{OF_WORD, 1, v30_opBC, 0, "mov sp,:2"},
	{OF_WORD, 1, v30_opBD, 0, "mov bp,:2"},
	{OF_WORD, 1, v30_opBE, 0, "mov ix,:2"},
	{OF_WORD, 1, v30_opBF, 0, "mov iy,:2"},
	// c0
	{OF_MODRM, 1, v30_opC0, NULL, ":R :e,:1"},		// 1+ :R rotate group (rol,ror,rcl,rcr,sal,shr,*rot6,sar)
	{OF_MODRM | OF_WORD, 1, v30_opC1, NULL, ":R :e,:1"},	// 1+
	{0, 1, v30_opC2, 0, "ret :2"},
	{0, 1, v30_opC3, 0, "ret"},
	{OF_MODRM | OF_WORD, 1, v30_opC4, 0, "mov ds1,:r,:e"},
	{OF_MODRM | OF_WORD, 1, v30_opC5, 0, "mov ds0,:r,:e"},
	{OF_MODRM, 1, v30_opC6, 0, "mov :e,:1"},			// /0 mov, /1../7 undef !!!
	{OF_MODRM | OF_WORD, 1, v30_opC7, 0, "mov :e,:2"},		// /0 mov, /1../7 undef
	// c8
	{0, 1, v30_opC8, NULL, "prepare :2,:1"},				// 1+
	{0, 1, v30_opC9, NULL, "dispose"},					// 1+
	{0, 1, v30_opCA, 0, "retf :2"},
	{0, 1, v30_opCB, 0, "retf"},
	{OF_SKIPABLE, 1, v30_opCC, 0, "brk 3"},
	{OF_SKIPABLE, 1, v30_opCD, 0, "brk :1"},
	{OF_SKIPABLE, 1, v30_opCE, 0, "brkv"},
	{0, 1, v30_opCF, 0, "reti"},
	// d0
	{OF_MODCOM, 1, v30_opD0, NULL, ":R :e,1"},
	{OF_MODCOM | OF_WORD, 1, v30_opD1, NULL, ":R :e,1"},
	{OF_MODCOM, 1, v30_opD2, NULL, ":R :e,cl"},
	{OF_MODCOM | OF_WORD, 1, v30_opD3, NULL, ":R :e,cl"},
	{0, 1, v30_opD4, 0, "cvtbd :1"},
	{0, 1, v30_opD5, 0, "cvtdb :1"},
	{0, 1, v30_opD6, 0, "transb*"},					// V30 D6=D7 (transb)
	{0, 1, v30_opD7, 0, "transb"}, // xlatb al,[:D::bw+al]"},
	// d8
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},				// D8..DF = FPO1, ESC external x87
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},
	{OF_MODRM, 1, v30_fpo1, 0, "fpo1 * :z"},
	// e0
	{0, 1, v30_opE0, 0, "dbnzne :3"},
	{0, 1, v30_opE1, 0, "dbnze :3"},
	{0, 1, v30_opE2, 0, "dbnz :3"},
	{0, 1, v30_opE3, 0, "bcwz :3"},
	{0, 1, v30_opE4, 0, "in al,:1"},
	{0, 1, v30_opE5, 0, "in aw,:1"},
	{0, 1, v30_opE6, 0, "out :1,al"},
	{0, 1, v30_opE7, 0, "out :1,aw"},
	// e8
	{OF_SKIPABLE, 1, v30_opE8, 0, "call :n"},		// :n = near, 2byte offset
	{0, 1, v30_opE9, 0, "br :n"},			// jmp near
	{0, 1, v30_opEA, 0, "br :p"},			// jmp far
	{0, 1, v30_opEB, 0, "br :3"},			// jmp short
	{0, 1, v30_opEC, 0, "in al,dw"},
	{0, 1, v30_opED, 0, "in aw,dw"},
	{0, 1, v30_opEE, 0, "out dw,al"},
	{0, 1, v30_opEF, 0, "out dw,aw"},
	// f0
	{OF_PREFIX, 1, v30_opF0, 0, "buslock"},
	{0, 1, v30_opF1, 0, "undef"},
	{OF_PREFIX, 1, v30_opF2, 0, "repnz"},
	{OF_PREFIX, 1, v30_opF3, 0, "repz/rep"},
	{0, 1, v30_opF4, 0, "halt"},
	{0, 1, v30_opF5, 0, "not1 cy"},
	{OF_MODCOM, 1, v30_opF6, NULL, ":X :e"},		// test,test,not,neg,mul,imul,div,idiv
	{OF_MODCOM | OF_WORD, 1, v30_opF7, NULL, ":Y :e"},
	// f8
	{0, 1, v30_opF8, 0, "clr1 cy"},
	{0, 1, v30_opF9, 0, "set1 cy"},
	{0, 1, v30_opFA, 0, "di"},
	{0, 1, v30_opFB, 0, "ei"},
	{0, 1, v30_opFC, 0, "clr1 dir"},
	{0, 1, v30_opFD, 0, "set1 dir"},
	{OF_MODCOM, 1, v30_opFE, NULL, ":E :e"},		// inc,dec,...
	{OF_MODCOM | OF_WORD | OF_SKIPABLE, 1, v30_opFF, NULL, ":F :e"},	// incw,decw,not,neg,call,callf,jmp,jmpf,push,???
};

// NOTES:
// ED,ED,n in 8080 mode is native-BRK n
// ED,FD in 8080 mode: pop pc,ps,psw; write disable MD
