//#include "video.h"
//#include "iosys.h"
//#include "memory.h"
#include "keyboard.h"
#include "hdd.h"
#include "bdi.h"
#include "sound.h"
#include "tape.h"
#include "spectrum.h"
#include "gs.h"

void zx_out(int port, unsigned char val) {
//	if (ide->out(port,val)) return;
	gs->sync(sys->vid->t);
	if (gs->out(port,val)) return;
	if (bdi->out(port,val)) return;
	port = machine->getport(port);
	machine->out(port,val);
}

unsigned char zx_in(int port) {
	unsigned char res = 0xff;
//	if (mwin->flags & FL_RZX) {
//		if (mwin->rfpos >= mwin->rzx[mwin->rfnum].in.size()) {
//			printf("WARNING: CPU.IN_COUNT > RZX.IN_COUNT at frame %i\n",mwin->rfnum);
//		} else {
//			res = mwin->rzx[mwin->rfnum].in[mwin->rfpos];
//			printf("res = %.2X\n",res);
//			mwin->rfpos++;
//		}
//		return res;
//	}
//	if (ide->in(port,&res)) return res;
	gs->sync(sys->vid->t);
	if (gs->in(port,&res)) return res;
	if (bdi->in(port,&res)) return res;
	port = machine->getport(port);
	return machine->in(port);
}

void IOSys::out7ffd(unsigned char val) {
	if (block7ffd) return;
	sys->mem->prt0 = val;
	sys->vid->curscr = val & 0x08;
	block7ffd = val & 0x20;
}

void IOSys::setmacptr(std::string nam) {
	machine = NULL;
	unsigned int i; for (i=0;i<machlist.size();i++) {
		if (machlist[i].name == nam) {
			machine = &machlist[i];
			flags = machine->flags;
			break;
		}
	}
}

unsigned char IOSys::iostdin(int port) {
	unsigned char res = 0xff;
	if ((port&0xff) == 0xfe) {
		tape->sync();
		res = keyb->getmap((port&0xff00)>>8) | ((tape->signal<<6)&0x40);
	}
	switch (port) {
		case 0xfbdf: res = mouse->xpos; break;
		case 0xffdf: res = mouse->ypos; break;
		case 0xfadf: res = mouse->buttons; break;
		case 0xfffd:
			if (snd->scc->curreg<14) {
				res = snd->scc->reg[snd->scc->curreg];
			} else {
				if ((snd->scc->reg[7]&0x40) && (snd->scc->curreg == 14)) res = snd->scc->reg[14];
				if ((snd->scc->reg[7]&0x80) && (snd->scc->curreg == 15)) res = snd->scc->reg[15];
			}
			break;
	}
	return res;
}

void IOSys::iostdout(int port, unsigned char val) {
	if ((port&0xff) == 0xfe) {
		sys->vid->brdcol = val&0x07;
		snd->beeplev = val&0x10;
		tape->outsig = val&0x08; tape->sync();
	}
	switch (port) {
		case 0xfffd: switch (val) {
				case 0xfe: if (snd->tstype == TS_NEDOPC) snd->scc = snd->sc1; break;	// fe / ff - select sound chip in TS
				case 0xff: if (snd->tstype == TS_NEDOPC) snd->scc = snd->sc2; break;
				default: snd->scc->curreg = val; break;		// set sound chip register
			}
			break;
		case 0xbffd: snd->scc->setreg(val); break;			// write in sound chip register
	}
}

void IOSys::addmachine(std::string nam,int(*gpfnc)(int),void(*ofnc)(int,unsigned char),unsigned char(*ifnc)(int),void(*sfnc)(), int msk, int flg) {
	Machine newmac;
	newmac.name = nam;
	newmac.mask = msk;
	newmac.getport = gpfnc;
	newmac.out = ofnc;
	newmac.in = ifnc;
	newmac.setrom = sfnc;
	newmac.flags = flg;
	machlist.push_back(newmac);
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
	sys->mem->setrom(bdi->active?3:1);
	sys->mem->setram(0);
}

void zx48_outport(int port, unsigned char val) {
	sys->io->iostdout(port,val);
}

unsigned char zx48_inport(int port) {
	return sys->io->iostdin(port);
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
	sys->mem->setrom((bdi->active)?3:((sys->mem->prt0 & 0x10)>>4));
	sys->mem->setram((sys->mem->prt0 & 7) | ((sys->mem->prt0 & 0xc0)>>3));
}

void pent_outport(int port, unsigned char val) {
	sys->io->iostdout(port,val);
	switch (port) {
		case 0x7ffd: sys->io->out7ffd(val); pent_setrom(); break;
	}
}

unsigned char pent_inport(int port) {
	return sys->io->iostdin(port);
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
	sys->mem->setrom(bdi->active?3:((sys->mem->prt1 & 8)?0xff:((sys->mem->prt0 & 0x10)>>4)));
	sys->mem->setram((sys->mem->prt0 & 7) | ((sys->mem->prt1 & 4)?0:((sys->mem->prt0 & 0x20) | ((sys->mem->prt0 & 0xc0)>>3))));
}

void p1m_outport(int port,unsigned char val) {
	sys->io->iostdout(port,val);
	switch (port) {
		case 0x7ffd:
			if (sys->io->block7ffd) break;
			sys->vid->curscr = val & 0x08;
			sys->mem->prt0 = val;
			sys->io->block7ffd = ((sys->mem->prt1 & 4) && (val & 0x20));
			p1m_setrom();
			break;
		case 0xeff7:
			sys->mem->prt1 = val;
			sys->vid->mode = (val & 1) ? VID_ALCO : VID_NORMAL;
			sys->cpu->frq = (val & 16) ? 7.0 : 3.5;
			p1m_setrom();
			break;
	}
}

unsigned char p1m_inport(int port) {
	return sys->io->iostdin(port);
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
	sys->mem->setrom((sys->mem->prt1 & 0x01)?0xff:((sys->mem->prt1 & 0x02)?2:((bdi->active)?3:((sys->mem->prt0 & 0x10)>>4))));
	sys->mem->setram((sys->mem->prt0 & 7) | ((sys->mem->prt1 & 0x10)>>1) | ((sys->mem->prt1 & 0xc0)>>2));
}

void scrp_outport(int port,unsigned char val) {
	sys->io->iostdout(port,val);
	switch (port) {
		case 0x1ffd: sys->mem->prt1 = val; scrp_setrom(); break;
		case 0x7ffd: sys->io->out7ffd(val); scrp_setrom(); break;
	}
}

unsigned char scrp_inport(int port) {
	unsigned char res = 0xff;
	res = sys->io->iostdin(port);
	switch (port) {
		case 0x7ffd: sys->cpu->frq = 7.0; break;
		case 0x1ffd: sys->cpu->frq = 3.5; break;
		case 0xff: if (((sys->vid->curr.h - sys->vid->bord.h) < 256) && ((sys->vid->curr.v - sys->vid->bord.v) < 192)) {
				res = sys->vid->atrbyte;
			} break;
	}
	return res;
}

//-------------

IOSys::IOSys(unsigned char(*p1)(int),void(*p2)(int,unsigned char)) {
	in = p1;
	out = p2;
	flags = 0;
	addmachine("ZX48K",&zx48_getport,&zx48_outport,&zx48_inport,&zx48_setrom,0x00,0);
	addmachine("Pentagon",&pent_getport,&pent_outport,&pent_inport,&pent_setrom,0x05,0);
	addmachine("Pentagon1024SL",&p1m_getport,&p1m_outport,&p1m_inport,&p1m_setrom,0x08,0);
	addmachine("Scorpion",&scrp_getport,&scrp_outport,&scrp_inport,&scrp_setrom,0x0a,IO_WAIT);
}
