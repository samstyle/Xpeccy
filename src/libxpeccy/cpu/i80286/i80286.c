#include "i80286.h"

extern opCode i80286_tab[256];

void i80286_reset(CPU* cpu) {
	cpu->flag = 0x0002;
	cpu->msw = 0xfff0;
	cpu->pc = 0xfff0;	// ip
	cpu->cs = 0xf000;
	cpu->ds = 0x0000;
	cpu->ss = 0x0000;
	cpu->es = 0x0000;
	cpu->mode = I286_MOD_REAL;
}

int i80286_exec(CPU* cpu) {
	cpu->t = 1;
	cpu->opTab = i80286_tab;
	cpu->seg = -1;
	do {
		cpu->t++;
		cpu->com = cpu->mrd((cpu->cs << 4) + cpu->pc, 1, cpu->data);
		cpu->pc++;
		cpu->op = &cpu->opTab[cpu->com & 0xff];
		cpu->op->exec(cpu);
	} while (!(cpu->op->flag & OF_PREFIX));

	return cpu->t;
}
