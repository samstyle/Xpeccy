#ifndef X_I8080_H
#define X_I8080_H

#include "../cpu.h"

#define I8080_INT	1

#define IFL_S	0x80
#define	IFL_Z	0x40
#define IFL_A	0x10
#define IFL_P	0x04
#define IFL_C	0x01

extern opCode i8080_tab[256];

void i8080_reset(CPU*);
int i8080_exec(CPU*);

#endif
