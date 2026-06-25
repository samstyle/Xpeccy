#pragma once

#include "../cpu.h"

#define V30_INT	(1<<0)
#define V30_NMI	(1<<1)
// INT numbers
#define V30_INT_DE	0
#define V30_INT_DB	1
#define V30_INT_NMI	2
#define V30_INT_BP	3
#define V30_INT_OF	4
#define V30_INT_BR	5
#define V30_INT_UD	6
#define V30_INT_NM	7
#define V30_INT_DF	8
#define V30_INT_SL	9
#define V30_INT_TS	10
#define V30_INT_NP	11
#define V30_INT_SS	12
#define V30_INT_GP	13
#define V30_INT_MF	16

void v30_reset(CPU*);
int v30_exec(CPU*);
xAsmScan v30_asm(int, const char*, char*);
xMnem v30_mnem(CPU*, int, cbdmr, void*);

void v30_setflag(CPU*, int);
int v30_getflag(CPU*);
