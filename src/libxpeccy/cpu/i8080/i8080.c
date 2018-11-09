#include "i8080.h"

// extern opCode i8080_tab[256];

void i8080_reset(CPU* cpu) {
	cpu->pc = 0x0000;
	cpu->bc = cpu->de = cpu->hl = 0xffff;
	cpu->a = 0xff;
	cpu->f = 0x02;
	cpu->inten = 0;
}

int i8080_int(CPU* cpu) {
	return 0;
}

int i8080_exec(CPU* cpu) {
	cpu->t = 0;
	if (cpu->intrq & cpu->inten)
		cpu->t = i8080_int(cpu);
	if (cpu->t) return cpu->t;
	cpu->com = cpu->mrd(cpu->pc++, 1, cpu->data);
//	cpu->op = &i8080_tab[cpu->com];
//	cpu->t = cpu->op->t;
//	cpu->op->exec(cpu);
	return cpu->t;
}
