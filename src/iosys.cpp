//#include "video.h"
//#include "iosys.h"
//#include "memory.h"
//#include "keyboard.h"
//#include "hdd.h"
//#include "bdi.h"
#include "sound.h"
//#include "tape.h"
#include "spectrum.h"
//#include "gs.h"

extern HardWare* hw;
//extern Keyboard* keyb;
//extern Mouse* mouse;
//extern IDE* ide;
//extern BDI* bdi;
extern Sound* snd;
//extern Tape* tape;
extern ZXComp* zx;
//extern GS* gs;

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

void IOSys::setmacptr(std::string nam) {
	hw = NULL;
	uint32_t i; for (i=0;i<hwlist.size();i++) {
		if (hwlist[i].name == nam) {
			hw = &hwlist[i];
			flags = hw->flags;
			break;
		}
	}
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

void IOSys::addhardware(std::string nam,int(*gpfnc)(int),void(*ofnc)(int,uint8_t),uint8_t(*ifnc)(int),void(*sfnc)(), int msk, int flg) {
	HardWare newmac;
	newmac.name = nam;
	newmac.mask = msk;
	newmac.getport = gpfnc;
	newmac.out = ofnc;
	newmac.in = ifnc;
	newmac.setrom = sfnc;
	newmac.flags = flg;
	hwlist.push_back(newmac);
}

//==========================
// Spectrum 48K
//==========================

int zx48_getport(int prt) {
	int port = 0;
	if ((prt&0x01)==0) port=(prt&0xff00)|0xfe;
	return port;
}

void zx48_setrom() {
	zx->sys->mem->setrom(zx->bdi->active?3:1);
	zx->sys->mem->setram(0);
}

void zx48_outport(int port, uint8_t val) {
	zx->sys->io->iostdout(port,val);
}

uint8_t zx48_inport(int port) {
	return zx->sys->io->iostdin(port);
}

//==========================
// Pentagon 128/512K
//==========================

int pent_getport(int prt) {
	int port = 0;
	if ((prt&0x8002)==0x0000) port=0x7ffd;
	if ((prt&0xc002)==0x8000) port=0xbffd;
	if ((prt&0xc002)==0xc000) port=0xfffd;
	if ((prt&0x05a1)==0x0081) port=0xfadf;
	if ((prt&0x05a1)==0x0181) port=0xfbdf;
	if ((prt&0x05a1)==0x0581) port=0xffdf;
	if ((prt&0x0003)==0x0002) port=(prt&0xff00)|0xfe;	// TODO: уточнить
	return port;
}

void pent_setrom() {
	zx->sys->mem->setrom((zx->bdi->active) ? 3 : ((zx->sys->mem->prt0 & 0x10)>>4));
	zx->sys->mem->setram((zx->sys->mem->prt0 & 7) | ((zx->sys->mem->prt0 & 0xc0)>>3));
}

void pent_outport(int port, uint8_t val) {
	zx->sys->io->iostdout(port,val);
	switch (port) {
		case 0x7ffd: zx->sys->io->out7ffd(val); pent_setrom(); break;
	}
}

uint8_t pent_inport(int port) {
	return zx->sys->io->iostdin(port);
}

//==========================
// Pentagon 1024 SL 2
//==========================

int p1m_getport(int prt) {
	int port = 0;
	if ((prt&0x8002)==0x0000) port=0x7ffd;
	if ((prt&0xc002)==0x8000) port=0xbffd;
	if ((prt&0xc002)==0xc000) port=0xfffd;
	if ((prt&0xf008)==0xe000) port=0xeff7;
	if ((prt&0x0003)==0x0002) port=(prt&0xff00)|0xfe;	// TODO: уточнить
	return port;
}

void p1m_setrom() {
	zx->sys->mem->setrom(zx->bdi->active ? 3 : ((zx->sys->mem->prt1 & 8) ? 0xff : ((zx->sys->mem->prt0 & 0x10)>>4)));
	zx->sys->mem->setram((zx->sys->mem->prt0 & 7) | ((zx->sys->mem->prt1 & 4)?0:((zx->sys->mem->prt0 & 0x20) | ((zx->sys->mem->prt0 & 0xc0)>>3))));
}

void p1m_outport(int port,uint8_t val) {
	zx->sys->io->iostdout(port,val);
	switch (port) {
		case 0x7ffd:
			if (zx->sys->io->block7ffd) break;
			zx->vid->curscr = val & 0x08;
			zx->sys->mem->prt0 = val;
			zx->sys->io->block7ffd = ((zx->sys->mem->prt1 & 4) && (val & 0x20));
			p1m_setrom();
			break;
		case 0xeff7:
			zx->sys->mem->prt1 = val;
			zx->vid->mode = (val & 1) ? VID_ALCO : VID_NORMAL;
			zx->sys->cpu->frq = (val & 16) ? 7.0 : 3.5;
			p1m_setrom();
			break;
	}
}

uint8_t p1m_inport(int port) {
	return zx->sys->io->iostdin(port);
}

//==========================
// ZS Scorpion 256/1024K
//==========================

int scrp_getport(int prt) {
	int port = 0;
	if ((prt&0x0023)==0x0001) port = 0x00dd;		// printer
	if ((prt&0x0523)==0x0003) port = 0xfadf;		// mouse
	if ((prt&0x0523)==0x0103) port = 0xfbdf;
	if ((prt&0x0523)==0x0503) port = 0xffdf;
	if ((prt&0xc023)==0x0021) port = 0x1ffd;		// mem
	if ((prt&0xc023)==0x4021) port = 0x7ffd;
	if ((prt&0xc023)==0x8021) port = 0xbffd;		// ay
	if ((prt&0xc023)==0xc021) port = 0xfffd;
	if ((prt&0x0023)==0x0022) port = (prt&0xff00)|0xfe;	// fe
	if ((prt&0x0023)==0xc023) port = 0x00ff;		// ff
	return port;
}

void scrp_setrom() {
	zx->sys->mem->setrom((zx->sys->mem->prt1 & 0x01)?0xff:((zx->sys->mem->prt1 & 0x02)?2:((zx->bdi->active)?3:((zx->sys->mem->prt0 & 0x10)>>4))));
	zx->sys->mem->setram((zx->sys->mem->prt0 & 7) | ((zx->sys->mem->prt1 & 0x10)>>1) | ((zx->sys->mem->prt1 & 0xc0)>>2));
}

void scrp_outport(int port,uint8_t val) {
	zx->sys->io->iostdout(port,val);
	switch (port) {
		case 0x1ffd: zx->sys->mem->prt1 = val; scrp_setrom(); break;
		case 0x7ffd: zx->sys->io->out7ffd(val); scrp_setrom(); break;
	}
}

uint8_t scrp_inport(int port) {
	uint8_t res = 0xff;
	res = zx->sys->io->iostdin(port);
	switch (port) {
		case 0x7ffd: zx->sys->cpu->frq = 7.0; break;
		case 0x1ffd: zx->sys->cpu->frq = 3.5; break;
		case 0xff: if (((zx->vid->curr.h - zx->vid->bord.h) < 256) && ((zx->vid->curr.v - zx->vid->bord.v) < 192)) {
				res = zx->vid->atrbyte;
			} break;
	}
	return res;
}

//-------------

//IOSys::IOSys(uint8_t(*p1)(int),void(*p2)(int,uint8_t)) {
IOSys::IOSys(int p1) {
//	in = p1;
//	out = p2;
	type = p1;
	flags = 0;
	addhardware("ZX48K",&zx48_getport,&zx48_outport,&zx48_inport,&zx48_setrom,0x00,0);
	addhardware("Pentagon",&pent_getport,&pent_outport,&pent_inport,&pent_setrom,0x05,0);
	addhardware("Pentagon1024SL",&p1m_getport,&p1m_outport,&p1m_inport,&p1m_setrom,0x08,0);
	addhardware("Scorpion",&scrp_getport,&scrp_outport,&scrp_inport,&scrp_setrom,0x0a,IO_WAIT);
}
