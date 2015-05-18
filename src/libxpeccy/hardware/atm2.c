#include "../spectrum.h"

// TODO : fill memMap & set prt1 for reset to separate ROM pages
void atm2Reset(ZXComp* comp) {
	comp->dosen = 1;
}

void atmSetBank(ZXComp* comp, int bank, memEntry me) {
	unsigned char page = me.page ^ 0xff;
	if (me.flag & 0x80) {
		if (me.flag & 0x40) {
			page = (page & 0x38) | (comp->prt0 & 7);	// mix with 7FFD bank;
		} else {
			page = (page & 0x3e) | (comp->dosen ? 1 : 0);	// mix with dosen
		}
	}
	memSetBank(comp->mem,bank,(me.flag & 0x40) ? MEM_RAM : MEM_ROM, page);
}

void atm2MapMem(ZXComp* comp) {
	if (comp->prt1 & 1) {			// pen = 0: last rom page in every bank && dosen on
		int adr = (comp->prt0 & 0x10) ? 4 : 0;
		atmSetBank(comp,MEM_BANK0,comp->memMap[adr]);
		atmSetBank(comp,MEM_BANK1,comp->memMap[adr+1]);
		atmSetBank(comp,MEM_BANK2,comp->memMap[adr+2]);
		atmSetBank(comp,MEM_BANK3,comp->memMap[adr+3]);
	} else {
		comp->dosen = 1;
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK1,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK2,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK3,MEM_ROM,0xff);
	}
}

// out

void atm2Out77(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {		// dos
	switch (val & 7) {
		case 0: vidSetMode(comp->vid,VID_ATM_EGA); break;
		case 2: vidSetMode(comp->vid,VID_ATM_HWM); break;
		case 3: vidSetMode(comp->vid,VID_NORMAL); break;
		case 6: vidSetMode(comp->vid,VID_ATM_TEXT); break;
		default: vidSetMode(comp->vid,VID_UNKNOWN); break;
	}
	zxSetFrq(comp,(val & 0x08) ? 7.0 : 3.5);
	comp->prt1 = (port & 0xff00) >> 8;
	atm2MapMem(comp);
}

void atm2OutF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {		// dos
	int adr = ((comp->prt0 & 0x10) ? 4 : 0) | ((port & 0xc000) >> 14);	// rom2.a15.a14
	comp->memMap[adr].flag = val & 0xc0;		// copy b6,7 to flag
	comp->memMap[adr].page = (val & 0x3f) | 0xc0;	// set b6,7 for PentEvo capability
	atm2MapMem(comp);
}

void atm2OutFB(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	sdrvOut(comp->sdrv, port, val);
}

/*
void atm2OutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->nextbrd = (val & 0x07) | (~port & 8);
	if (!comp->vid->border4t)
		comp->vid->brdcol = comp->vid->nextbrd;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}
*/

void atm2Out7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (comp->prt0 & 0x20) return;
	comp->prt0 = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	atm2MapMem(comp);
}

void atm2OutFF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {		// dos. bdiOut already done
	if (comp->prt1 & 0x40) return;
	val ^= 0xff;	// inverse colors
	int adr = comp->vid->brdcol & 0x0f;
	comp->vid->pal[adr].b = ((val & 0x01) ? 0xaa : 0x00) + ((val & 0x20) ? 0x55 : 0x00);
	comp->vid->pal[adr].r = ((val & 0x02) ? 0xaa : 0x00) + ((val & 0x40) ? 0x55 : 0x00);
	comp->vid->pal[adr].g = ((val & 0x10) ? 0xaa : 0x00) + ((val & 0x80) ? 0x55 : 0x00);
}

xPort atm2PortMap[] = {
	{0x0007,0x00fe,2,&xInFE,	&xOutFE},
	{0x0007,0x00fa,2,NULL,		NULL},		// fa
	{0x0007,0x00fb,2,NULL,		&atm2OutFB},	// fb (covox)
	{0x8202,0x7ffd,2,NULL,		&atm2Out7FFD},
	{0x8202,0x7dfd,2,NULL,		NULL},		// 7DFD
	{0xc202,0xbffd,2,NULL	,	&xOutBFFD},	// ay
	{0xc202,0xfffd,2,&xInFFFD,	&xOutFFFD},
	// dos
	{0x009f,0x00ff,1,NULL,		&atm2OutFF},	// palette (dos)
	{0x009f,0x00f7,1,NULL,		&atm2OutF7},
	{0x009f,0x0077,1,NULL,		&atm2Out77},
	{0x0000,0x0000,2,NULL,		NULL}
};

void atm2Out(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val, int dos) {
	if (~comp->prt1 & 2) dos = 1;
	difOut(comp->dif, port, val, dos);
	hwOut(atm2PortMap, comp, port, val, dos);
}

Z80EX_BYTE atm2In(ZXComp* comp, Z80EX_WORD port, int dos) {
	Z80EX_BYTE res = 0xff;
	if (~comp->prt1 & 2) dos = 1;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(atm2PortMap, comp, port, dos);
	return res;
}
