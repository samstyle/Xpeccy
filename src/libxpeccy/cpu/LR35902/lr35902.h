#ifndef _LR35902_H
#define _LR35902_H

#include "../cpu.h"

// typedef struct CPU CPU;

void lr_reset(CPU*);
int lr_exec(CPU*);
int lr_int(CPU*);
int lr_nmi(CPU*);

xAsmScan lr_asm(const char*, char*);
xMnem lr_mnem(unsigned short, cbdmr, void*);

#endif
