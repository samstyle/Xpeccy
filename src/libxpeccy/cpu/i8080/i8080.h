#pragma once

#include "../cpu.h"

#define I8080_INT	1

/*
#define IFL_S	0x80
#define	IFL_Z	0x40
#define IFL_5	0x20
#define IFL_A	0x10
#define	IFL_3	0x08
#define IFL_P	0x04
#define IFL_1	0x02
#define IFL_C	0x01
*/

#define regA regs[0].l
#define regBC regs[1].w
#define regB regs[1].h
#define regC regs[1].l
#define regDE regs[2].w
#define regD regs[2].h
#define regE regs[2].l
#define regHL regs[3].w
#define regH regs[3].h
#define regL regs[3].l
#define regSP regs[6].w
#define regSPh regs[6].h
#define regSPl regs[6].l
#define regPC regs[7].w
#define regPCh regs[7].h
#define regPCl regs[7].l
#define regWZ regs[8].w
#define regWZh regs[8].h
#define regWZl regs[8].l

enum {
	I8080_REG_PC = 1,
	I8080_REG_SP,
	I8080_REG_AF,
	I8080_REG_BC,
	I8080_REG_DE,
	I8080_REG_HL,
};

#ifdef WORDS_LITTLE_ENDIAN
typedef struct {
	unsigned c:1;
	unsigned f1:1;
	unsigned p:1;
	unsigned f3:1;
	unsigned a:1;
	unsigned f5:1;
	unsigned z:1;
	unsigned s:1;
	unsigned _nu:24;
} i80flag_t;
#else
typedef struct {
	unsigned _nu:24;
	unsigned s:1;
	unsigned z:1;
	unsigned f5:1;
	unsigned a:1;
	unsigned f3:1;
	unsigned p:1;
	unsigned f1:1;
	unsigned c:1;
} i80flag_t;
#endif

void i8080_reset(CPU*);
int i8080_exec(CPU*);
xAsmScan i8080_asm(int, const char*, char*);
xMnem i8080_mnem(CPU*, int, cbdmr, void*);
void i8080_get_regs(CPU*, xRegBunch*);
void i8080_set_regs(CPU*, xRegBunch);

void i8080_set_flag(CPU*, int);
int i8080_get_flag(CPU*);
