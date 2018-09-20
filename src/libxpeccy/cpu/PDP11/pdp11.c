#include "pdp11.h"
#include <string.h>

void pdp11_reset(CPU* cpu) {

}

int pdp11_int(CPU* cpu) {

	return 1;
}

typedef struct {
	int mask;
	int val;
	void(*exec)(CPU*);
} xpdp11com;

/*
static xpdp11com pdp_tab[] = {
	{0x7000, 0x1000, pdp_mov},
	{0x7000, 0x2000, pdp_cmp},
	{0x7000, 0x3000, pdp_bit},
	{0x7000, 0x4000, pdp_bic},
	{0x7000, 0x5000, pdp_bis},
	{0, 0, NULL}
};
*/

int pdp11_exec(CPU* cpu) {
	// tmpw = command
	cpu->lcom = cpu->mrd(cpu->pc++, 1, cpu->data);
	cpu->hcom = cpu->mrd(cpu->pc++, 1, cpu->data);
	switch (cpu->com & 0x7000) {
		case 0x1000:		// B1SSDD:mov
			break;
		case 0x2000:		// B2SSDD:cmp
			break;
		case 0x3000:		// B3SSDD:bit (and)
			break;
		case 0x4000:		// B4SSDD:bic (and not)
			break;
		case 0x5000:		// B4SSDD:bis (or)
			break;
		default:
			switch (cpu->com & 0xf000) {
				case 0x6000:	// 06SSDD:add
					break;
				case 0xe000:	// 16SSDD:sub
					break;
			}
	}

	return 1;
}

// registers

static xRegDsc pdp11RegTab[] = {
	{PDP11_REG0, "R0", 0},
	{PDP11_REG1, "R1", 0},
	{PDP11_REG2, "R2", 0},
	{PDP11_REG3, "R3", 0},
	{PDP11_REG4, "R4", 0},
	{PDP11_REG5, "R5", 0},
	{PDP11_REG6, "R6", 0},
	{PDP11_REG7, "R7", 0},
	{PDP11_REGF, "F", 0},
	{REG_NONE, "", 0}
};

void pdp11_get_regs(CPU* cpu, xRegBunch* bunch) {
	int idx = 0;
	while (pdp11RegTab[idx].id != REG_NONE) {
		bunch->regs[idx].id = pdp11RegTab[idx].id;
		strncpy(bunch->regs[idx].name, pdp11RegTab[idx].name, 7);
		bunch->regs[idx].byte = pdp11RegTab[idx].byte;
		switch(pdp11RegTab[idx].id) {
			case PDP11_REG0: bunch->regs[idx].value = cpu->preg[0]; break;
			case PDP11_REG1: bunch->regs[idx].value = cpu->preg[1]; break;
			case PDP11_REG2: bunch->regs[idx].value = cpu->preg[2]; break;
			case PDP11_REG3: bunch->regs[idx].value = cpu->preg[3]; break;
			case PDP11_REG4: bunch->regs[idx].value = cpu->preg[4]; break;
			case PDP11_REG5: bunch->regs[idx].value = cpu->preg[5]; break;
			case PDP11_REG6: bunch->regs[idx].value = cpu->preg[6]; break;
			case PDP11_REG7: bunch->regs[idx].value = cpu->preg[7]; break;
			case PDP11_REGF: bunch->regs[idx].value = cpu->pflag; break;
		}
	}
}

void pdp11_set_regs(CPU* cpu, xRegBunch bunch) {

}

// asm/disasm

xMnem pdp11_mnem(CPU* cpu, unsigned short adr, cbdmr crd, void* dat) {
	xMnem res;
	res.len = 1;

	return res;
}

xAsmScan pdp11_asm(const char* mnm, char* buf) {
	xAsmScan res;
	res.match = 0;

	return res;
}
