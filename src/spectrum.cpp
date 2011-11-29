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

Z80EX_BYTE memrd(Z80EX_CONTEXT*,Z80EX_WORD adr,int,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	Z80EX_BYTE res = memRd(comp->mem,adr);
	if ((comp->hw->type == HW_SCORP) && ((memGet(comp->mem,MEM_ROM) & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3] & memGet(comp->mem,MEM_PROFMASK);
		comp->mapMemory();
	}
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
	vid = new Video(mem);
	keyb = new Keyboard;
	mouse = new Mouse;
	tape = new Tape;
	bdi = new BDI;
	ide = new IDE;
	ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
	gs = gsCreate();
	gsReset(gs);
	reset(RES_DEFAULT);
}

void ZXComp::reset(int wut) {
	rzxPlay = false;
	block7ffd=false;
	int resbank = res;
	switch (wut) {
		case RES_48: resbank = 1; break;
		case RES_128: resbank = 0; break;
		case RES_DOS: resbank = 3; break;
		case RES_SHADOW: resbank = 2; break;
	}
	prt2 = 0;
	prt1 = 0;
	prt0 = ((resbank & 1) << 4);
	memSetBank(mem,MEM_BANK0,MEM_ROM,resbank);
	memSetBank(mem,MEM_BANK3,MEM_RAM,0);
//	mem->rzx.clear();
	z80ex_reset(cpu);
	vid->curscr = false;
	vid->mode = VID_NORMAL;
	bdi->active = (resbank == 3);
	bdi->vg93.count = 0;
	bdi->vg93.setmr(false);
	if (gsGetFlag(gs) & GS_RESET) gsReset(gs);
	tsReset(ts);
	ide->reset();
}

void ZXComp::mapMemory() {
	uint8_t rp;
	switch (hw->type) {
		case HW_ZX48:
			memSetBank(mem,MEM_BANK0,MEM_ROM,bdi->active?3:1);
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

int32_t ZXComp::getPort(int32_t port) {
	switch (hw->type) {
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
//	gsSync(gs,vid->t);
	if (ide->in(port,&res,bdi->active)) return res;
	if (gsIn(gs,port,&res) == GS_OK) return res;
	if (bdi->in(port,&res)) return res;
	port = getPort(port);
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
					tape->sync();
					res = keyb->getmap((port & 0xff00) >> 8) | (tape->signal ? 0x40 : 0x00);
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
//	gsSync(gs,vid->t);
	if (ide->out(port,val,bdi->active)) return;
	if (gsOut(gs,port,val) == GS_OK) return;
	if (bdi->out(port,val)) return;
	port = getPort(port);	
	switch (port) {
		case 0xfffd:
		case 0xbffd:
			tsOut(ts,port,val);
			break;
		default:
			if ((port&0xff) == 0xfe) {
				vid->nextBorder = val & 0x07;
				beeplev = val & 0x10;
				tape->outsig = val & 0x08;
				tape->sync();
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

double tk;

uint32_t ZXComp::exec() {
	uint32_t ltk = vid->t;
	int res = 0;
	do {
		res += z80ex_step(cpu);
	} while (z80ex_last_op_type(cpu) != 0);
	vid->sync(res,cpuFreq);
	intStrobe = vid->intStrobe;

	Z80EX_WORD pc = z80ex_get_reg(cpu,regPC);

	if ((pc > 0x3fff) && nmiRequest) NMIHandle();
	if (intStrobe) INTHandle();

	ltk = vid->t - ltk;

	if (bdi->enable) {
		bdi->sync(ltk);
		if (bdi->active && (pc > 0x3fff)) {
			bdi->active = false;
			mapMemory();
		}
		if (!bdi->active && ((pc & 0xff00) == 0x3d00) && (prt0 & 0x10)) {
			bdi->active = true;
			mapMemory();
		}
	}
	gsSync(gs,ltk);
	return ltk;
}

void ZXComp::INTHandle() {
	int res = z80ex_int(cpu);
	vid->sync(res,cpuFreq);
}

void ZXComp::NMIHandle() {
	int res = z80ex_nmi(cpu);
	if (res == 0) return;
	bdi->active = true;
	mapMemory();
	vid->sync(res,cpuFreq);
}

/*
ZXBase::ZXBase (ZXSystem* par) {
	parent = par;
	nmi = false;
}

uint8_t ZXBase::in(uint16_t port) {
	return parent->in(port);
}

void ZXBase::out(uint16_t port, uint8_t val) {
	parent->out(port,val);
}

ZOp ZXBase::fetch() {
	ZOp res;
	cpu->err = false;
	if (cpu->nextei) {
		cpu->iff1 = cpu->iff2 = true;
		cpu->nextei = false;
	}
	cpu->mod = 0;
	cpu->ti = cpu->t;
	do {
		cpu->r = ((cpu->r + 1) & 0x7f) | (cpu->r & 0x80);
		cpu->cod = mem->rd(cpu->pc++);
		res = inst[cpu->mod][cpu->cod];
		cpu->t += res.t;
		if (res.flags & ZPREF) res.func(this);
	} while (res.flags & ZPREF);
	switch (res.cond) {
		case CND_NONE: break;
		case CND_Z: cpu->t += (cpu->f & FZ) ? res.tcn1 : res.tcn0; break;
		case CND_C: cpu->t += (cpu->f & FC) ? res.tcn1 : res.tcn0; break;
		case CND_P: cpu->t += (cpu->f & FP) ? res.tcn1 : res.tcn0; break;
		case CND_S: cpu->t += (cpu->f & FS) ? res.tcn1 : res.tcn0; break;
		case CND_DJNZ: cpu->t += (cpu->b != 1) ? res.tcn1 : res.tcn0; break;
		case CND_LDIR: cpu->t += (cpu->bc != 1) ? res.tcn1 : res.tcn0; break;
		case CND_CPIR: cpu->t += ((cpu->bc != 1) && (cpu->a != mem->rd(cpu->hl))) ? res.tcn1 : res.tcn0; break;
		default: printf("undefined condition\n"); throw(0);
	}
	if ((hwflags & IO_WAIT) && (hwflags & WAIT_ON) && (cpu->t & 1)) cpu->t++;		// WAIT
	res.t = cpu->t - cpu->ti;
	return res;
}

int32_t ZXBase::interrupt() {
	if (!cpu->iff1) return 0;
	cpu->iff1 = cpu->iff2 = false;
	int32_t res = 0;
	cpu->r = ((cpu->r + 1) & 0x7f) | (cpu->r & 0x80);	// increase R
	switch (cpu->imode) {
		case 0: mem->wr(--cpu->sp,cpu->hpc);
			mem->wr(--cpu->sp,cpu->lpc);
			cpu->pc = 0x38;
			cpu->mptr = cpu->pc;
			res = 12;
			break;
		case 1: mem->wr(--cpu->sp,cpu->hpc);
			mem->wr(--cpu->sp,cpu->lpc);
			cpu->pc = 0x38;
			cpu->mptr = cpu->pc;
			res = 13;
			break;
		case 2: mem->wr(--cpu->sp,cpu->hpc);
			mem->wr(--cpu->sp,cpu->lpc);
			cpu->adr = (cpu->i << 8) | 0xff;
			cpu->lpc = mem->rd(cpu->adr);
			cpu->hpc = mem->rd(cpu->adr+1);
			cpu->mptr = cpu->pc;
			res = 19;
			break;
	}
	if ((hwflags & IO_WAIT) && (hwflags & WAIT_ON) && (cpu->t & 1)) res++;		// TODO: is INT handle is WAIT'ing too?
	cpu->t += res;
	return res;
}
*/
