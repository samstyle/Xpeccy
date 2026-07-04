#include "v30.h"
#include "v30regs.h"

void v30_undef(CPU*);

int v30_get_reg8(CPU*, int);
void v30_set_reg8(CPU*, int, int);
void v30_set_ps(CPU*, int);

int v30_immb(CPU*);
int v30_immw(CPU*);
int v30_mrdw(CPU*, int, int);
int v30_mwrw(CPU*, int, int, int);
int v30_mrdb(CPU*, int, int);
int v30_mwrb(CPU*, int, int, int);

void v30_push(CPU*, int);
int v30_pop(CPU*);

int v30_rd_ea(CPU*, int);
void v30_wr_ea(CPU*, int, int);

// bits
// 0f,10; 0f,11 : test1 :e,cl	Z = ! bit state
void v30_op0f10(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->flgZ = !(cpu->tmpb & (1 << (cpu->regCL & 7)));
}

void v30_op0f11(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->flgZ = !(cpu->tmpw & (1 << (cpu->regCL & 15)));
}

// 0f,12; 0f,13 : clr1 :e,cl	reset bit
void v30_op0f12(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->tmpb &= ~(1 << (cpu->regCL & 7));
	v30_wr_ea(cpu, cpu->tmpb, 0);
}

void v30_op0f13(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->tmpw &= ~(1 << (cpu->regCL & 15));
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 0f,14; 0f,15 : set1 :e,cl	set bit
void v30_op0f14(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->tmpb |= (1 << (cpu->regCL & 7));
	v30_wr_ea(cpu, cpu->tmpb, 0);
}

void v30_op0f15(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->tmpw |= (1 << (cpu->regCL & 15));
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 0f,16; 0f,17 : not1 :e,cl	invert bit
void v30_op0f16(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->tmpb ^= (1 << (cpu->regCL & 7));
	v30_wr_ea(cpu, cpu->tmpb, 0);
}

void v30_op0f17(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->tmpw ^= (1 << (cpu->regCL & 15));
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

//-----

// 0f,18; 0f,19 : test1 :e,imm
void v30_op0f18(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->tmp = v30_immb(cpu);
	cpu->flgZ = !(cpu->tmpb & (1 << (cpu->tmp & 7)));
}

void v30_op0f19(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->tmp = v30_immb(cpu);
	cpu->flgZ = !(cpu->tmpw & (1 << (cpu->tmp & 15)));
}

// 0f,1a; 0f,1b : clr1 :e,imm
void v30_op0f1a(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->tmp = v30_immb(cpu);
	cpu->tmpb &= ~(1 << (cpu->tmp & 7));
	v30_wr_ea(cpu, cpu->tmpb, 0);
}

void v30_op0f1b(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->tmp = v30_immb(cpu);
	cpu->tmpw &= ~(1 << (cpu->tmp & 15));
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 0f,1c; 0f,1d : set1 :e,imm
void v30_op0f1c(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->tmp = v30_immb(cpu);
	cpu->tmpb |= (1 << (cpu->tmp & 7));
	v30_wr_ea(cpu, cpu->tmpb, 0);
}

void v30_op0f1d(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->tmp = v30_immb(cpu);
	cpu->tmpw |= (1 << (cpu->tmp & 15));
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// 0f,1e; 0f,1f : not1 :e,imm
void v30_op0f1e(CPU* cpu) {
	cpu->tmpb = v30_rd_ea(cpu, 0);
	cpu->tmp = v30_immb(cpu);
	cpu->tmpb ^= (1 << (cpu->tmp & 7));
	v30_wr_ea(cpu, cpu->tmpb, 0);
}

void v30_op0f1f(CPU* cpu) {
	cpu->tmpw = v30_rd_ea(cpu, 1);
	cpu->tmp = v30_immb(cpu);
	cpu->tmpw ^= (1 << (cpu->tmp & 15));
	v30_wr_ea(cpu, cpu->tmpw, 1);
}

// bcd

// 0f,20	add4s		 [ds1:iy] += [ds0:ix], cl 4-bit digits
void v30_op0f20(CPU* cpu) {
	cpu->flgCY = 0;
	cpu->flgZ = 1;
	int sadr = cpu->regIX;
	int dadr = cpu->regIY;
	while ((cpu->regCL < 0xff) && (cpu->regCL > 0)) {
		cpu->tmp = v30_mrdb(cpu, cpu->regDS1, dadr);
		cpu->tmpb = v30_mrdb(cpu, cpu->regDS0, sadr);
		cpu->ltw = (cpu->tmp & 0x0f) + (cpu->tmpb & 0x0f) + cpu->flgCY;
		cpu->flgCY = (cpu->ltw > 0x0f);
		if (cpu->flgCY) cpu->ltw -= 10;
		cpu->htw = ((cpu->tmp >> 4) & 0x0f) + ((cpu->tmpb >> 4) & 0x0f) + cpu->flgCY;
		cpu->flgCY = (cpu->htw > 0x0f);
		if (cpu->flgCY) cpu->htw -= 10;
		if (cpu->ltw || cpu->htw) cpu->flgZ = 0;
		cpu->ltw = (cpu->ltw & 0x0f) | (cpu->htw << 4);
		v30_mwrb(cpu, cpu->regDS1, dadr, cpu->ltw);
		dadr++;
		cpu->regCL -= 2;
	}
}

// 0f,22	sub4s
void v30_op0f22(CPU* cpu) {
	cpu->flgCY = 0;
	cpu->flgZ = 1;
	int sadr = cpu->regIX;
	int dadr = cpu->regIY;
	while ((cpu->regCL < 0xff) && (cpu->regCL > 0)) {
		cpu->tmp = v30_mrdb(cpu, cpu->regDS1, dadr);
		cpu->tmpb = v30_mrdb(cpu, cpu->regDS0, sadr);
		cpu->ltw = (cpu->tmp & 0x0f) - (cpu->tmpb & 0x0f) - cpu->flgCY;
		cpu->flgCY = (cpu->ltw > 0x0f);
		if (cpu->flgCY) cpu->ltw += 10;
		cpu->htw = ((cpu->tmp >> 4) & 0x0f) - ((cpu->tmpb >> 4) & 0x0f) - cpu->flgCY;
		cpu->flgCY = (cpu->htw > 0x0f);
		if (cpu->flgCY) cpu->htw += 10;
		if (cpu->ltw || cpu->htw) cpu->flgZ = 0;
		cpu->ltw = (cpu->ltw & 0x0f) | (cpu->htw << 4);
		v30_mwrb(cpu, cpu->regDS1, dadr, cpu->ltw);
		dadr++;
		cpu->regCL -= 2;
	}
}

// 0f,26	cmp4s = sub4s w/o writing back
void v30_op0f26(CPU* cpu) {
	cpu->flgCY = 0;
	cpu->flgZ = 1;
	int sadr = cpu->regIX;
	int dadr = cpu->regIY;
	while ((cpu->regCL < 0xff) && (cpu->regCL > 0)) {
		cpu->tmp = v30_mrdb(cpu, cpu->regDS1, dadr);
		cpu->tmpb = v30_mrdb(cpu, cpu->regDS0, sadr);
		if (cpu->tmp != cpu->tmpb) {
			cpu->flgZ = 0;					// reset till the end
		} else {
			cpu->flgCY = !!(cpu->tmp < cpu->tmpb);		// count only last pair
		}
		dadr++;
		cpu->regCL -= 2;
	}
}

// rotate

// rol4 :e
void v30_op0f28(CPU* cpu) {
	cpu->tmp = v30_rd_ea(cpu, 0);		// 1:2
	cpu->tmpb = cpu->regAL & 0x0f;		// 0:4
	cpu->regAL &= 0xf0;			// 3:0
	cpu->regAL |= (cpu->tmp >> 4) & 0x0f;	// 3:1		al
	cpu->tmp <<= 4;				// 2:0
	cpu->tmp |= cpu->tmpb;			// 2:4		ea
	v30_wr_ea(cpu, cpu->tmp, 0);
}

// ror4 :e
void v30_op0f2a(CPU* cpu) {
	cpu->tmp = v30_rd_ea(cpu, 0);		// 1:2
	cpu->tmpb = cpu->regAL & 0x0f;		// 0:4
	cpu->regAL &= 0xf0;			// 3:0
	cpu->regAL |= (cpu->tmp & 0x0f);	// 3:2		al
	cpu->tmp >>= 4;				// 0:1
	cpu->tmp &= 0x0f;
	cpu->tmp |= (cpu->tmpb << 4);		// 4:1		ea
	v30_wr_ea(cpu, cpu->tmp, 0);
}

// ins : move lower r1 bits from AW to [ds1:iy]+(r2.bits)
void v30_ins(CPU* cpu) {	// in:cpu->tmpb = src.bits, cpu->tmp & 15 = dst.offset; out:iy,cpu->tmp updated
	cpu->tmpw = v30_mrdw(cpu, cpu->regDS1, cpu->regIY);	// lo16
	cpu->twrd = v30_mrdw(cpu, cpu->regDS1, cpu->regIY + 2);	// hi16
	cpu->tmpi = (cpu->twrd << 16) | cpu->tmpw;	// full 32
	int smsk = (1 << cpu->tmpb) - 1;		// mask for src.bits
	int dmsk = ~(smsk << (cpu->tmp & 15));		// shift to dst.offset and revert
	cpu->tmpi &= dmsk;				// reset dst.bits
	cpu->tmpi |= (cpu->regAW & smsk) << (cpu->tmp & 15);	// copy src.bits to dst
	v30_mwrw(cpu, cpu->regDS1, cpu->regIY, cpu->tmpi & 0xffff);	// write it back
	v30_mwrw(cpu, cpu->regDS1, cpu->regIY + 2, (cpu->tmpi >> 16) & 0xffff);
	// iy and dst.reg must be updated to next bit
	int nextbit = cpu->tmpb + (cpu->tmp & 15);	// next bit nr (max 16+15=31)
	cpu->regIY += (nextbit >> 3) & 2;
	cpu->tmp &= 0xf0;
	cpu->tmp |= (nextbit & 15);
}

void v30_op0f31(CPU* cpu) {
	cpu->regMOD = v30_immb(cpu);
	cpu->tmpb = (v30_get_reg8(cpu, cpu->regMOD & 7) & 15) + 1;		// r1	src.bits (1-16)
	cpu->tmp = v30_get_reg8(cpu, (cpu->regMOD >> 3) & 7);			// r2	dst.offset (& 15)
	v30_ins(cpu);
	v30_set_reg8(cpu, (cpu->regMOD >> 3) & 7, cpu->tmp);
}

void v30_op0f39(CPU* cpu) {
	cpu->regMOD = v30_immb(cpu);
	cpu->tmpb = (v30_immb(cpu) & 15) + 4;				// imm4		src.bits
	cpu->tmp = v30_get_reg8(cpu, cpu->regMOD & 7);			// r1		dst offset (& 15)
	v30_ins(cpu);
	v30_set_reg8(cpu, cpu->regMOD & 7, cpu->tmp);
}

// ext: reverse ins: bits from memory to lower bits of aw
void v30_ext(CPU* cpu) {
	cpu->tmpw = v30_mrdw(cpu, cpu->regDS1, cpu->regIY);	// lo16
	cpu->twrd = v30_mrdw(cpu, cpu->regDS1, cpu->regIY + 2);	// hi16
	cpu->tmpi = (cpu->twrd << 16) | cpu->tmpw;	// full 32
	int smsk = (1 << cpu->tmpb) - 1;		// mask for src.bits
	cpu->tmpi >>= (cpu->tmp & 15);
	cpu->regAW &= ~smsk;
	cpu->regAW |= (cpu->tmpi & smsk);
	// move to next bit
	int nextbit = cpu->tmpb + (cpu->tmp & 15);		// next bit nr (max 16+15=31)
	cpu->regIY += (nextbit >> 3) & 2;
	cpu->tmp &= 0xf0;
	cpu->tmp |= (nextbit & 15);
}

void v30_op0f33(CPU* cpu) {
	cpu->regMOD = v30_immb(cpu);
	cpu->tmpb = (v30_get_reg8(cpu, cpu->regMOD & 7) & 15) + 1;		// r1	src.bits (1-16)
	cpu->tmp = v30_get_reg8(cpu, (cpu->regMOD >> 3) & 7);			// r2	dst.offset (& 15)
	v30_ext(cpu);
	v30_set_reg8(cpu, (cpu->regMOD >> 3) & 7, cpu->tmp);
}

void v30_op0f3b(CPU* cpu) {
	cpu->regMOD = v30_immb(cpu);
	cpu->tmpb = (v30_immb(cpu) & 15) + 4;				// imm4		src.bits
	cpu->tmp = v30_get_reg8(cpu, cpu->regMOD & 7);			// r1		dst offset (& 15)
	v30_ext(cpu);
	v30_set_reg8(cpu, cpu->regMOD & 7, cpu->tmp);
}

// enter emulation mode by 'brk imm8'
// 0f,ff brkem imm8
void v30_op0fff(CPU* cpu) {
	cpu->tmp = v30_immb(cpu);
	v30_push(cpu, v30_getflag(cpu));
	v30_push(cpu, cpu->regPS);
	v30_push(cpu, cpu->regPC);
	cpu->flgMD = 0;		// emulation mode
	cpu->flgBLKM = 0;	// enable MD change by RETI
	v30_set_ps(cpu, v30_mrdw(cpu, 0, (cpu->tmp * 4) + 2));
	cpu->regPC = v30_mrdw(cpu, 0, cpu->tmp * 4);
}

// NOTE: if opcode.t==0, take opcode from main table
opCode v30_0f_tab[256] = {
	// CHECK: 00..0F = 10..1F ???
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{OF_MODRM, 3, v30_op0f10, NULL, "test1 :e,cl"},				// 10 /0: Z = bit CL of [EA]
	{OF_MODRM | OF_WORD, 3, v30_op0f11, NULL, "test1 :e,cl"},		// 11 /0: ...
	{OF_MODRM, 4, v30_op0f12, NULL, "clr1 :e,cl"},				// 12 /0: reset bit cl in [EA]
	{OF_MODRM | OF_WORD, 4, v30_op0f13, NULL, "clr1 :e,cl"},		// 13 /0: ...
	{OF_MODRM, 4, v30_op0f14, NULL, "set1 :e,cl"},				// 14 /0: set bit cl in [EA]
	{OF_MODRM | OF_WORD, 4, v30_op0f15, NULL, "set1 :e,cl"},		// 15 /0: ...
	{OF_MODRM, 4, v30_op0f16, NULL, "not1 :e,cl"},				// 16 /0: invert bit cl in [EA]
	{OF_MODRM | OF_WORD, 4, v30_op0f17, NULL, "not1 :e,cl"},		// 17 /0: ...

	{OF_MODRM, 4, v30_op0f18, NULL, "test1 :e,:1"},			// 18 /0: test1 :e,imm3	(immb instead of cl)
	{OF_MODRM | OF_WORD, 4, v30_op0f19, NULL, "test1 :e,:1"},	// 19 /0: ... (imm4)
	{OF_MODRM, 4, v30_op0f1a, NULL, "clr :e,:1"},			// 1a /0: reset bit (imm3) in [EA]
	{OF_MODRM | OF_WORD, 4, v30_op0f1b, NULL, "clr :e,:1"},		// 1b /0: ... (imm4)
	{OF_MODRM, 4, v30_op0f1c, NULL, "set1 :e,:1"},			// 1c /0: set bit (imm3) in [EA]
	{OF_MODRM | OF_WORD, 4, v30_op0f1d, NULL, "set1 :e,:1"},	// 1d /0: ... (imm4)
	{OF_MODRM, 4, v30_op0f1e, NULL, "not1 :e,:1"},			// 1e /0 invert bit (imm3) in [EA]
	{OF_MODRM | OF_WORD, 4, v30_op0f1f, NULL, "not1 :e,:1"},	// 1f /0 ... (imm4)

	{0, 1, v30_op0f20, NULL, "*add4s"},	// 20: bcd add: [ds1:iy] += [ds0:ix], cl 4-bit digits
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_op0f22, NULL, "*sub4s"},	// 22: bcd sub
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_op0f26, NULL, "*cmp4s"},	// 26: bcd compare
	{0, 1, v30_undef, NULL, "undef"},

	{OF_MODRM, 25, v30_op0f28, NULL, "rol4 :e"},	// 28: cyclic rotate left al<-EA by 4 bits
	{0, 1, v30_undef, NULL, "undef"},
	{OF_MODRM, 29, v30_op0f2a, NULL, "ror4 :e"},	// 2a: cyclic rotate right al->EA by 4 bits
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},	// 30
	{0, 35, v30_op0f31, NULL, "ins :d,:r"},	// 0f,31,c0..ff: ins r1,r2	; r2 (lower 4 bits) bits from aw -> to [ds1:iy] + r1 (lower 4 bits) bits
	{0, 1, v30_undef, NULL, "undef"},
	{0, 26, v30_op0f33, NULL, "ext :d,:r"},	// 0f,33,c0..ff: ext r1,r2	; r2.4bits from [ds1:iy]+r1.4bits copy to lower bits of aw
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},	// 38
	{0, 67, v30_op0f39, NULL, "ins :d,:1"},	// 0f,31,c0..cf: ins r1,imm4
	{0, 1, v30_undef, NULL, "undef"},
	{0, 21, v30_op0f3b, NULL, "ext :d,:1"},	// ext r1,imm4
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},

	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_undef, NULL, "undef"},
	{0, 1, v30_op0fff, NULL, "brkem :1"},		// push psw,ps,pc; MD=0, write enable MD; ps=(imm8 * 4 + 2); pc=(imm8 * 4);
};
