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

Z80EX_BYTE memrd(Z80EX_CONTEXT*,Z80EX_WORD adr,int m1,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	if ((comp->hw->type == HW_SCORP) && ((memGet(comp->mem,MEM_ROM) & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3] & memGet(comp->mem,MEM_PROFMASK);
		comp->mapMemory();
	}
	if ((m1 == 1) && comp->bdi->enable) {
		if (!comp->bdi->active && ((adr & 0xff00) == 0x3d00) && (comp->prt0 & 0x10)) {
			comp->bdi->active = true;
			comp->mapMemory();
		}
		if (comp->bdi->active && (adr > 0x3fff)) {
			comp->bdi->active = false;
			comp->mapMemory();
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
	return comp->in(port);
}

void iowr(Z80EX_CONTEXT*,Z80EX_WORD port,Z80EX_BYTE val,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	comp->out(port,val);
}

Z80EX_BYTE intrq(Z80EX_CONTEXT*,void*) {
	return 0xff;
}

ZXComp::ZXComp() {
	void* ptr = (void*)this;
	cpu = z80ex_create(&memrd,ptr,&memwr,ptr,&iord,ptr,&iowr,ptr,&intrq,ptr);
	cpuFreq = 3.5;
	mem = memCreate();
	vid = vidCreate(mem);
	keyb = keyCreate();
	mouse = mouseCreate();
	tape = tapCreate();
	bdi = bdiCreate();
	ide = ideCreate(IDE_NONE);
	ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
	gs = gsCreate();
	gsReset(gs);
	reset(RES_DEFAULT);
	gsCount = 0;
	tapCount = 0;
}

void ZXComp::reset(int wut) {
	rzxPlay = false;
	block7ffd=false;
	int resto = resbank;
	switch (wut) {
		case RES_48: resto = 1; break;
		case RES_128: resto = 0; break;
		case RES_DOS: resto = 3; break;
		case RES_SHADOW: resto = 2; break;
	}
	prt2 = 0;
	prt1 = 0;
	prt0 = ((resbank & 1) << 4);
	memSetBank(mem,MEM_BANK0,MEM_ROM,resto);
	memSetBank(mem,MEM_BANK3,MEM_RAM,0);
//	mem->rzx.clear();
	z80ex_reset(cpu);
	vid->curscr = false;
	vid->mode = VID_NORMAL;
	bdi->active = (resto == 3);
	bdi->vg93.count = 0;
	bdi->vg93.setmr(false);
	if (gsGet(gs,GS_FLAG) & GS_RESET) gsReset(gs);
	tsReset(ts);
	ideReset(ide);
}

void ZXComp::mapMemory() {
	uint8_t rp;
	switch (hw->type) {
		case HW_ZX48:
			memSetBank(mem,MEM_BANK0,MEM_ROM,bdi->active ? 3 : 1);
			memSetBank(mem,MEM_BANK3,MEM_RAM,0);
			break;
		case HW_PENT:
			memSetBank(mem,MEM_BANK0,MEM_ROM,(bdi->active) ? 3 : ((prt0 & 0x10) >> 4));
			memSetBank(mem,MEM_BANK3,MEM_RAM,(prt0 & 7) | ((prt0 & 0xc0) >> 3));
			break;
		case HW_P1024:
			memSetBank(mem,MEM_BANK0,MEM_ROM,bdi->active ? 3 : ((prt1 & 8) ? 0xff : ((prt0 & 0x10) >> 4)));
			memSetBank(mem,MEM_BANK3,MEM_RAM,(prt0 & 7) | ((prt1 & 4) ? 0 : ((prt0 & 0x20) | ((prt0 & 0xc0) >> 3))));
			break;
		case HW_SCORP:
			rp = (prt1 & 0x01) ? 0xff : ((prt1 & 0x02) ? 2 : ((bdi->active) ? 3 : ((prt0 & 0x10) >> 4)));
			rp |= ((prt2 & 3) << 2);
			memSetBank(mem,MEM_BANK0,MEM_ROM,rp);
			memSetBank(mem,MEM_BANK3,MEM_RAM,(prt0 & 7) | ((prt1 & 0x10) >> 1) | ((prt1 & 0xc0) >> 2));
			break;
	}
}

int zxGetPort(int port, int hardware) {
	switch (hardware) {
		case HW_ZX48:
			if ((port & 0x01) == 0) {
				port = (port & 0xff00) | 0xfe;
			}
			break;
		case HW_PENT:
			if ((port & 0x8002) == 0x0000) port = 0x7ffd;
			if ((port & 0xc002) == 0x8000) port = 0xbffd;
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			if ((port & 0x05a1) == 0x0081) port = 0xfadf;
			if ((port & 0x05a1) == 0x0181) port = 0xfbdf;
			if ((port & 0x05a1) == 0x0581) port = 0xffdf;
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;	// TODO: уточнить
			break;
		case HW_P1024:
			if ((port & 0x8002) == 0x0000) port = 0x7ffd;
			if ((port & 0xc002) == 0x8000) port = 0xbffd;
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			if ((port & 0xf008) == 0xe000) port = 0xeff7;
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;	// TODO: уточнить
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
			break;
	}
	return port;
}

uint8_t ZXComp::in(uint16_t port) {
	uint8_t res = 0xff;
	gsSync(gs,gsCount); gsCount = 0;
	if (ideIn(ide,port,&res,bdi->active)) return res;
	if (gsIn(gs,port,&res) == GS_OK) return res;
	if (bdiIn(bdi,port,&res)) return res;
	port = zxGetPort(port,hw->type);
//	if (rzxPlay) return mem->getRZXIn();
	switch (port) {
		case 0xfbdf: res = mouse->xpos; break;
		case 0xffdf: res = mouse->ypos; break;
		case 0xfadf: res = mouse->buttons; break;
		case 0xfffd:
			res = tsIn(ts,port);
			break;
		default:
			switch (port & 0xff) {
				case 0xfe:
					tapSync(tape,tapCount); tapCount = 0;
					res = keyInput(keyb, (port & 0xff00) >> 8) | (tapGetSignal(tape) ? 0x40 : 0x00);
					break;
				case 0x1f:
					break;
				default:
					switch (hw->type) {
						case HW_ZX48:
							break;
						case HW_PENT:
							break;
						case HW_P1024:
							break;
						case HW_SCORP:
							switch (port) {
								case 0x7ffd:
									cpuFreq = 7.0;
									break;
								case 0x1ffd:
									cpuFreq = 3.5;
									break;
								case 0xff:
									if (((vid->curr.h - vid->bord.h) < 256) && ((vid->curr.v - vid->bord.v) < 192)) {
										res = vid->atrbyte;
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

void ZXComp::out(uint16_t port,uint8_t val) {
	gsSync(gs,gsCount); gsCount = 0;
	if (ideOut(ide,port,val,bdi->active)) return;
	if (gsOut(gs,port,val) == GS_OK) return;
	if (bdiOut(bdi,port,val)) return;
	port = zxGetPort(port,hw->type);	
	switch (port) {
		case 0xfffd:
		case 0xbffd:
			tsOut(ts,port,val);
			break;
		default:
			if ((port&0xff) == 0xfe) {
				vid->nextBorder = val & 0x07;
				beeplev = val & 0x10;
				tapSync(tape,tapCount); tapCount = 0;
				tapSetSignal(tape, (val & 0x08) ? true : false);
			} else {
				switch (hw->type) {
					case HW_ZX48:
						break;
					case HW_PENT:
						switch(port) {
							case 0x7ffd:
								if (block7ffd) break;
								prt0 = val;
								vid->curscr = val & 0x08;
								block7ffd = val & 0x20;
								mapMemory();
								break;
						}
						break;
					case HW_P1024:
						switch(port) {
							case 0x7ffd:
								if (block7ffd) break;
								vid->curscr = val & 0x08;
								prt0 = val;
								block7ffd = ((prt1 & 4) && (val & 0x20));
								mapMemory();
								break;
							case 0xeff7:
								prt1 = val;
								vid->mode = (val & 1) ? VID_ALCO : VID_NORMAL;
								cpuFreq = (val & 16) ? 7.0 : 3.5;
								mapMemory();
								break;
						}
						break;
					case HW_SCORP:
						switch(port) {
							case 0x7ffd:
								if (block7ffd) break;
								prt0 = val;
								vid->curscr = val & 0x08;
								block7ffd = val & 0x20;
								mapMemory();
								break;
							case 0x1ffd:
								prt1 = val;
								mapMemory();
								break;
						}
						break;
				}
		}
	}
}

double ltk;
int res1 = 0;
int res2 = 0;

double ZXComp::exec() {
	res1 = 0;
	do {
		res1 += z80ex_step(cpu);
	} while (z80ex_last_op_type(cpu) != 0);
	vidSync(vid,res1,cpuFreq);
	intStrobe = vid->intStrobe;

	Z80EX_WORD pc = z80ex_get_reg(cpu,regPC);

	if ((pc > 0x3fff) && nmiRequest) {
		res2 = z80ex_nmi(cpu);
		res1 += res2;
		if (res2 != 0) {
			bdi->active = true;
			mapMemory();
			vidSync(vid,res2,cpuFreq);
		}
	}
	if (intStrobe) {
		res2 = z80ex_int(cpu);
		res1 += res2;
		vidSync(vid,res2,cpuFreq);
	}

	ltk = res1 * 7.0 / cpuFreq;

	if (gsGet(gs,GS_FLAG) & GS_ENABLE) gsCount += ltk;
	tapCount += ltk;
	if (bdi->enable) bdiSync(bdi,ltk);
	return ltk;
}
