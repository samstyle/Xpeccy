#include "../spectrum.h"

#include <time.h>

enum {
	kbdZX = 0,
	kbdCODE,
	kbdCPM,
	kbdDIRECT
};

// TODO : fill memMap & set prt1 for reset to separate ROM pages
void atm2Reset(Computer* comp) {
	comp->dos = 1;
	comp->p77hi = 0;
	comp->atm2.kbd.enable = 1;
	comp->atm2.kbd.mode = kbdZX;
	comp->atm2.kbd.wcom = 0;
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
	memSetBank(comp->mem, bank, (me.flag & 0x40) ? MEM_RAM : MEM_ROM, page, NULL, NULL, NULL);
}

void atm2MapMem(Computer* comp) {
	if (comp->p77hi & 1) {			// pen = 0: last rom page in every bank && dosen on
		int adr = (comp->rom) ? 4 : 0;
		atmSetBank(comp,MEM_BANK0,comp->memMap[adr]);
		atmSetBank(comp,MEM_BANK1,comp->memMap[adr+1]);
		atmSetBank(comp,MEM_BANK2,comp->memMap[adr+2]);
		atmSetBank(comp,MEM_BANK3,comp->memMap[adr+3]);
	} else {
		comp->dos = 1;
		memSetBank(comp->mem,MEM_BANK0,MEM_ROM,0xff, NULL, NULL, NULL);
		memSetBank(comp->mem,MEM_BANK1,MEM_ROM,0xff, NULL, NULL, NULL);
		memSetBank(comp->mem,MEM_BANK2,MEM_ROM,0xff, NULL, NULL, NULL);
		memSetBank(comp->mem,MEM_BANK3,MEM_ROM,0xff, NULL, NULL, NULL);
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
	comp->atm2.kbd.enable = (val & 0x40) ? 0 : 1;
	comp->p77hi = (port & 0xff00) >> 8;
	atm2MapMem(comp);
}

void atm2OutF7(Computer* comp, unsigned short port, unsigned char val) {		// dos
	int adr = (comp->rom ? 4 : 0) | ((port & 0xc000) >> 14);	// rom2.a15.a14
	comp->memMap[adr].flag = val & 0xc0;		// copy b6,7 to flag
	comp->memMap[adr].page = (val & 0x3f) | 0xc0;	// set b6,7 for PentEvo capability
	atm2MapMem(comp);
}

/*
void atm2OutFB(Computer* comp, unsigned short port, unsigned char val) {
	sdrvOut(comp->sdrv, port, val);
}
*/

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

static unsigned char kmodTab[4] = {kbdZX, kbdCODE, kbdCPM, kbdDIRECT};
static unsigned char kmodVer[4] = {6,0,1,0};

unsigned char atm2inFE(Computer* comp, unsigned short port) {

//	if (!comp->atm2.kbd.enable) return xInFE(comp, port);

	time_t rtime;
	time(&rtime);
	struct tm* ctime = localtime(&rtime);

	unsigned char res = 0xff;
	int hi = (port >> 14) & 3;
	//printf("arg:%.2X\n",port >> 8);
	if (comp->atm2.kbd.wcom) {	// if waiting command
		comp->atm2.kbd.com = (port >> 8) & 0x3f;
		comp->atm2.kbd.wcom = 0;
		//printf("com:%.2X\n",port >> 8);
		switch(comp->atm2.kbd.com) {
			case 0x01:			// 01,41,81,c1 : version
				res = kmodVer[hi];
				break;
			case 0x07:
//				comp->atm2.kbd.kbpos = 0;
				break;
			case 0x09:
				switch(hi) {
					case 0:
						res = comp->atm2.kbd.keycode;
						comp->atm2.kbd.keycode = 0;
						break;
					case 1:
						res = comp->atm2.kbd.lastkey;
						break;
					case 2:
						res = (comp->atm2.kbd.shift ? 1 : 0) | \
							(comp->atm2.kbd.ctrl ? 2 : 0) | \
							(comp->atm2.kbd.alt ? 4 : 0) | \
							(comp->atm2.kbd.caps ? 16 : 0) | \
							(comp->atm2.kbd.numlock ? 32 : 0) | \
							(comp->atm2.kbd.scrlock ? 64 : 0) | \
							(comp->atm2.kbd.lat ? 0 : 128);
						break;
					case 3:
						res = comp->atm2.kbd.rshift ? 1 : 0;
						break;
				}
				printf("%i:%.2X\n",hi,res);
				break;
			case 0x0a:				// rus
				comp->atm2.kbd.lat = 0;
				break;
			case 0x0b:				// lat
				comp->atm2.kbd.lat = 1;
				break;
			case 0x0c:				// TODO:pause
				break;
			case 0x0d:
				compReset(comp, RES_DEFAULT);
				break;
			case 0x10:			// read time (not BCD format!)
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
				break;
			case 0x16: break;
			case 0x17: break;
			case 0x11:				// set time
			case 0x13:				// set date
			case 0x14:				// set P1 bits
			case 0x15:				// res P1 bits
			case 0x08:				// set mode
				comp->atm2.kbd.warg = 1;
				break;
			default:			// unknown command = return FF
				res = 0xff;
				break;
		}
	} else if (comp->atm2.kbd.warg) {
		comp->atm2.kbd.arg = (port >> 8) & 0xff;
		comp->atm2.kbd.warg = 0;
		switch (comp->atm2.kbd.com) {
			case 0x08:
				comp->atm2.kbd.mode = kmodTab[comp->atm2.kbd.arg & 3];
				printf("mode:%.2X\n",comp->atm2.kbd.mode);
				break;
		}
	} else if ((port & 0xff00) == 0x5500) {		// switch to command mode
		comp->atm2.kbd.wcom = 1;
		comp->atm2.kbd.warg = 0;
		res = 0xaa;
	} else {
		switch (comp->atm2.kbd.mode) {
			case kbdZX:
				res = xInFE(comp, port);
				break;
			case kbdCODE:
				res = comp->atm2.kbd.keycode;
				comp->atm2.kbd.keycode = 0;
				break;
			case kbdCPM:
				switch (port & 0xf000) {
					case 0x0000:
						res = comp->atm2.kbd.keycode;
						comp->atm2.kbd.keycode = 0;
						break;
					case 0x4000:
						res = comp->atm2.kbd.rshift ? 1 : 0;
						break;
					case 0x8000:
						res = (comp->atm2.kbd.shift ? 1 : 0) | \
							(comp->atm2.kbd.ctrl ? 2 : 0) | \
							(comp->atm2.kbd.alt ? 4 : 0) | \
							(comp->atm2.kbd.caps ? 16 : 0) | \
							(comp->atm2.kbd.numlock ? 32 : 0) | \
							(comp->atm2.kbd.scrlock ? 64 : 0) | \
							(comp->atm2.kbd.lat ? 0 : 128);
						break;
				}
				break;
			case kbdDIRECT:
				break;
		}
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
	int row = ((kent.atmCode.rowScan >> 4) & 7) ^ 7;
	int msk = 1 << ((kent.atmCode.rowScan & 7) - 1);
	switch (kent.key) {
		case XKEY_RALT:
		case XKEY_LALT:
			comp->atm2.kbd.alt = 1;
			break;
		case XKEY_RCTRL:
		case XKEY_LCTRL:
			comp->atm2.kbd.ctrl = 1;
			break;
		case XKEY_RSHIFT:
			comp->atm2.kbd.rshift = 1;
		case XKEY_LSHIFT:
			comp->atm2.kbd.shift = 1;
			break;
		case XKEY_CAPS:
			comp->atm2.kbd.caps ^= 1;
			break;
		// num lock
		// scroll lock
		default:
			break;
	}
	if (comp->atm2.kbd.enable) {
		switch (comp->atm2.kbd.mode) {
			case kbdZX:
//				printf("%i:%.2X:%.2X\n",row,msk,kent.atmCode.rowScan);
				comp->atm2.kbd.keycode = kent.atmCode.rowScan;
				comp->keyb->map[row] &= ~msk;
				if (kent.atmCode.rowScan & 0x80) comp->keyb->map[0] &= ~2;	// sym.shift
				if (kent.atmCode.rowScan & 0x08) comp->keyb->map[7] &= ~1;	// cap.shift
				break;
			case kbdCODE:
			case kbdCPM:
				comp->atm2.kbd.keycode = kent.atmCode.cpmCode;
				break;
		}
		comp->atm2.kbd.lastkey = comp->atm2.kbd.keycode;
	} else {
		zx_keyp(comp, kent);
	}
}

void atm2_keyr(Computer* comp, keyEntry kent) {
	int row = ((kent.atmCode.rowScan >> 4) & 7) ^ 7;
	int msk = (0x80 << (kent.atmCode.rowScan & 7)) >> 8;
	switch (kent.key) {
		case XKEY_RALT:
		case XKEY_LALT:
			comp->atm2.kbd.alt = 0;
			break;
		case XKEY_RCTRL:
		case XKEY_LCTRL:
			comp->atm2.kbd.ctrl = 0;
			break;
		case XKEY_RSHIFT:
			comp->atm2.kbd.rshift = 0;
		case XKEY_LSHIFT:
			comp->atm2.kbd.shift = 0;
			break;
		default:
			break;
	}
	if (comp->atm2.kbd.enable) {
		switch(comp->atm2.kbd.mode) {
			case kbdZX:
				comp->atm2.kbd.keycode = 0;
				comp->keyb->map[row] |= msk;
				if (kent.atmCode.rowScan & 0x80) comp->keyb->map[0] |= 2;	// sym.shift
				if (kent.atmCode.rowScan & 0x08) comp->keyb->map[7] |= 1;	// cap.shift
				break;
			case kbdCODE:
			case kbdCPM:
				comp->atm2.kbd.keycode = 0;
				break;

		}
	} else {
		zx_keyr(comp, kent);
	}
}
