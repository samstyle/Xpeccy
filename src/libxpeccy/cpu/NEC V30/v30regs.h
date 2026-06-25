#pragma once

// nec v30 registers
// +8080 emulation mode

enum {
	V30_REG_AW = 1,
	V30_REG_BW,
	V30_REG_CW,
	V30_REG_DW,
	V30_REG_PC,
	V30_REG_SP,
	V30_REG_BP,
	V30_REG_IX,
	V30_REG_IY,
	V30_SEG_PS,
	V30_SEG_SS,
	V30_SEG_DS0,
	V30_SEG_DS1
};

enum {
	V30_REP_NONE,
	V30_REPNZ,
	V30_REPZ,
	V30_REPNC,
	V30_REPC
};

#define regAW	regs[0].w
#define regAH	regs[0].h
#define regAL	regs[0].l
#define regA	regAL

#define regBW	regs[1].w
#define regBH	regs[1].h
#define regBL	regs[1].l
#define regHL	regBW
#define regH	regBH
#define regL	regBL

#define regCW	regs[2].w
#define regCH	regs[2].h
#define regCL	regs[2].l
#define regBC	regCW
#define regB	regCH
#define regC	regCL

#define regDW	regs[3].w
#define regDH	regs[3].h
#define regDL	regs[3].l
#define regDE	regDW
#define regD	regDH
#define regE	regDL

#define regSP	regs[4].w
#define regSPh	regs[4].h
#define regSPl	regs[4].l

#define regBP	regs[5].w
#define regBPh	regs[5].h
#define regBPl	regs[5].l

#define regIX	regs[6].w
#define regIXh	regs[6].h
#define regIXl	regs[6].l

#define regIY	regs[7].w
#define regIYh	regs[7].h
#define regIYl	regs[7].l

#define regPC	regs[8].w
#define regPCh	regs[8].h
#define regPCl	regs[8].l

#define regTA	regs[9].ih
#define regTB	regs[9].w
#define regTC	regs[10].ih
#define regLC	regs[10].w

// segment regsiters (only values)
#define regPS	regs[11].ih
#define regSS	regs[11].w
#define regDS0	regs[12].ih
#define regDS1	regs[12].w
#define regSEG	regs[13].ih	// segment overwrite (if flgSOVR)
#define regMOD	regs[13].l	// mod r/m byte
#define regREP	regs[14].ih	// repeat type

#define regWZ	regs[15].w
#define regWZh	regs[15].h
#define regWZl	regs[15].l

// flags

#define flgCY	flags[0]
#define flgC	flgCY
#define flgF1	flags[1]
#define flgP	flags[2]
#define	flgF3	flags[3]
#define flgAC	flags[4]
#define flgA	flgAC
#define flgF5	flags[5]
#define flgZ	flags[6]
#define flgS	flags[7]
#define flgBRK	flags[8]
#define flgIE	flags[9]
#define flgIFF1	flgIE
#define flgDIR	flags[10]
#define flgV	flags[11]
#define flgMD	flags[15]
#define flgSOVR	flags[16]	// overwrite segment (DS0) with cpu->regSEG
#define flgBNMI	flags[17]	// block NMI
#define flgBLKM	flags[18]	// block MD flag change
