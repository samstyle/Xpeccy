#pragma once

#include "../cpu.h"

#ifdef WORDS_BIG_ENDIAN
typedef struct {
	unsigned _nu:24;		// not used (padding to 32 bits)
	unsigned s:1;
	unsigned z:1;
	unsigned f5:1;
	unsigned h:1;
	unsigned f3:1;
	unsigned pv:1;
	unsigned n:1;
	unsigned c:1;
} z80flag_t;
#else
typedef struct {
	unsigned c:1;
	unsigned n:1;
	unsigned pv:1;
	unsigned f3:1;
	unsigned h:1;
	unsigned f5:1;
	unsigned z:1;
	unsigned s:1;
	unsigned _nu:24;		// not used (padding to 32 bits)
} z80flag_t;
#endif

#include "z80_macro.h"

enum {
	Z80_REG_PC = 1,
	Z80_REG_SP,
	Z80_REG_AF,
	Z80_REG_BC,
	Z80_REG_DE,
	Z80_REG_HL,
	Z80_REG_AFA,
	Z80_REG_BCA,
	Z80_REG_DEA,
	Z80_REG_HLA,
	Z80_REG_IX,
	Z80_REG_IY,
	Z80_REG_I,
	Z80_REG_R
};

#define Z80_INT	1
#define Z80_NMI	(1<<1)

void z80_reset(CPU*);
int z80_exec(CPU*);
xAsmScan z80_asm(int, const char*, char*);
xMnem z80_mnem(CPU*, int, cbdmr, void*);

void z80_get_regs(CPU*, xRegBunch*);
void z80_set_regs(CPU*, xRegBunch);

void z80_set_flag(CPU*, int);
int z80_get_flag(CPU*);

// internal

int z80_mrd(CPU*, int);
void z80_mwr(CPU*, int, int);
int z80_iord(CPU*, int);
void z80_iowr(CPU*, int, int);
// alu
unsigned char z80_inc8(CPU*, unsigned char);
unsigned char z80_dec8(CPU*, unsigned char);
unsigned char z80_add8(CPU*, unsigned char, unsigned char);
unsigned char z80_sub8(CPU*, unsigned char, unsigned char);
unsigned short z80_add16(CPU*, unsigned short, unsigned short);
unsigned short z80_adc16(CPU*, unsigned short, unsigned short, unsigned char);
unsigned short z80_sub16(CPU*, unsigned short, unsigned short, unsigned char);
void z80_and8(CPU*, unsigned char);
void z80_or8(CPU*, unsigned char);
void z80_xor8(CPU*, unsigned char);
void z80_cp8(CPU*, unsigned char);
// rotation
unsigned char z80_rl(CPU*, unsigned char);
unsigned char z80_rr(CPU*, unsigned char);
unsigned char z80_rlc(CPU*, unsigned char);
unsigned char z80_rrc(CPU*, unsigned char);
unsigned char z80_sla(CPU*, unsigned char);
unsigned char z80_sra(CPU*, unsigned char);
unsigned char z80_sll(CPU*, unsigned char);
unsigned char z80_srl(CPU*, unsigned char);

unsigned short z80_pop(CPU*);
void z80_push(CPU*, unsigned short);
void z80_ret(CPU*);
