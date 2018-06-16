#include "../spectrum.h"

#include <time.h>

// TODO : fill memMap & set prt1 for reset to separate ROM pages
void atm2Reset(Computer* comp) {
	comp->dos = 1;
	comp->p77hi = 0;
	kbdSetMode(comp->keyb, KBD_ATM2);
	comp->keyb->submode = kbdZX;
	comp->keyb->wcom = 0;
	comp->keyb->warg = 0;
	kbdReleaseAll(comp->keyb);
}

void atmSetBank(Computer* comp, int bank, memEntry me) {
	unsigned char page = me.page ^ 0xff;
	if (me.flag & 0x80) {
		if (me.flag & 0x40) {
			page = (page & 0x38) | (comp->p7FFD & 7);	// mix with 7FFD bank;
		} else {
			page = (page & 0x3e) | (comp->dos ? 1 : 0);	// mix with dosen
		}
	}
	memSetBank(comp->mem, bank, (me.flag & 0x40) ? MEM_RAM : MEM_ROM, page, MEM_16K, NULL, NULL, NULL);
}

void atm2MapMem(Computer* comp) {
	if (comp->p77hi & 1) {			// pen = 0: last rom page in every bank && dosen on
		int adr = (comp->rom) ? 4 : 0;
		atmSetBank(comp,0x00,comp->memMap[adr]);
		atmSetBank(comp,0x40,comp->memMap[adr+1]);
		atmSetBank(comp,0x80,comp->memMap[adr+2]);
		atmSetBank(comp,0xc0,comp->memMap[adr+3]);
	} else {
		comp->dos = 1;
		memSetBank(comp->mem,0x00,MEM_ROM,0xff,MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x40,MEM_ROM,0xff,MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x80,MEM_ROM,0xff,MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0xc0,MEM_ROM,0xff,MEM_16K, NULL, NULL, NULL);
	}
}

// out

void atm2OutFE(Computer* comp, unsigned short port, unsigned char val) {
	xOutFE(comp, port, val);
	if (~port & 0x08) {		// A3 = 0 : bright border
		comp->vid->nextbrd |= 0x08;
		comp->vid->brdcol = comp->vid->nextbrd;
	}
}

void atm2Out77(Computer* comp, unsigned short port, unsigned char val) {		// dos
	switch (val & 7) {
		case 0: vidSetMode(comp->vid,VID_ATM_EGA); break;
		case 2: vidSetMode(comp->vid,VID_ATM_HWM); break;
		case 3: vidSetMode(comp->vid,VID_NORMAL); break;
		case 6: vidSetMode(comp->vid,VID_ATM_TEXT); break;
		default: vidSetMode(comp->vid,VID_UNKNOWN); break;
	}
	compSetTurbo(comp,(val & 0x08) ? 2 : 1);
	comp->keyb->mode = (val & 0x40) ? KBD_SPECTRUM : KBD_ATM2;
	comp->p77hi = (port & 0xff00) >> 8;
	atm2MapMem(comp);
}

void atm2OutF7(Computer* comp, unsigned short port, unsigned char val) {		// dos
	int adr = (comp->rom ? 4 : 0) | ((port & 0xc000) >> 14);	// rom2.a15.a14
	comp->memMap[adr].flag = val & 0xc0;		// copy b6,7 to flag
	comp->memMap[adr].page = (val & 0x3f) | 0xc0;	// set b6,7 for PentEvo capability
	atm2MapMem(comp);
}

void atm2Out7FFD(Computer* comp, unsigned short port, unsigned char val) {
	if (comp->p7FFD & 0x20) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	atm2MapMem(comp);
}

void atm2OutFF(Computer* comp, unsigned short port, unsigned char val) {		// dos. bdiOut already done
	if (comp->p77hi & 0x40) return;			// pen2 = 1
	val ^= 0xff;	// inverse colors
	int adr = comp->vid->brdcol & 0x0f;
	// printf("%.2X : %.2X\n",adr, val);
	comp->vid->pal[adr].b = ((val & 0x01) ? 0xaa : 0x00) + ((val & 0x20) ? 0x55 : 0x00);
	comp->vid->pal[adr].r = ((val & 0x02) ? 0xaa : 0x00) + ((val & 0x40) ? 0x55 : 0x00);
	comp->vid->pal[adr].g = ((val & 0x10) ? 0xaa : 0x00) + ((val & 0x80) ? 0x55 : 0x00);
}

// in

unsigned char atm2inFE(Computer* comp, unsigned short port) {
	unsigned char res = kbdRead(comp->keyb, port);
	if (comp->keyb->mode == KBD_SPECTRUM) {
		res |= (comp->tape->volPlay & 0x80) ? 0x40 : 0x00;
	} else if (comp->keyb->submode == kbdZX) {
		res |= (comp->tape->volPlay & 0x80) ? 0x40 : 0x00;
	}
	return res;
}

static xPort atm2PortMap[] = {
	{0x0007,0x00fe,2,2,2,atm2inFE,	atm2OutFE},
	{0x0007,0x00fa,2,2,2,NULL,	NULL},		// fa
//	{0x0007,0x00fb,2,2,2,NULL,	atm2OutFB},	// fb (covox)
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

void atm2Out(Computer* comp, unsigned short port, unsigned char val, int dos) {
	if (~comp->p77hi & 2) dos = 1;
	zx_dev_wr(comp, port, val, dos);
	difOut(comp->dif, port, val, dos);
	hwOut(atm2PortMap, comp, port, val, dos);
}

unsigned char atm2In(Computer* comp, unsigned short port, int dos) {
	unsigned char res = 0xff;
	if (~comp->p77hi & 2) dos = 1;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(atm2PortMap, comp, port, dos);
	return res;
}

void atm2_keyp(Computer* comp, keyEntry kent) {
	kbdPress(comp->keyb, kent);
}

void atm2_keyr(Computer* comp, keyEntry kent) {
	kbdRelease(comp->keyb, kent);
}
