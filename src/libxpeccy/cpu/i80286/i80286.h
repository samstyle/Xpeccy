#pragma once

#include "../cpu.h"

// flag
#define I286_FC	0x0001	// carry
#define I286_FP 0x0004	// parity
#define I286_FA 0x0010	// half-carry
#define I286_FZ 0x0040	// zero
#define I286_FS 0x0080	// sign
#define I286_FT 0x0100	// trap
#define I286_FI 0x0200	// interrupt
#define I286_FD	0x0400	// direction
#define I286_FO 0x0800	// overflow
#define I286_FIP 0x3000	// 2bits: IOPL
#define I286_FN	0x4000	// nested flag
// msw
#define I286_FPE 0x0001	// protected mode
#define I286_FMP 0x0002 // allow int7 on wait
#define I286_FEM 0x0004 // allow int7 on esc
#define I286_FTS 0x0008 // task switch: next instruction will cause int7

#define I286_INT	(1<<0)
#define I286_NMI	(1<<1)
#define I286_BLK_NMI	(1<<16)

// INT numbers
#define I286_INT_DE	0
#define I286_INT_DB	1
#define I286_INT_NMI	2
#define I286_INT_BP	3
#define I286_INT_OF	4
#define I286_INT_BR	5
#define I286_INT_UD	6
#define I286_INT_NM	7
#define I286_INT_DF	8
#define I286_INT_SL	9	// segment limit
#define I286_INT_TS	10
#define I286_INT_NP	11
#define I286_INT_SS	12
#define I286_INT_GP	13
#define I286_INT_MF	16

enum {
	I286_MOD_REAL = 0,
	I286_MOD_PROT
};

enum {
	I286_REP_NONE = 0,
	I286_REPNZ,
	I286_REPZ,
	I286_SEG_ES,
	I286_SEG_CS,
	I286_SEG_SS,
	I286_SEG_DS
};

enum {
	I286_AX = 1,
	I286_CX,
	I286_DX,
	I286_BX,
	I286_IP,
	I286_SP,
	I286_BP,
	I286_SI,
	I286_DI,
	I286_CS,
	I286_SS,
	I286_DS,
	I286_ES,
	I286_MSW,
	I286_GDT,
	I286_LDT,
	I286_IDT
};

void i286_interrupt(CPU*, int);
void i286_rd_ea(CPU*, int);
void i286_wr_ea(CPU*, int, int);
void i286_set_reg(CPU*, int, int);
unsigned char i286_mrd(CPU*, xSegPtr, int, unsigned short);
void i286_mwr(CPU*, xSegPtr, int, unsigned short, int);

void i286_reset(CPU*);
int i286_exec(CPU*);
xAsmScan i286_asm(const char*, char*);
xMnem i286_mnem(CPU*, unsigned short, cbdmr, void*);
void i286_get_regs(CPU*, xRegBunch*);
void i286_set_regs(CPU*, xRegBunch);
