#ifndef _Z80_H
#define _Z80_H

#include "../cpu.h"

#define FS	0x80
#define	FZ	0x40
#define	F5	0x20
#define	FH	0x10
#define	F3	0x08
#define	FP	0x04
#define	FV	FP
#define	FN	0x02
#define	FC	0x01

typedef struct CPU CPU;

void z80_reset(CPU*);
int z80_exec(CPU*);
int z80_int(CPU*);
int z80_nmi(CPU*);
xAsmScan z80_asm(const char*, char*);
xMnem z80_mnem(unsigned short, cbdmr, void*);

#endif
