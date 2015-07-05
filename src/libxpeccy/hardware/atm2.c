#include "../spectrum.h"

// TODO : fill memMap & set prt1 for reset to separate ROM pages
void atm2Reset(ZXComp* comp) {
	comp->dos = 1;
}

void atmSetBank(ZXComp* comp, int bank, memEntry me) {
	unsigned char page = me.page ^ 0xff;
	if (me.flag & 0x80) {
		if (me.flag & 0x40) {
			page = (page & 0x38) | (comp->p7FFD & 7);	// mix with 7FFD bank;
		} else {
			page = (page & 0x3e) | (comp->dos ? 1 : 0);	// mix with dosen
		}
	}
	memSetBank(comp->mem,bank,(me.flag & 0x40) ? MEM_RAM : MEM_ROM, page);
}

void atm2MapMem(ZXComp* comp) {
	if (comp->p77hi & 1) {			// pen = 0: last rom page in every bank && dosen on
		int adr = (comp->rom) ? 4 : 0;
		atmSetBank(comp,MEM_BANK0,comp->memMap[adr]);
		atmSetBank(comp,MEM_BANK1,comp->memMap[adr+1]);
		atmSetBank(comp,MEM_BANK2,comp->memMap[adr+2]);
		atmSetBank(comp,MEM_BANK3,comp->memMap[adr+3]);
	} else {
		comp->dos = 1;
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK1,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK2,MEM_ROM,0xff);
		memSetBank(comp->mem,MEM_BANK3,MEM_ROM,0xff);
	}
}

// out

void atm2Out77(ZXComp* comp, unsigned short port, unsigned char val) {		// dos
	switch (val & 7) {
		case 0: vidSetMode(comp->vid,VID_ATM_EGA); break;
		case 2: vidSetMode(comp->vid,VID_ATM_HWM); break;
		case 3: vidSetMode(comp->vid,VID_NORMAL); break;
		case 6: vidSetMode(comp->vid,VID_ATM_TEXT); break;
		default: vidSetMode(comp->vid,VID_UNKNOWN); break;
	}
	zxSetFrq(comp,(val & 0x08) ? 7.0 : 3.5);
	comp->p77hi = (port & 0xff00) >> 8;
	atm2MapMem(comp);
}

void atm2OutF7(ZXComp* comp, unsigned short port, unsigned char val) {		// dos
	int adr = ((comp->rom) ? 4 : 0) | ((port & 0xc000) >> 14);	// rom2.a15.a14
	comp->memMap[adr].flag = val & 0xc0;		// copy b6,7 to flag
	comp->memMap[adr].page = (val & 0x3f) | 0xc0;	// set b6,7 for PentEvo capability
	atm2MapMem(comp);
}

void atm2OutFB(ZXComp* comp, unsigned short port, unsigned char val) {
	sdrvOut(comp->sdrv, port, val);
}

/*
void atm2OutFE(ZXComp* comp, unsigned short port, unsigned char val) {
	comp->vid->nextbrd = (val & 0x07) | (~port & 8);
	if (!comp->vid->border4t)
		comp->vid->brdcol = comp->vid->nextbrd;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}
*/

void atm2Out7FFD(ZXComp* comp, unsigned short port, unsigned char val) {
	if (comp->p7FFD & 0x20) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	atm2MapMem(comp);
}

void atm2OutFF(ZXComp* comp, unsigned short port, unsigned char val) {		// dos. bdiOut already done
	if (comp->p77hi & 0x40) return;
	val ^= 0xff;	// inverse colors
	int adr = comp->vid->brdcol & 0x0f;
	comp->vid->pal[adr].b = ((val & 0x01) ? 0xaa : 0x00) + ((val & 0x20) ? 0x55 : 0x00);
	comp->vid->pal[adr].r = ((val & 0x02) ? 0xaa : 0x00) + ((val & 0x40) ? 0x55 : 0x00);
	comp->vid->pal[adr].g = ((val & 0x10) ? 0xaa : 0x00) + ((val & 0x80) ? 0x55 : 0x00);
}

xPort atm2PortMap[] = {
	{0x0007,0x00fe,2,2,2,xInFE,	xOutFE},
	{0x0007,0x00fa,2,2,2,NULL,	NULL},		// fa
	{0x0007,0x00fb,2,2,2,NULL,	atm2OutFB},	// fb (covox)
	{0x8202,0x7ffd,2,2,2,NULL,	atm2Out7FFD},
	{0x8202,0x7dfd,2,2,2,NULL,	NULL},		// 7DFD
	{0xc202,0xbffd,2,2,2,NULL,	xOutBFFD},	// ay
	{0xc202,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	// dos
	{0x009f,0x00ff,1,2,2,NULL,	atm2OutFF},	// palette (dos)
	{0x009f,0x00f7,1,2,2,NULL,	atm2OutF7},
	{0x009f,0x0077,1,2,2,NULL,	atm2Out77},
	{0x0000,0x0000,2,2,2,NULL,	NULL}
};

void atm2Out(ZXComp* comp, unsigned short port, unsigned char val, int dos) {
	if (~comp->p77hi & 2) dos = 1;
	difOut(comp->dif, port, val, dos);
	hwOut(atm2PortMap, comp, port, val, dos);
}

unsigned char atm2In(ZXComp* comp, unsigned short port, int dos) {
	unsigned char res = 0xff;
	if (~comp->p77hi & 2) dos = 1;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(atm2PortMap, comp, port, dos);
	return res;
}
