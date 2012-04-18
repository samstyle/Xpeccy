#include <stdio.h>
#include "spectrum.h"

/*
 * ProfROM switch:
 * 	page 2,6,10,14
 * 	PC = E4B5 : ld l,(hl)
 * 	HL = 0110..0113
 * ProfROM table :
 *  adr | 0 1 2 3 <- current layer
 * -----+---------
 * 0110 | 0 1 2 3 <- result layers
 * 0111 | 3 3 3 2
 * 0112 | 2 2 0 1
 * 0113 | 1 0 1 0
 */

uint8_t ZSLays[4][4] = {
	{0,1,2,3},
	{3,3,3,2},
	{2,2,0,1},
	{1,0,1,0}
};

int zxGetPort(int port, int hardware) {
	switch (hardware) {
		case HW_ZX48:
			if ((port & 0x01) == 0) port = (port & 0xff00) | 0xfe;
			if ((port & 0x21) == 1) port = 0x1f;
			break;
		case HW_PENT:
			if ((port & 0x8002) == 0x0000) port = 0x7ffd;
			if ((port & 0xc002) == 0x8000) port = 0xbffd;
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			if ((port & 0x05a1) == 0x0081) port = 0xfadf;
			if ((port & 0x05a1) == 0x0181) port = 0xfbdf;
			if ((port & 0x05a1) == 0x0581) port = 0xffdf;
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;	// TODO: orly
			if ((port & 0x00ff) == 0x001f) port = 0x1f;			// TODO: orly
			break;
		case HW_P1024:
			if ((port & 0x8002) == 0x0000) port = 0x7ffd;
			if ((port & 0xc002) == 0x8000) port = 0xbffd;
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			if ((port & 0xf008) == 0xe000) port = 0xeff7;
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;	// TODO: orly
			// if ((port & 0x00ff) == 0x001f) port = 0x1f;			// TODO: P1024 doesn't have Kempston
			break;
		case HW_SCORP:
			if ((port & 0x0023) == 0x0001) port = 0x00dd;		// printer
			if ((port & 0x0523) == 0x0003) port = 0xfadf;		// mouse
			if ((port & 0x0523) == 0x0103) port = 0xfbdf;
			if ((port & 0x0523) == 0x0503) port = 0xffdf;
			if ((port & 0xc023) == 0x0021) port = 0x1ffd;		// mem
			if ((port & 0xc023) == 0x4021) port = 0x7ffd;
			if ((port & 0xc023) == 0x8021) port = 0xbffd;		// ay
			if ((port & 0xc023) == 0xc021) port = 0xfffd;
			if ((port & 0x0023) == 0x0022) port = (port & 0xff00) | 0xfe;	// fe
			if ((port & 0x0023) == 0xc023) port = 0x00ff;		// ff
			if ((port & 0x00ff) == 0x001f) port = 0x1f;		// TODO: orly
			break;
	}
	return port;
}

uint8_t prt0,prt1,prt2;

void zxMapMemory(ZXComp* comp) {
	uint8_t rp;
	prt0 = comp->prt0;
	prt1 = comp->prt1;
	prt2 = comp->prt2;
	switch (comp->hw->type) {
		case HW_ZX48:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->bdi->flag & BDI_ACTIVE) ? 3 : 1);
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
			break;
		case HW_PENT:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->bdi->flag & BDI_ACTIVE) ? 3 : ((prt0 & 0x10) >> 4));
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(prt0 & 7) | ((prt0 & 0xc0) >> 3));
			break;
		case HW_P1024:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->bdi->flag & BDI_ACTIVE) ? 3 : ((prt1 & 8) ? 0xff : ((prt0 & 0x10) >> 4)));
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(prt0 & 7) | ((prt1 & 4) ? 0 : ((prt0 & 0x20) | ((prt0 & 0xc0) >> 3))));
			break;
		case HW_SCORP:
			rp = (prt1 & 0x01) ? 0xff : ((prt1 & 0x02) ? 2 : ((comp->bdi->flag & BDI_ACTIVE) ? 3 : ((prt0 & 0x10) >> 4)));
			rp |= ((prt2 & 3) << 2);
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,rp);
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(prt0 & 7) | ((prt1 & 0x10) >> 1) | ((prt1 & 0xc0) >> 2));
			break;
	}
}

Z80EX_BYTE memrd(Z80EX_CONTEXT*,Z80EX_WORD adr,int m1,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	if ((comp->hw->type == HW_SCORP) && ((memGet(comp->mem,MEM_ROM) & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3] & memGet(comp->mem,MEM_PROFMASK);
		zxMapMemory(comp);
	}
	if (m1 == 1) {
		if (comp->rzxPlay) comp->rzxFetches--;
		if (comp->bdi->flag & BDI_ENABLE) {
			if (!(comp->bdi->flag & BDI_ACTIVE) && ((adr & 0xff00) == 0x3d00) && (comp->prt0 & 0x10)) {
				comp->bdi->flag |= BDI_ACTIVE;
				zxMapMemory(comp);
			}
			if ((comp->bdi->flag & BDI_ACTIVE) && (adr > 0x3fff)) {
				comp->bdi->flag &= ~BDI_ACTIVE;
				zxMapMemory(comp);
			}
		}
	}
	Z80EX_BYTE res = memRd(comp->mem,adr);
	return res;
}

void memwr(Z80EX_CONTEXT*,Z80EX_WORD adr,Z80EX_BYTE val,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	memWr(comp->mem,adr,val);
}

Z80EX_BYTE iord(Z80EX_CONTEXT*,Z80EX_WORD port,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	Z80EX_BYTE res = 0xff;
	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
	if (ideIn(comp->ide,port,&res,comp->bdi->flag & BDI_ACTIVE)) return res;
	if (gsIn(comp->gs,port,&res) == GS_OK) return res;
	if (bdiIn(comp->bdi,port,&res)) return res;
	port = zxGetPort(port,comp->hw->type);
	if (comp->rzxPlay) {
		if (comp->rzxPos < comp->rzx[comp->rzxFrame].in.size()) {
			res = comp->rzx[comp->rzxFrame].in[comp->rzxPos];
			comp->rzxPos++;
			return res;
		} else {
	//		printf("RZX: too many IN for frame %lu\n",(long unsigned int)rzxFrame);
			return 0xff;
		}
	}
	switch (port) {
		case 0xfbdf: res = comp->mouse->xpos; break;
		case 0xffdf: res = comp->mouse->ypos; break;
		case 0xfadf: res = comp->mouse->buttons; break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
		default:
			switch (port & 0xff) {
				case 0xfe:
					tapSync(comp->tape,comp->tapCount);
					comp->tapCount = 0;
					res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
					break;
				case 0x1f:
					res = joyInput(comp->joy);
					break;
				default:
					switch (comp->hw->type) {
						case HW_ZX48:
							break;
						case HW_PENT:
							break;
						case HW_P1024:
							break;
						case HW_SCORP:
							switch (port) {
								case 0x7ffd:
									zxSetFrq(comp,7.0);
									break;
								case 0x1ffd:
									zxSetFrq(comp,3.5);
									break;
								case 0xff:
									if (((comp->vid->curr.h - comp->vid->bord.h) < 256) && ((comp->vid->curr.v - comp->vid->bord.v) < 192)) {
										res = comp->vid->atrbyte;
									} else {
										res = 0xff;
									}
									break;
							}
							break;
					}
					break;
				}
				break;
		}
		return res;
}

void iowr(Z80EX_CONTEXT*,Z80EX_WORD port,Z80EX_BYTE val,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
	if (ideOut(comp->ide,port,val,comp->bdi->flag & BDI_ACTIVE)) return;
	if (gsOut(comp->gs,port,val) == GS_OK) return;
	if (bdiOut(comp->bdi,port,val)) return;
	port = zxGetPort(port,comp->hw->type);
	switch (port) {
		case 0xfffd:
		case 0xbffd:
			tsOut(comp->ts,port,val);
			break;
		default:
			if ((port & 0xff) == 0xfe) {
				comp->vid->nextBorder = val & 0x07;
				comp->beeplev = val & 0x10;
				tapSync(comp->tape,comp->tapCount);
				comp->tapCount = 0;
				comp->tape->outsig = (val & 0x08) ? true : false;
			} else {
				switch (comp->hw->type) {
					case HW_ZX48:
						break;
					case HW_PENT:
						switch(port) {
							case 0x7ffd:
								if (comp->block7ffd) break;
								comp->prt0 = val;
								comp->vid->curscr = val & 0x08;
								comp->block7ffd = val & 0x20;
								zxMapMemory(comp);
								break;
						}
						break;
					case HW_P1024:
						switch(port) {
							case 0x7ffd:
								if (comp->block7ffd) break;
								comp->vid->curscr = val & 0x08;
								comp->prt0 = val;
								comp->block7ffd = ((comp->prt1 & 4) && (val & 0x20));
								zxMapMemory(comp);
								break;
							case 0xeff7:
								comp->prt1 = val;
								comp->vid->mode = (val & 1) ? VID_ALCO : VID_NORMAL;
								zxSetFrq(comp, (val & 16) ? 7.0 : 3.5);
								zxMapMemory(comp);
								break;
						}
						break;
					case HW_SCORP:
						switch(port) {
							case 0x7ffd:
								if (comp->block7ffd) break;
								comp->prt0 = val;
								comp->vid->curscr = val & 0x08;
								comp->block7ffd = val & 0x20;
								zxMapMemory(comp);
								break;
							case 0x1ffd:
								comp->prt1 = val;
								zxMapMemory(comp);
								break;
						}
						break;
				}
			}
	}
}

Z80EX_BYTE intrq(Z80EX_CONTEXT*,void*) {
	return 0xff;
}

ZXComp* zxCreate() {
	ZXComp* comp = new ZXComp;
	void* ptr = (void*)comp;
	comp->cpu = z80ex_create(&memrd,ptr,&memwr,ptr,&iord,ptr,&iowr,ptr,&intrq,ptr);
	zxSetFrq(comp,3.5);
	comp->mem = memCreate();
	comp->vid = vidCreate(comp->mem);
	comp->keyb = keyCreate();
	comp->joy = joyCreate();
	comp->mouse = mouseCreate();
	comp->tape = tapCreate();
	comp->bdi = bdiCreate();
	comp->ide = ideCreate(IDE_NONE);
	comp->ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
	comp->gs = gsCreate();
	gsReset(comp->gs);
	zxReset(comp,RES_DEFAULT);
	comp->gsCount = 0;
	comp->tapCount = 0;
	return comp;
}

void zxDestroy(ZXComp* comp) {
	delete(comp);
}

void zxReset(ZXComp* comp,int wut) {
	comp->rzxPlay = false;
	comp->block7ffd=false;
	int resto = comp->resbank;
	switch (wut) {
		case RES_48: resto = 1; break;
		case RES_128: resto = 0; break;
		case RES_DOS: resto = 3; break;
		case RES_SHADOW: resto = 2; break;
	}
	comp->prt2 = 0;
	comp->prt1 = 0;
	comp->prt0 = ((resto & 1) << 4);
	memSetBank(comp->mem,MEM_BANK0,MEM_ROM,resto);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
	comp->rzx.clear();
	comp->rzxPlay = false;
	z80ex_reset(comp->cpu);
	comp->vid->curscr = false;
	comp->vid->mode = VID_NORMAL;
	comp->bdi->flag &= ~BDI_ACTIVE;
	if (resto == 3) comp->bdi->flag |= BDI_ACTIVE;
	bdiReset(comp->bdi);
	if (comp->gs->flag & GS_RESET) gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
}

void zxSetFrq(ZXComp* comp, float frq) {
	comp->cpuFrq = frq;
	comp->dotPerTick = 7.0 / frq;
}

void zxOut(ZXComp* comp, Z80EX_WORD port,Z80EX_BYTE val) {
	iowr(NULL,port,val,(void*)comp);
}

double ltk;
int res1 = 0;
int res2 = 0;
Z80EX_WORD pcreg;

double zxExec(ZXComp* comp) {
	res1 = 0;
	do {
		res1 += z80ex_step(comp->cpu);
	} while (z80ex_last_op_type(comp->cpu) != 0);
	vidSync(comp->vid,res1 * comp->dotPerTick);
	pcreg = z80ex_get_reg(comp->cpu,regPC);
	if (comp->rzxPlay) {
		comp->intStrobe = (comp->rzxFetches < 1);
	} else {
		comp->intStrobe = comp->vid->intStrobe;
	}

	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzxPlay) {
		res2 = z80ex_nmi(comp->cpu);
		res1 += res2;
		if (res2 != 0) {
			comp->bdi->flag |= BDI_ACTIVE;
			zxMapMemory(comp);
			vidSync(comp->vid,res2 * comp->dotPerTick);
		}
	}
	if (comp->intStrobe) {
		res2 = z80ex_int(comp->cpu);
		res1 += res2;
		vidSync(comp->vid,res2 * comp->dotPerTick);
		if (comp->rzxPlay) {
			comp->rzxFrame++;
			if (comp->rzxFrame >= comp->rzx.size()) {
				comp->rzxPlay = false;
				comp->rzx.clear();
			} else {
				comp->rzxFetches = comp->rzx[comp->rzxFrame].fetches;
				comp->rzxPos = 0;
			}
		}
	}

	ltk = res1 * comp->dotPerTick;

	if (comp->gs->flag & GS_ENABLE) comp->gsCount += ltk;
	comp->tapCount += ltk;
	if (comp->bdi->flag & BDI_ENABLE) bdiSync(comp->bdi,ltk);
	return ltk;
}
