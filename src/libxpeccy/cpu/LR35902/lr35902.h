#ifndef _LR35902_H
#define _LR35902_H

#define FLZ 0x80
#define FLN 0x40
#define FLH 0x20
#define FLC 0x10

typedef struct CPU CPU;

void lr_reset(CPU*);
int lr_exec(CPU*);
int lr_int(CPU*);
int lr_nmi(CPU*);

xAsmScan lr_asm(const char*, char*);
xMnem lr_mnem(unsigned short, cbdmr, void*);

#endif
