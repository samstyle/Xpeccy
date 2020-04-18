#include "../spectrum.h"

#include <stdio.h>

void evoReset(Computer* comp) {
	comp->dos = 1;
	comp->prt2 = 0x03;
}

void evoSetVideoMode(Computer* comp) {
	int mode = (comp->pEFF7 & 0x20) | ((comp->pEFF7 & 0x01) << 1) | (comp->prt2 & 0x07);	// z5.z0.0.b2.b1.b0	b:FF77, z:eff7
	switch (mode) {
		case 0x03: vidSetMode(comp->vid,VID_NORMAL); break;		// common
		case 0x13: vidSetMode(comp->vid,VID_ALCO); break;		// alco 16c
		case 0x23: vidSetMode(comp->vid,VID_HWMC); break;		// zx hardware multicolor
		case 0x02: vidSetMode(comp->vid,VID_ATM_HWM); break;	// atm hardware multicolor
		case 0x00: vidSetMode(comp->vid,VID_ATM_EGA); break;	// atm ega
		case 0x06: vidSetMode(comp->vid,VID_ATM_TEXT); break;	// atm text
		case 0x07: vidSetMode(comp->vid,VID_EVO_TEXT); break;	// pentevo text
		default: vidSetMode(comp->vid,VID_UNKNOWN); break;
	}
}

int evoMRd(Computer* comp, int adr, int m1) {
	if (m1 && (comp->dif->type == DIF_BDI)) {
		if (comp->dos && (comp->mem->map[(adr >> 8) & 0xff].type == MEM_RAM) && (comp->prt2 & 0x40)) {
			comp->dos = 0;
			if (comp->rom) comp->hw->mapMem(comp);
		}
		if (!comp->dos && ((adr & 0xff00) == 0x3d00) && (comp->rom)) {
			comp->dos = 1;
			comp->hw->mapMem(comp);
		}
	}
	return memRd(comp->mem,adr);
}

void evoMWr(Computer* comp, int adr, int val) {
	if (comp->evo.evoBF & 4) comp->vid->font[adr & 0x7ff] = val & 0xff;	// PentEvo: write font byte
	memWr(comp->mem,adr,val);
}

void evoSetBank(Computer* comp, int bank, memEntry me) {
	unsigned char page = me.page ^ 0xff;
	if (me.flag & 0x80) {
		if (me.flag & 0x40) {
			if (comp->pEFF7 & 4) {
				page = (page & 0xf8) | (comp->p7FFD & 7);				// mix with b0..2 (7FFD) - 128K mode
			} else {
				page = (page & 0xc0) | (comp->p7FFD & 7) | ((comp->p7FFD & 0xe0) >> 2);	// mix with b0..2,5..7 (7FFD) - P1024 mode
			}
		} else {
			page = (page & 0x3e) | (comp->dos ? 1 : 0);				// mix with dosen
		}
	}
	memSetBank(comp->mem, bank, (me.flag & 0x40) ? MEM_RAM : MEM_ROM, page, MEM_16K, NULL, NULL, NULL);
}

void evoMapMem(Computer* comp) {
	if (comp->prt2 & 0x20) {		// A8.xx77
		int adr = (comp->rom) ? 4 : 0;
		evoSetBank(comp,0x00,comp->memMap[adr]);
		evoSetBank(comp,0x40,comp->memMap[adr+1]);
		evoSetBank(comp,0x80,comp->memMap[adr+2]);
		evoSetBank(comp,0xc0,comp->memMap[adr+3]);
	} else {
		comp->dos = 1;
		memSetBank(comp->mem,0x00,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x40,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0x80,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
		memSetBank(comp->mem,0xc0,MEM_ROM,0xff, MEM_16K, NULL, NULL, NULL);
	}
	if (comp->pEFF7 & 8)				// b3.EFF7: ram0 @ 0x0000 : high priority
		memSetBank(comp->mem,0x00,MEM_RAM,0x00, MEM_16K, NULL, NULL, NULL);
}

// in

int evoIn1F(Computer* comp, int port) {	// !dos
	return joyInput(comp->joy);
}

int evoIn57(Computer* comp, int port) {	// !dos
	return sdcRead(comp->sdc);
}

int evoIn77(Computer* comp, int port) {	// !dos
	int res = 0x02;		// rd only
	if (comp->sdc->image != NULL) res |= 0x01;
	return res;
}

int evoInBE(Computer* comp, int port) {
	int res = -1;
	int i;
	if ((port & 0xf800) == 0x0000) {
		res = comp->memMap[(port & 0x0700) >> 8].page;
	} else {
		switch (port & 0xff00) {
			case 0x0800:
				res = 0;
				for (i = 0; i < 8; i++) {
					res = (res >> 1);
					if (comp->memMap[i].flag & 0x40) res |= 0x80;
				}
				break;
			case 0x0900:
				res = 0;
				for (i = 0; i < 8; i++) {
					res = (res >> 1);
					if (comp->memMap[i].flag & 0x80) res |= 0x80;
				}
				break;
			case 0x0a00: res = comp->p7FFD; break;
			case 0x0b00: res = comp->pEFF7; break;
			case 0x0c00: res = comp->prt2 | (comp->dos ? 0x10 : 0x00); break;
			default:
//				printf("PentEvo\tin %.4X.%i\n",port,bdiz);
//				assert(0);
				break;
		}
	}
	return res;
}

int evoInBF(Computer* comp, int port) {
	return comp->evo.evoBF;
}

int evoInBDI(Computer* comp, int port) {
	int res = -1;
	difIn(comp->dif, port, &res, 1);
	return res;
}

int evoIn2F(Computer* comp, int port) {
	return comp->evo.evo2F;
}

int evoIn4F(Computer* comp, int port) {
	return comp->evo.evo4F;
}

int evoIn6F(Computer* comp, int port) {
	return comp->evo.evo6F;
}

int evoIn8F(Computer* comp, int port) {
	return comp->evo.evo8F;
}

int evoInBEF7(Computer* comp, int port) {	// dos
	return cmsRd(comp);
}

int evoInBFF7(Computer* comp, int port) {	// !dos
	return (comp->pEFF7 & 0x80) ? cmsRd(comp) : 0xff;
}

// out

void evoOutBF(Computer* comp, int port, int val) {
	comp->evo.evoBF = val & 0xff;
}

void evoOut2F(Computer* comp, int port, int val) {
	comp->evo.evo2F = val & 0xff;
}

void evoOut4F(Computer* comp, int port, int val) {
	comp->evo.evo4F = val & 0xff;
}

void evoOut6F(Computer* comp, int port, int val) {
	comp->evo.evo6F = val & 0xff;
}

void evoOut8F(Computer* comp, int port, int val) {
	comp->evo.evo8F = val & 0xff;
}

void evoOut57(Computer* comp, int port, int val) {
	sdcWrite(comp->sdc, val);
}

void evoOut77(Computer* comp, int port, int val) {
	comp->sdc->on = val & 1;
	comp->sdc->cs = (val & 2) ? 1 : 0;
}

void evoOut77d(Computer* comp, int port, int val) {
	comp->prt2 = ((port & 0x4000) >> 7) | ((port & 0x0300) >> 3) | (val & 0x0f);	// a14.a9.a8.0.b3.b2.b1.b0
	compSetTurbo(comp,(val & 0x08) ? 4 : ((comp->pEFF7 & 0x10) ? 1 : 2));
	evoSetVideoMode(comp);
	evoMapMem(comp);
}

void evoOutF7(Computer* comp, int port, int val) {
	int adr = ((comp->rom) ? 4 : 0) | ((port & 0xc000) >> 14);
	if (port & 0x0800) {
		comp->memMap[adr].flag = val & 0xc0;
		comp->memMap[adr].page = (val & 0xff) | 0xc0;
	} else {
		comp->memMap[adr].flag |= 0x40;		// ram
		comp->memMap[adr].page = val & 0xff;
	}
	evoMapMem(comp);
}

void evoOutBDI(Computer* comp, int port, int val) {		// dos
	difOut(comp->dif, port, val, 1);
}

static const unsigned char atm3clev[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};

void evoOutFF(Computer* comp, int port, int val) {		// dos
	difOut(comp->dif, 0xff, val, 1);
	if (comp->prt2 & 0x80) return;
	val ^= 0xff;	// inverse colors
	int adr = comp->vid->brdcol & 0x0f;
#if 1			// ddp extend palete
	port ^= 0xff00;
	if (!comp->ddpal) port = (port & 0xff) | ((val << 8) & 0xff00);
	comp->vid->pal[adr].b = atm3clev[((val & 0x01) << 3) | ((val & 0x20) >> 3) | ((port & 0x0100) >> 7) | ((port & 0x2000) >> 13)];
	comp->vid->pal[adr].r = atm3clev[((val & 0x02) << 2) | ((val & 0x40) >> 4) | ((port & 0x0200) >> 8) | ((port & 0x4000) >> 14)];
	comp->vid->pal[adr].g = atm3clev[((val & 0x10) >> 1) | ((val & 0x80) >> 5) | ((port & 0x1000) >> 11)| ((port & 0x8000) >> 15)];
#else
	comp->vid->pal[adr].b = ((val & 0x01) ? 0xaa : 0x00) + ((val & 0x20) ? 0x55 : 0x00);
	comp->vid->pal[adr].r = ((val & 0x02) ? 0xaa : 0x00) + ((val & 0x40) ? 0x55 : 0x00);
	comp->vid->pal[adr].g = ((val & 0x10) ? 0xaa : 0x00) + ((val & 0x80) ? 0x55 : 0x00);
#endif
}

void evoOut7FFD(Computer* comp, int port, int val) {
	if ((comp->pEFF7 & 4) && (comp->p7FFD & 0x20)) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val & 0xff;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	evoMapMem(comp);
}

void evoOutBEF7(Computer* comp, int port, int val) {	// dos
	cmsWr(comp,val);
}

void evoOutDEF7(Computer* comp, int port, int val) {	// dos
	comp->cmos.adr = val & 0xff;
}

void evoOutBFF7(Computer* comp, int port, int val) {	// !dos
	if (comp->pEFF7 & 0x80)
		cmsWr(comp,val);
}

void evoOutDFF7(Computer* comp, int port, int val) {	// !dos
	if (comp->pEFF7 & 0x80)
		comp->cmos.adr = val & 0xff;
}

void evoOutEFF7(Computer* comp, int port, int val) {	// !dos
	comp->pEFF7 = val;
	compSetTurbo(comp,(comp->prt2 & 0x08) ? 4 : (val & 0x08) ? 2 : 1);
	evoSetVideoMode(comp);
	evoMapMem(comp);
}

static xPort evoPortMap[] = {
	{0x00f7,0x00fe,2,2,2,xInFE,	xOutFE},	// A3 = border bright
//	{0x00ff,0x00fb,2,2,2,NULL,	evoOutFB},	// covox
	{0x00ff,0x00be,2,2,2,evoInBE,	NULL},
	{0x00ff,0x00bf,2,2,2,evoInBF,	evoOutBF},
	{0xffff,0x7ffd,2,2,2,NULL,	evoOut7FFD},
	{0xffff,0xfadf,2,2,2,xInFADF,	NULL},		// k-mouse (fadf,fbdf,ffdf)
	{0xffff,0xfbdf,2,2,2,xInFBDF,	NULL},
	{0xffff,0xffdf,2,2,2,xInFFDF,	NULL},
	{0xfeff,0xbffd,2,2,2,NULL,	xOutBFFD},	// ay/ym; bffd, fffd
	{0xfeff,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	// dos only
	{0x009f,0x001f,1,2,2,evoInBDI,	evoOutBDI},	// bdi 1f,3f,5f,7f
	{0x00ff,0x00ff,1,2,2,evoInBDI,	evoOutFF},	// bdi ff + set palette
	{0x00ff,0x002f,1,2,2,evoIn2F,	evoOut2F},	// extend bdi ports
	{0x00ff,0x004f,1,2,2,evoIn4F,	evoOut4F},
	{0x00ff,0x006f,1,2,2,evoIn6F,	evoOut6F},
	{0x00ff,0x008f,1,2,2,evoIn8F,	evoOut8F},
	{0x00ff,0x0057,1,2,2,NULL,	evoOut77},	// 55,77 : spi. 55 dos = 77 !dos
	{0x00ff,0x0077,1,2,2,NULL,	evoOut77d},
	{0xffff,0xbef7,1,2,2,evoInBEF7,	evoOutBEF7},	// nvram
	{0xffff,0xdef7,1,2,2,NULL,	evoOutDEF7},
	{0x07ff,0x07f7,1,2,2,NULL,	evoOutF7},	// x7f7
	// !dos only
	{0x00ff,0x001f,0,2,2,evoIn1F,	NULL},		// k-joy
	{0x00ff,0x0057,0,2,2,evoIn57,	evoOut57},	// 57,77 : spi
	{0x00ff,0x0077,0,2,2,evoIn77,	evoOut77},
	{0xffff,0xbff7,0,2,2,evoInBFF7,	evoOutBFF7},	// nvram
	{0xffff,0xdff7,0,2,2,NULL,	evoOutDFF7},
	{0xffff,0xeff7,0,2,2,NULL,	evoOutEFF7},

	{0x0000,0x0000,2,2,2,NULL,	NULL}
};

void evoOut(Computer* comp, int port, int val, int dos) {
	if (comp->evo.evoBF & 0x01) dos = 1;	// force open ports
	zx_dev_wr(comp, port, val, dos);
	hwOut(evoPortMap, comp, port, val, dos);
}

int evoIn(Computer* comp, int port, int dos) {
	int res = -1;
	if (comp->evo.evoBF & 1) dos = 1;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	res = hwIn(evoPortMap, comp, port, dos);
	return res;
}
