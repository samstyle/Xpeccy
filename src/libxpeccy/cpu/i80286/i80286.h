#pragma once

#include "../cpu.h"

#define regEAX regs[0].i
#define regAX regs[0].w
#define regAH regs[0].h
#define regAL regs[0].l
#define regEBX regs[1].i
#define regBX regs[1].w
#define regBH regs[1].h
#define regBL regs[1].l
#define regECX regs[2].i
#define regCX regs[2].w
#define regCH regs[2].h
#define regCL regs[2].l
#define regEDX regs[3].i
#define regDX regs[3].w
#define regDH regs[3].h
#define regDL regs[3].l
#define regESI regs[4].i
#define regSI regs[4].w
#define regEDI regs[5].i
#define regDI regs[5].w
#define regESP regs[6].i
#define regSP regs[6].w
#define regEIP regs[7].i
#define regIP regs[7].w
#define regBP regs[8].w
#define regIOPL regs[9].l
#define regMSW	regs[10].i
#define regREP	regs[11].l	// repeat condition
#define regMOD	regs[11].h	// mod byte (EA/reg)
// x87 registers
#define regX87top regs[16].l	// x87 top of stack pos (0-7)
#define regX87cr regs[16].ih	// x87 control register
#define regX87sr regs[17].w	// x87 status register
#define regX87tw regs[17].ih	// x87 tag word

// TODO: (?) segment registers:
//	regs[n].i = base addr
//	regs[n+1].i = limit
//	regs[n+2].ih = index
//	regs[n+2].w = flags

// flag
/*
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
*/

#define flgC	flags[0]
#define flgP	flags[2]
#define flgA	flags[4]
#define flgZ	flags[6]
#define flgS	flags[7]
#define flgT	flags[8]
#define flgI	flags[9]
#define flgD	flags[10]
#define flgO	flags[11]
#define flgN	flags[15]
#define flgWRD	flags[32]

enum {
	X86_REAL = 0,
	X86_PROT,
	X86_VIRT8086
};

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
	I286_MOD_PROT,
	X86_MOD_V86		// virtual 8086 mode (386+)
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

void i286_interrupt(CPU*, int);
void i286_rd_ea(CPU*, int);
void i286_wr_ea(CPU*, int, int);
void i286_set_reg(CPU*, int, int);
unsigned char i286_mrd(CPU*, xSegPtr, int, unsigned short);
void i286_mwr(CPU*, xSegPtr, int, unsigned short, int);

void i086_init(CPU*);
void i186_init(CPU*);
void i286_init(CPU*);
void i8086_init(CPU*);
void i286_reset(CPU*);
int i286_exec(CPU*);
xAsmScan i286_asm(int, const char*, char*);
xMnem i286_mnem(CPU*, int, cbdmr, void*);
void i286_get_regs(CPU*, xRegBunch*);
void i286_set_regs(CPU*, xRegBunch);
void x86_set_flag(CPU*, int);
int x86_get_flag(CPU*);

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
