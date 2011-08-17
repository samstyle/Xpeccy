#include "sound.h"
#include "spectrum.h"

extern Sound* snd;
extern ZXComp* zx;

uint8_t IOSys::in(int32_t port) {
	uint8_t res = 0xff;
	switch (type) {
		case IO_ZX: res = zx->in(port); break;
		case IO_GS: res = zx->gs->intin(port); break;
	}
	return res;
}

void IOSys::out(int32_t port, uint8_t val) {
	switch (type) {
		case IO_ZX: zx->out(port,val); break;
		case IO_GS: zx->gs->intout(port,val); break;
	}
}

void IOSys::out7ffd(uint8_t val) {
	if (block7ffd) return;
	zx->sys->mem->prt0 = val;
	zx->vid->curscr = val & 0x08;
	block7ffd = val & 0x20;
}

uint8_t IOSys::iostdin(int port) {
	uint8_t res = 0xff;
	if ((port&0xff) == 0xfe) {
		zx->tape->sync();
		res = zx->keyb->getmap((port&0xff00)>>8) | ((zx->tape->signal<<6)&0x40);
	}
	switch (port) {
		case 0xfbdf: res = zx->mouse->xpos; break;
		case 0xffdf: res = zx->mouse->ypos; break;
		case 0xfadf: res = zx->mouse->buttons; break;
		case 0xfffd:
			if (zx->aym->scc->curreg<14) {
				res = zx->aym->scc->reg[zx->aym->scc->curreg];
			} else {
				if ((zx->aym->scc->reg[7]&0x40) && (zx->aym->scc->curreg == 14)) res = zx->aym->scc->reg[14];
				if ((zx->aym->scc->reg[7]&0x80) && (zx->aym->scc->curreg == 15)) res = zx->aym->scc->reg[15];
			}
			break;
	}
	return res;
}

void IOSys::iostdout(int port, uint8_t val) {
	if ((port&0xff) == 0xfe) {
		zx->vid->brdcol = val&0x07;
		snd->beeplev = val&0x10;
		zx->tape->outsig = val&0x08; zx->tape->sync();
	}
	switch (port) {
		case 0xfffd: switch (val) {
				case 0xfe: if (zx->aym->tstype == TS_NEDOPC) zx->aym->scc = zx->aym->sc1; break;	// fe / ff - select sound chip in TS
				case 0xff: if (zx->aym->tstype == TS_NEDOPC) zx->aym->scc = zx->aym->sc2; break;
				default: zx->aym->scc->curreg = val; break;		// set sound chip register
			}
			break;
		case 0xbffd: zx->aym->scc->setreg(val); break;			// write in sound chip register
	}
}

//-------------

IOSys::IOSys(int p1) {
	type = p1;
}
