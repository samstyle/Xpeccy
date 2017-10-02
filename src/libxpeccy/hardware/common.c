#include "hardware.h"

// debug


unsigned char brkIn(Computer* comp, unsigned short port) {
	printf("IN %.4X (dos:rom:cpm = %i:%i:%i)\n",port,comp->dos,comp->rom,comp->cpm);
	assert(0);
	comp->brk = 1;
	return 0xff;
}

void brkOut(Computer* comp, unsigned short port, unsigned char val) {
	printf("OUT %.4X,%.2X (dos:rom:cpm = %i:%i:%i)\n",port,val,comp->dos,comp->rom,comp->cpm);
	assert(0);
	comp->brk = 1;
}

unsigned char dummyIn(Computer* comp, unsigned short port) {
	return 0xff;
}

void dummyOut(Computer* comp, unsigned short port, unsigned char val) {

}

// INT handle/check

void zx_sync(Computer* comp, int ns) {
//	if (!comp->cpu->iff1 || comp->cpu->noint) return;
	if (comp->vid->intFRAME && (comp->vid->intMask & 1)) {
		comp->intVector = 0xff;
		comp->cpu->intrq |= Z80_INT;
		comp->vid->intFRAME = 0;
	} else if (comp->vid->intLINE) {
		comp->intVector = 0xfd;
		comp->cpu->intrq |= Z80_INT;
		comp->vid->intLINE = 0;
	} else if (comp->vid->intDMA) {
		comp->intVector = 0xfb;
		comp->cpu->intrq |= Z80_INT;
		comp->vid->intDMA = 0;
	}
}

// zx keypress/release

void zx_keyp(Computer* comp, keyEntry ent) {
	keyPressXT(comp->keyb, ent.keyCode);
	keyPress(comp->keyb, ent.zxKey, 0);
}

void zx_keyr(Computer* comp, keyEntry ent) {
	keyReleaseXT(comp->keyb, ent.keyCode);
	keyRelease(comp->keyb, ent.zxKey, 0);
}

// volume

sndPair zx_vol(Computer* comp, sndVolume* sv) {
	sndPair vol;
	sndPair svol;
	int lev = 0;
	vol.left = 0;
	vol.right = 0;
	// tape sound
	if (comp->tape->on) {
		if (comp->tape->rec) {
			lev = comp->tape->levRec ? 0x3f : 0x00;
		} else {
			lev = comp->tape->levPlay ? 0x3f : 0x00;
		}
	}
	vol = mixer(vol, lev, lev, sv->tape);
	// beeper
	bcSync(comp->beep, -1);
	lev = comp->beep->val >> 2;			// ff -> 3f
	vol = mixer(vol, lev, lev, sv->beep);
	// turbo sound
	svol = tsGetVolume(comp->ts);
	vol = mixer(vol, svol.left, svol.right, sv->ay);
	// general sound
	svol = gsVolume(comp->gs);
	vol = mixer(vol, svol.left, svol.right, sv->gs);
	// soundrive
	svol = sdrvVolume(comp->sdrv);
	vol = mixer(vol, svol.left, svol.right, sv->sdrv);
	// saa
	svol = saaVolume(comp->saa);
	vol = mixer(vol, svol.left, svol.right, sv->saa);

	return vol;
}

// in

int zx_dev_wr(Computer* comp, unsigned short adr, unsigned char val, int dos) {
	int res = 0;
	res = gsWrite(comp->gs, adr, val);
	if (!dos) {
		res |= saaWrite(comp->saa, adr, val);
		res |= sdrvWrite(comp->sdrv, adr, val);
	}
	return res;
}

int zx_dev_rd(Computer* comp, unsigned short adr, unsigned char* ptr, int dos) {
	if (gsRead(comp->gs, adr, ptr)) return 1;
	return 0;
}

unsigned char xIn1F(Computer* comp, unsigned short port) {
	return joyInput(comp->joy);
}

unsigned char xInFE(Computer* comp, unsigned short port) {
	unsigned char res = keyInput(comp->keyb, (port & 0xff00) | 0xfe);
	res |= (comp->tape->levPlay ? 0x40 : 0x00);
	return res;
}

unsigned char xInFFFD(Computer* comp, unsigned short port) {
	return tsIn(comp->ts, 0xfffd);
}

unsigned char xInFADF(Computer* comp, unsigned short port) {
	unsigned char res = 0xff;
	comp->mouse->used = 1;
	if (!comp->mouse->enable) return res;
	if (comp->mouse->hasWheel) {
		res &= 0x0f;
		res |= ((comp->mouse->wheel & 0x0f) << 4);
	}
	res ^= comp->mouse->mmb ? 4 : 0;
	if (comp->mouse->swapButtons) {
		res ^= comp->mouse->rmb ? 1 : 0;
		res ^= comp->mouse->lmb ? 2 : 0;
	} else {
		res ^= comp->mouse->lmb ? 1 : 0;
		res ^= comp->mouse->rmb ? 2 : 0;
	}
	return res;
}

unsigned char xInFBDF(Computer* comp, unsigned short port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->xpos : 0xff;
}

unsigned char xInFFDF(Computer* comp, unsigned short port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->ypos : 0xff;
}

// out

void xOutFE(Computer* comp, unsigned short port, unsigned char val) {
	comp->vid->nextbrd = val & 0x07;
	bcSync(comp->beep, -1);
	comp->beep->lev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void xOutBFFD(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xbffd, val);
}

void xOutFFFD(Computer* comp, unsigned short port, unsigned char val) {
	tsOut(comp->ts, 0xfffd, val);
}
