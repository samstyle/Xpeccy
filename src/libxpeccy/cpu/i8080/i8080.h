#pragma once

#define I8080_INT	1

#define IFL_S	0x80
#define	IFL_Z	0x40
#define IFL_5	0x20
#define IFL_A	0x10
#define	IFL_3	0x08
#define IFL_P	0x04
#define IFL_1	0x02
#define IFL_C	0x01

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

#include "../cpu.h"

void i8080_reset(CPU*);
int i8080_exec(CPU*);
xAsmScan i8080_asm(const char*, char*);
xMnem i8080_mnem(CPU*, int, cbdmr, void*);
void i8080_get_regs(CPU*, xRegBunch*);
void i8080_set_regs(CPU*, xRegBunch);
