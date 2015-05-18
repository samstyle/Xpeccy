#include "../spectrum.h"
#include <assert.h>

void prfReset(ZXComp* comp) {

}

void prfMapMem(ZXComp* comp) {
	if (comp->prt1 & 0x10) {
		memSetBank(comp->mem, MEM_BANK0, MEM_RAM, 0);
	} else {
		memSetBank(comp->mem, MEM_BANK0, MEM_ROM, (comp->dosen ? 0 : 2) | (comp->prt0 & 0x10 ? 1 : 0));
	}
	memSetBank(comp->mem, (comp->prt1 & 0x08) ? MEM_BANK1 : MEM_BANK3, MEM_RAM, ((comp->prt1 & 7) << 3) | (comp->prt0 & 7));
}

Z80EX_BYTE brkIn(ZXComp* comp, Z80EX_WORD port) {
	printf("Profi IN %.4X\n",port);
	//assert(0);
	comp->brk = 1;
	return 0xff;
}

void brkOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	printf("Profi OUT %.4X,%.2X\n",port,val);
//	assert(0);
	comp->brk = 1;
}


Z80EX_BYTE prfInF7(ZXComp* comp, Z80EX_WORD port) {
	return 0xff;
}

void prfOutF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
}

void prfOut7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (!(comp->prt1 & 0x10) && (comp->prt0 & 0x20)) return;	// 7FFD is blocked
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	prfMapMem(comp);
}

void prfOutDFFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->prt1 = val;
}

xPort prfPortMap[] = {
	{0x00f7,0x00fe,2,&xInFE,	&xOutFE},		// fe, f6 (dos)
	{0x00ff,0x00f7,2,&prfInF7,	&prfOutF7},
	{0x8002,0x7ffd,2,NULL,		&prfOut7FFD},
	{0xffff,0xdffd,2,NULL,		&prfOutDFFD},
	{0xc002,0xbffd,2,NULL,		&xOutBFFD},
	{0xc002,0xfffd,2,&xInFFFD,	&xOutFFFD},
	{0x0000,0x0000,2,&brkIn,	&brkOut}
};

void prfOut(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	if (comp->prt1 & 0x20) dos = 1;
	if (difOut(comp->dif, port, val, dos)) return;
	hwOut(prfPortMap, comp, port, val, dos);
}

Z80EX_BYTE prfIn(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res = 0xff;
	if (comp->prt1 & 0x20) dos = 1;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(prfPortMap, comp, port, dos);
	return res;
}
