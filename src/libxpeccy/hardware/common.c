#include "hardware.h"
#include "../filetypes/filetypes.h"

// debug


int brkIn(Computer* comp, int port) {
	printf("IN %.4X (dos:rom:cpm = %i:%i:%i)\n",port,comp->dos,comp->rom,comp->cpm);
	assert(0);
	//comp->brk = 1;
	return -1;
}

void brkOut(Computer* comp, int port, int val) {
	printf("OUT %.4X,%.2X (dos:rom:cpm = %i:%i:%i)\n",port,val,comp->dos,comp->rom,comp->cpm);
	assert(0);
	//comp->brk = 1;
}

int dummyIn(Computer* comp, int port) {
	return -1;
}

void dummyOut(Computer* comp, int port, int val) {

}

// INT handle/check

void zx_sync(Computer* comp, int ns) {
	// devices
	difSync(comp->dif, ns);
	gsSync(comp->gs, ns);
	saaSync(comp->saa, ns);
	tsSync(comp->ts, ns);
	tapSync(comp->tape, ns);
	bcSync(comp->beep, ns);
	// nmi
	if ((comp->cpu->pc > 0x3fff) && comp->nmiRequest) {
		comp->cpu->intrq |= Z80_NMI;	// request nmi
		comp->dos = 1;			// set dos page
		comp->rom = 1;
		comp->hw->mapMem(comp);
	}
	// int
//	if (comp->vid->intFRAME) {
//		comp->intVector = 0xff;
//		comp->cpu->intrq |= Z80_INT;
//	} else if (comp->vid->intLINE) {
//		comp->intVector = 0xfd;
//		comp->cpu->intrq |= Z80_INT;
//		comp->vid->intLINE = 0;
//	} else if (comp->vid->intDMA) {
//		comp->intVector = 0xfb;
//		comp->cpu->intrq |= Z80_INT;
//		comp->vid->intDMA = 0;
//	} else if (comp->cpu->intrq & Z80_INT) {
//		comp->cpu->intrq &= ~Z80_INT;
//	}
}

void zx_irq(Computer* comp, int t) {
	switch(t) {
		case IRQ_VID_INT:			// frame int start
			if (!comp->rzx.play) {		// ignore when playing rzx
				comp->vid->intFRAME = comp->vid->intsize;
				comp->intVector = 0xff;
				comp->cpu->intrq |= Z80_INT;
			}
			break;
		case IRQ_RZX_INT:
			comp->intVector = 0xff;
			comp->cpu->intrq |= Z80_INT;
			comp->vid->intFRAME = comp->vid->intsize;
			comp->rzx.fCurrent++;
			comp->rzx.fCount--;
			rzxGetFrame(comp);
			break;
		case IRQ_VID_INT_E:			// frame int end
			if (comp->vid->intLINE) {
				zx_irq(comp, IRQ_VID_LINE);
			} else if (comp->vid->intDMA) {
				zx_irq(comp, IRQ_DMA);
			} else {
				comp->cpu->intrq &= ~Z80_INT;
			}
			break;
		case IRQ_VID_LINE:			// line int (tsconf)
			if (comp->vid->intFRAME) break;
			comp->vid->intLINE = 0;
			comp->intVector = 0xfd;
			comp->cpu->intrq |= Z80_INT;
			break;
		case IRQ_DMA:				// dma int (tsconf)
			if (comp->vid->intFRAME) break;
			comp->vid->intDMA = 0;
			comp->intVector = 0xfb;
			comp->cpu->intrq |= Z80_INT;
			break;
	}
}

void zx_init(Computer* comp) {
	comp->nsPerTick &= ~1;		// make even
	comp->fps = 50;
	vidUpdateTimings(comp->vid, comp->nsPerTick >> 1);
	fdc_set_hd(comp->dif->fdc, 0);
}

// zx keypress/release

void zx_keyp(Computer* comp, keyEntry ent) {
	kbdPress(comp->keyb, ent);
}

void zx_keyr(Computer* comp, keyEntry ent) {
	kbdRelease(comp->keyb, ent);
}

// volume

sndPair zx_vol(Computer* comp, sndVolume* sv) {
	sndPair vol;
	sndPair svol;
	int lev = 0;
	vol.left = 0;
	vol.right = 0;
	// 1:tape sound
//	if (comp->tape->on) {
		if (comp->tape->rec) {
			lev = comp->tape->levRec ? 0x1000 * sv->tape / 100 : 0;
		} else {
			lev = (comp->tape->volPlay << 8) * sv->tape / 1600;
		}
//	}
	// 2:beeper
	// bcSync(comp->beep, -1);
	lev += comp->beep->val * sv->beep / 6;
	vol.left = lev;
	vol.right = lev;
	// 3:turbo sound
	svol = tsGetVolume(comp->ts);
	vol.left += svol.left * sv->ay / 100;
	vol.right += svol.right * sv->ay / 100;
	// 4:general sound
	svol = gsVolume(comp->gs);
	vol.left += svol.left * sv->gs / 100;
	vol.right += svol.right * sv->gs / 100;
	// 5:soundrive
	svol = sdrvVolume(comp->sdrv);
	vol.left += svol.left * sv->sdrv / 100;
	vol.right += svol.right * sv->sdrv / 100;
	// 6:saa
	svol = saaVolume(comp->saa);
	vol.left += svol.left * sv->saa / 100;
	vol.right += svol.right * sv->saa / 100;
	// end
	return vol;
}

// set std zx palette

void zx_set_pal(Computer* comp) {
	int i;
	xColor xcol;
	for (i = 0; i < 16; i++) {
		xcol.b = (i & 1) ? ((i & 8) ? 0xff : 0xaa) : 0x00;
		xcol.r = (i & 2) ? ((i & 8) ? 0xff : 0xaa) : 0x00;
		xcol.g = (i & 4) ? ((i & 8) ? 0xff : 0xaa) : 0x00;
		vid_set_col(comp->vid, i, xcol);
	}
}

// in

int zx_dev_wr(Computer* comp, int adr, int val) {
	if (gsWrite(comp->gs, adr, val)) return 1;
	if (!comp->bdiz && saaWrite(comp->saa, adr, val)) return 1;
	if (!comp->bdiz && sdrvWrite(comp->sdrv, adr, val)) return 1;
	if (ideOut(comp->ide, adr, val, comp->bdiz)) return 1;
	if (ula_wr(comp->vid->ula, adr, val)) return 1;
	return 0;
}

int zx_dev_rd(Computer* comp, int adr, int* ptr) {
	if (gsRead(comp->gs, adr, ptr)) return 1;
	if (ideIn(comp->ide, adr, ptr, comp->bdiz)) return 1;
	if (ula_rd(comp->vid->ula, adr, ptr)) return 1;
	return 0;
}

int xIn1F(Computer* comp, int port) {
	return joyInput(comp->joy);
}

int xInFE(Computer* comp, int port) {
	comp->keyb->port &= (port >> 8);
	unsigned char res = kbdRead(comp->keyb, port) | 0xa0;		// set bits 7,5
	if (comp->tape->volPlay & 0x80)
		res |= 0x40;
	return res;
}

int xInFFFD(Computer* comp, int port) {
	return tsIn(comp->ts, 0xfffd);
}

int xInFADF(Computer* comp, int port) {
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

int xInFBDF(Computer* comp, int port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->xpos : 0xff;
}

int xInFFDF(Computer* comp, int port) {
	comp->mouse->used = 1;
	return comp->mouse->enable ? comp->mouse->ypos : 0xff;
}

// out

void xOutFE(Computer* comp, int port, int val) {
	comp->vid->nextbrd = (val & 0x07);
	comp->beep->lev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void xOutBFFD(Computer* comp, int port, int val) {
	tsOut(comp->ts, 0xbffd, val);
}

void xOutFFFD(Computer* comp, int port, int val) {
	tsOut(comp->ts, 0xfffd, val);
}
