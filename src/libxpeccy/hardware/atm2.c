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

void atm2OutFE(Computer* comp, int port, int val) {
	xOutFE(comp, port, val);
	if (~port & 0x08) {		// A3 = 0 : bright border
		comp->vid->nextbrd |= 0x08;
		comp->vid->brdcol = comp->vid->nextbrd;
	}
}

void atm2Out77(Computer* comp, int port, int val) {		// dos
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

void atm2OutF7(Computer* comp, int port, int val) {			// dos
	int adr = (comp->rom ? 4 : 0) | ((port & 0xc000) >> 14);	// rom2.a15.a14
	comp->memMap[adr].flag = val & 0xc0;				// copy b6,7 to flag
	comp->memMap[adr].page = (val & 0x3f) | 0xc0;			// set b6,7 for PentEvo capability
	atm2MapMem(comp);
}

void atm2Out7FFD(Computer* comp, int port, int val) {
	if (comp->p7FFD & 0x20) return;
	comp->rom = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val & 0xff;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	atm2MapMem(comp);
}

static const unsigned char atm2clev[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};

void atm2OutFF(Computer* comp, int port, int val) {		// dos. bdiOut already done
	if (comp->p77hi & 0x40) return;			// pen2 = 1
	val ^= 0xff;	// inverse colors
	int adr = comp->vid->brdcol & 0x0f;
#if 1			// ddp extend palete
	port ^= 0xff00;
	if (!comp->ddpal) port = (port & 0xff) | ((val << 8) & 0xff00);
	comp->vid->pal[adr].b = atm2clev[((val & 0x01) << 3) | ((val & 0x20) >> 3) | ((port & 0x0100) >> 7) | ((port & 0x2000) >> 13)];
	comp->vid->pal[adr].r = atm2clev[((val & 0x02) << 2) | ((val & 0x40) >> 4) | ((port & 0x0200) >> 8) | ((port & 0x4000) >> 14)];
	comp->vid->pal[adr].g = atm2clev[((val & 0x10) >> 1) | ((val & 0x80) >> 5) | ((port & 0x1000) >> 11)| ((port & 0x8000) >> 15)];
#else
	comp->vid->pal[adr].b = ((val & 0x01) ? 0xaa : 0x00) + ((val & 0x20) ? 0x55 : 0x00);
	comp->vid->pal[adr].r = ((val & 0x02) ? 0xaa : 0x00) + ((val & 0x40) ? 0x55 : 0x00);
	comp->vid->pal[adr].g = ((val & 0x10) ? 0xaa : 0x00) + ((val & 0x80) ? 0x55 : 0x00);
#endif
}

// in

static unsigned char kmodTab[4] = {kbdZX, kbdCODE, kbdCPM, kbdDIRECT};
static unsigned char kmodVer[4] = {6,0,1,0};
extern unsigned char kbdScanZX(Keyboard*, int);

int atm2inFE(Computer* comp, int port) {
	// int res = kbdRead(comp->keyb, port & 0xffff);
	int res = 0xff;
	int hi = (port >> 14) & 3;
	time_t rtime;
	time(&rtime);
	struct tm* ctime = localtime(&rtime);
	if (comp->keyb->wcom) {
		comp->keyb->com = (port >> 8) & 0x3f;
		comp->keyb->wcom = 0;
		switch(comp->keyb->com) {
			case 0x01: res = kmodVer[hi]; break;
			case 0x07: comp->keyb->kBufPos = 0; break;
			case 0x00:
			case 0x09:
				switch(hi) {
					case 0:
						res = comp->keyb->keycode;
						comp->keyb->keycode = 0;
						break;
					case 1: res = comp->keyb->lastkey; break;
					case 2: res = comp->keyb->flag1; break;
					case 3: res = comp->keyb->flag2; break;
				}
				break;
			case 0x0a: comp->keyb->flag1 |= KFL_RUS; break;
			case 0x0b: comp->keyb->flag1 &= ~KFL_RUS; break;
			case 0x0c: break;			// TODO: pause
			case 0x0d:
				comp->keyb->reset = 1;
				break;
			case 0x10:
				switch(hi) {
					case 0: res = ctime->tm_sec & 0xff; break;
					case 1: res = ctime->tm_min & 0xff; break;
					case 2: res = ctime->tm_hour & 0xff; break;
				}
				break;
			case 0x12:
				switch(hi) {
					case 0: res = ctime->tm_mday & 0xff; break;
					case 1: res = ctime->tm_mon & 0xff; break;
					case 2: res = ctime->tm_year & 0xff; break;
				}
				//printf("%i = %i\n",hi,res);
				break;
			case 0x16: break;
			case 0x17: break;
			case 0x11:				// set time
			case 0x13:				// set date
			case 0x14:				// set P1 bits
			case 0x15:				// res P1 bits
			case 0x08:				// set mode
				comp->keyb->warg = 1;
				break;
			default: res = 0xff; break;
		}
	} else if (comp->keyb->warg) {
		comp->keyb->arg = (port >> 8) & 0xff;
		comp->keyb->warg = 0;
		switch (comp->keyb->com) {
			case 0x08:
				comp->keyb->submode = kmodTab[comp->keyb->arg & 3];
				break;
		}
	} else if ((port & 0xff00) == 0x5500) {
		comp->keyb->wcom = 1;
		comp->keyb->warg = 0;
		res = 0xaa;
	} else {
		switch(comp->keyb->submode) {
			case kbdZX:
				res = kbdScanZX(comp->keyb, port);
				break;
			case kbdCODE:
				res = comp->keyb->keycode;
				comp->keyb->keycode = 0;
				break;
			case kbdCPM:
				switch(hi) {
					case 0: res = comp->keyb->keycode;
						comp->keyb->keycode = 0;
						break;
					case 1: res = comp->keyb->flag2;
						break;
					case 2: res = comp->keyb->flag1;
						break;
				}
				break;
			case kbdDIRECT:
				break;
		}
	}

	if (comp->keyb->mode == KBD_SPECTRUM) {
		res |= (comp->tape->volPlay & 0x80) ? 0x40 : 0x00;
	} else if (comp->keyb->submode == kbdZX) {
		res |= (comp->tape->volPlay & 0x80) ? 0x40 : 0x00;
	}
	return res;
}

int atm2InFF(Computer* comp, int port) {
	return (comp->vid->vbrd || comp->vid->hbrd) ? 0xff : comp->vid->atrbyte & 0xff;
}

static xPort atm2PortMap[] = {
	{0x0007,0x00fe,2,2,2,atm2inFE,	atm2OutFE},
	{0x0007,0x00fa,2,2,2,NULL,	NULL},		// fa
//	{0x0007,0x00fb,2,2,2,NULL,	atm2OutFB},	// fb (covox)
	{0x8202,0x7ffd,2,2,2,NULL,	atm2Out7FFD},
	{0x8202,0x7dfd,2,2,2,NULL,	NULL},		// 7DFD
	{0xc202,0xbffd,2,2,2,NULL,	xOutBFFD},	// ay
	{0xc202,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	// !dos
	{0xffff,0xfadf,0,2,2,xInFADF,	NULL},		// k-mouse
	{0xffff,0xfbdf,0,2,2,xInFBDF,	NULL},
	{0xffff,0xffdf,0,2,2,xInFFDF,	NULL},
	// dos
	{0x009f,0x00ff,1,2,2,NULL,	atm2OutFF},	// palette (dos)
	{0x009f,0x00f7,1,2,2,NULL,	atm2OutF7},
	{0x009f,0x0077,1,2,2,NULL,	atm2Out77},
	{0x0000,0x0000,2,2,2,atm2InFF,	NULL}
};

void atm2Out(Computer* comp, int port, int val, int dos) {
	if (~comp->p77hi & 2) dos = 1;
	zx_dev_wr(comp, port, val, dos);
	difOut(comp->dif, port, val, dos);
	hwOut(atm2PortMap, comp, port, val, dos);
}

int atm2In(Computer* comp, int port, int dos) {
	int res = -1;
	if (~comp->p77hi & 2) dos = 1;
	if (zx_dev_rd(comp, port, &res, dos)) return res;
	if (difIn(comp->dif, port, &res, dos)) return res;
	res = hwIn(atm2PortMap, comp, port, dos);
	return res;
}

extern keyScan keyTab[];
void atm2_keyp(Computer* comp, keyEntry kent) {
	switch(kent.key) {
		case XKEY_LSHIFT: comp->keyb->flag1 |= KFL_SHIFT; break;
		case XKEY_RSHIFT: comp->keyb->flag2 |= KFL_RSHIFT; break;
		case XKEY_RCTRL:
		case XKEY_LCTRL: comp->keyb->flag1 |= KFL_CTRL; break;
		case XKEY_RALT:
		case XKEY_LALT: comp->keyb->flag1 |= KFL_ALT; break;
		case XKEY_CAPS: comp->keyb->flag1 ^= KFL_CAPS; break;
	}
	switch (comp->keyb->submode) {
		case kbdZX:
			kbd_press(comp->keyb, keyTab, comp->keyb->map, kent.zxKey);
			//kbd->keycode = ent.atmCode.rowScan;
			//kbd->lastkey = kbd->keycode;
			//kbd->map[((ent.atmCode.rowScan >> 4) & 7) ^ 7] &= ~(1 << ((ent.atmCode.rowScan & 7) - 1));
			//if (ent.atmCode.rowScan & 0x80) kbd->map[0] &= ~2;	// sym.shift
			//if (ent.atmCode.rowScan & 0x08) kbd->map[7] &= ~1;	// cap.shift
			break;
		case kbdCODE:
		case kbdCPM:
			comp->keyb->keycode = kent.atmCode.cpmCode;
			comp->keyb->lastkey = comp->keyb->keycode;
			break;
		case kbdDIRECT:
			break;
	}
}

void atm2_keyr(Computer* comp, keyEntry kent) {
	//kbdRelease(comp->keyb, kent);
	switch(kent.key) {
		case XKEY_LSHIFT: comp->keyb->flag1 &= ~KFL_SHIFT; break;
		case XKEY_RSHIFT: comp->keyb->flag2 &= ~KFL_RSHIFT; break;
		case XKEY_RCTRL:
		case XKEY_LCTRL: comp->keyb->flag1 &= ~KFL_CTRL; break;
		case XKEY_RALT:
		case XKEY_LALT: comp->keyb->flag1 &= ~KFL_ALT; break;
	}
	switch (comp->keyb->submode) {
		case kbdZX:
			kbd_release(comp->keyb, keyTab, comp->keyb->map, kent.zxKey);
			//kbd->keycode = 0;
			//kbd->map[((ent.atmCode.rowScan >> 4) & 7) ^ 7] |= (1 << ((ent.atmCode.rowScan & 7) - 1));
			//if (ent.atmCode.rowScan & 0x80) kbd->map[0] |= 2;	// sym.shift
			//if (ent.atmCode.rowScan & 0x08) kbd->map[7] |= 1;	// cap.shift
			break;
		case kbdCODE:
		case kbdCPM:
			comp->keyb->keycode = 0;
			break;
		case kbdDIRECT:
			break;
	}
}
