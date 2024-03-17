#pragma once

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

enum {
	X86_REAL = 0,
	X86_PROT
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
	unsigned i:1;
	unsigned d:1;
	unsigned o:1;
	unsigned iopl:2;
	unsigned n:1;
	unsigned md:1;
	unsigned _nu:16;
} x86flag_t;
#else
typedef struct {
	unsigned _nu:16;
	unsigned md:1;
	unsigned n:1;
	unsigned iopl:2;
	unsigned o:1;
	unsigned d:1;
	unsigned i:1;
	unsigned s:1;
	unsigned z:1;
	unsigned f5:1;
	unsigned a:1;
	unsigned f3:1;
	unsigned p:1;
	unsigned f1:1;
	unsigned c:1;
} x86flag_t;
#endif

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
	I286_IDT,
	I286_TSS
};

#include "../cpu.h"

void i286_interrupt(CPU*, int);
void i286_rd_ea(CPU*, int);
void i286_wr_ea(CPU*, int, int);
void i286_set_reg(CPU*, int, int);
unsigned char i286_mrd(CPU*, xSegPtr, int, unsigned short);
void i286_mwr(CPU*, xSegPtr, int, unsigned short, int);

void i286_init(CPU*);
void i8086_init(CPU*);
void i286_reset(CPU*);
int i286_exec(CPU*);
xAsmScan i286_asm(const char*, char*);
xMnem i286_mnem(CPU*, int, cbdmr, void*);
void i286_get_regs(CPU*, xRegBunch*);
void i286_set_regs(CPU*, xRegBunch);

void x86_set_mode(CPU*, int);

// ALU
unsigned char i286_add8(CPU*, unsigned char, unsigned char, int);
unsigned short i286_add16(CPU*, unsigned short, unsigned short, int);
unsigned char i286_sub8(CPU*, unsigned char, unsigned char, int);
unsigned short i286_sub16(CPU*, unsigned short, unsigned short, int);
int i286_smul(CPU*, signed short, signed short);
unsigned char i286_and8(CPU*, unsigned char, unsigned char);
unsigned short i286_and16(CPU*, unsigned short, unsigned short);
unsigned char i286_or8(CPU*, unsigned char, unsigned char);
unsigned short i286_or16(CPU*, unsigned short, unsigned short);
unsigned char i286_xor8(CPU*, unsigned char, unsigned char);
unsigned short i286_xor16(CPU*, unsigned short, unsigned short);

// shift/rotation
unsigned char i286_rol8(CPU* cpu, unsigned char);
unsigned short i286_rol16(CPU*, unsigned short);
unsigned char i286_ror8(CPU*, unsigned char);
unsigned short i286_ror16(CPU*, unsigned short);
unsigned char i286_rcl8(CPU*, unsigned char);
unsigned short i286_rcl16(CPU*, unsigned short);
unsigned char i286_rcr8(CPU*, unsigned char);
unsigned short i286_rcr16(CPU*, unsigned short);
unsigned char i286_sal8(CPU*, unsigned char);
unsigned short i286_sal16(CPU*, unsigned short);
unsigned char i286_shr8(CPU*, unsigned char);
unsigned short i286_shr16(CPU*, unsigned short);
unsigned char i286_sar8(CPU*, unsigned char);
unsigned short i286_sar16(CPU*, unsigned short);
