#include <stdio.h>
#include <stdlib.h>

#include "spectrum.h"

double ltk;
int res1 = 0;
int res2 = 0;
int res3 = 0;	// tick in op, wich has last OUT/MWR (and vidSync)
int res4 = 0;	// save last res3 (vidSync on OUT/MWR process do res3-res4 ticks)
int vflg = 0;
Z80EX_WORD pcreg;

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

unsigned char ZSLays[4][4] = {
	{0,1,2,3},
	{3,3,3,2},
	{2,2,0,1},
	{1,0,1,0}
};

// layout for +2a extend memory mode

unsigned char plus2Lays[4][4] = {
	{0,1,2,3},
	{4,5,6,7},
	{4,5,6,3},
	{4,7,6,3}
};

Z80EX_WORD zxGetPort(ZXComp* comp, Z80EX_WORD port) {
	switch (comp->hw->type) {
		case HW_ZX48:
			if ((port & 0x01) == 0) {port = (port & 0xff00) | 0xfe; break;}
			if ((port & 0x21) == 1) {port = 0x1f; break;}
			break;
		case HW_PENT:
			if ((port & 0x8002) == 0x0000) port = 0x7ffd;
			if ((port & 0xc002) == 0x8000) port = 0xbffd;
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			if (comp->mouse->flags & INF_ENABLED) {
				if ((port & 0x05a1) == 0x0081) port = 0xfadf;		// mouse
				if ((port & 0x05a1) == 0x0181) port = 0xfbdf;
				if ((port & 0x05a1) == 0x0581) port = 0xffdf;
			}
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;	// TODO: orly
			if ((port & 0x00ff) == 0x001f) port = 0x1f;			// TODO: orly
			break;
		case HW_P1024:
			if ((port & 0x8002) == 0x0000) port = 0x7ffd;
			if ((port & 0xc002) == 0x8000) port = 0xbffd;
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			if ((port & 0xf008) == 0xe000) port = 0xeff7;
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;	// TODO: orly
			if (comp->mouse->flags & INF_ENABLED) {				// mouse
				if ((port & 0x05a1) == 0x0081) port = 0xfadf;		// TODO: orly
				if ((port & 0x05a1) == 0x0181) port = 0xfbdf;
				if ((port & 0x05a1) == 0x0581) port = 0xffdf;
			}
			// if ((port & 0x00ff) == 0x001f) port = 0x1f;			// TODO: P1024 doesn't have Kempston
			break;
		case HW_SCORP:
			if ((port & 0x0023) == 0x0001) port = 0x00dd;		// printer (covox)
			if (comp->mouse->flags & INF_ENABLED) {
				if ((port & 0x0523) == 0x0003) port = 0xfadf;		// mouse
				if ((port & 0x0523) == 0x0103) port = 0xfbdf;
				if ((port & 0x0523) == 0x0503) port = 0xffdf;
			}
			if ((port & 0xc023) == 0x0021) port = 0x1ffd;		// mem
			if ((port & 0xc023) == 0x4021) port = 0x7ffd;
			if ((port & 0xc023) == 0x8021) port = 0xbffd;		// ay
			if ((port & 0xc023) == 0xc021) port = 0xfffd;
			if ((port & 0x0023) == 0x0022) port = (port & 0xff00) | 0xfe;	// fe
			if ((port & 0x0023) == 0xc023) port = 0x00ff;		// ff
			if ((port & 0x00ff) == 0x001f) port = 0x1f;		// TODO: orly
			break;
		case HW_PLUS3:
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;
			if ((port & 0xf002) == 0x0000) port = 0x0ffd;	// ? printer
			if ((port & 0xf002) == 0x1000) port = 0x1ffd;
			if ((port & 0xf002) == 0x2000) port = 0x2ffd;	// TODO: 8272status
			if ((port & 0xf002) == 0x3000) port = 0x3ffd;	// TODO: 8272data
			if ((port & 0xc002) == 0x4000) port = 0x7ffd;	// 7ffd
			if ((port & 0xc002) == 0x8000) port = 0xbffd;		// ay
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			break;
		case HW_PLUS2:
			if ((port & 0x0003) == 0x0002) port = (port & 0xff00) | 0xfe;	// TODO: check it
			if ((port & 0xc002) == 0x4000) port = 0x7ffd;		// mem
			if ((port & 0xf002) == 0x1000) port = 0x1ffd;
			if ((port & 0xc002) == 0x8000) port = 0xbffd;		// ay
			if ((port & 0xc002) == 0xc000) port = 0xfffd;
			break;
	}
	return port;
}

#define PRT0	comp->prt0
#define	PRT1	comp->prt1
#define	PRT2	comp->prt2

void zxMapMemory(ZXComp* comp) {
	Z80EX_BYTE rp;
	switch (comp->hw->type) {
		case HW_ZX48:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->bdi->flag & BDI_ACTIVE) ? 3 : 1);
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
			break;
		case HW_PENT:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->bdi->flag & BDI_ACTIVE) ? 3 : ((PRT0 & 0x10) >> 4));
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(PRT0 & 7) | ((PRT0 & 0xc0) >> 3));
			break;
		case HW_P1024:
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->bdi->flag & BDI_ACTIVE) ? 3 : ((PRT1 & 8) ? 0xff : ((PRT0 & 0x10) >> 4)));
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(PRT0 & 7) | ((PRT1 & 4) ? 0 : ((PRT0 & 0x20) | ((PRT0 & 0xc0) >> 3))));
			break;
		case HW_SCORP:
			rp = (PRT1 & 0x01) ? 0xff : ((PRT1 & 0x02) ? 2 : ((comp->bdi->flag & BDI_ACTIVE) ? 3 : ((PRT0 & 0x10) >> 4)));
			rp |= ((PRT2 & 3) << 2);
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,rp);
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,(PRT0 & 7) | ((PRT1 & 0x10) >> 1) | ((PRT1 & 0xc0) >> 2));
			break;
		case HW_PLUS3:
		case HW_PLUS2:
			if (PRT1 & 1) {
				// extend mem mode
				rp = ((PRT1 & 0x06) >> 1);	// b1,2 of 1ffd
				memSetBank(comp->mem,MEM_BANK0,MEM_RAM,plus2Lays[rp][0]);
				memSetBank(comp->mem,MEM_BANK1,MEM_RAM,plus2Lays[rp][1]);
				memSetBank(comp->mem,MEM_BANK2,MEM_RAM,plus2Lays[rp][2]);
				memSetBank(comp->mem,MEM_BANK3,MEM_RAM,plus2Lays[rp][3]);
			} else {
				// normal mem mode
				memSetBank(comp->mem,MEM_BANK0,MEM_ROM,((PRT0 & 0x10) >> 4) | ((PRT1 & 0x04) >> 1));
				memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
				memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
				memSetBank(comp->mem,MEM_BANK3,MEM_RAM,PRT0 & 7);
			}
			break;
	}
}

Z80EX_BYTE memrd(Z80EX_CONTEXT* cpu,Z80EX_WORD adr,int m1,void* ptr) {
	Z80EX_BYTE res;
	ZXComp* comp = (ZXComp*)ptr;
	if ((comp->hw->type == HW_SCORP) && ((comp->mem->crom & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3] & comp->mem->profMask;
		zxMapMemory(comp);
	}
	if (m1 == 1) {
		if (comp->rzxPlay) comp->rzxFetches--;
		if (comp->bdi->fdc->type == FDC_93) {
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
	res3 = res2 + z80ex_op_tstate(cpu);
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	if (((adr & 0xc000) == 0x4000) && (comp->vid->flags & VID_SLOWMEM))
		vidWaitSlow(comp->vid);
	res = memRd(comp->mem,adr);
	return res;
}

// NOTE (2012.07.14)
// real OUT or MWR must be AFTER vidSync()

void memwr(Z80EX_CONTEXT* cpu, Z80EX_WORD adr, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	res3 = res2 + z80ex_op_tstate(cpu);
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	if (((adr & 0xc000) == 0x4000) && (comp->vid->flags & VID_SLOWMEM))
		vidWaitSlow(comp->vid);
	memWr(comp->mem,adr,val);
	if (comp->mem->flags & MEM_BRK_WRITE) {
		comp->flags |= ZX_BREAK;
	}
}

Z80EX_BYTE iord(Z80EX_CONTEXT* cpu, Z80EX_WORD port, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	Z80EX_BYTE res = 0xff;
	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
	if (comp->rzxPlay) {
		if (comp->rzxPos < comp->rzxData[comp->rzxFrame].frmSize) {
			res = comp->rzxData[comp->rzxFrame].frmData[comp->rzxPos];
			comp->rzxPos++;
			return res;
		} else {
			return 0xff;
		}
	}
	if (ideIn(comp->ide,port,&res,comp->bdi->flag & BDI_ACTIVE)) return res;
	if (gsIn(comp->gs,port,&res) == GS_OK) return res;
	if (bdiIn(comp->bdi,port,&res)) return res;
	port = zxGetPort(comp,port);
	switch (port) {
		case 0xfbdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->xpos : 0xff; break;
		case 0xffdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->ypos : 0xff; break;
		case 0xfadf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->buttons : 0xff; break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
		default:
			switch (port & 0xff) {
				case 0xfe:
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
						case HW_PLUS3:
							if (comp->bdi->fdc->type == FDC_765) {
								switch (port) {
									case 0x2ffd:		// read main status register
										res = fdcRd(comp->bdi->fdc,FDC_STATE);
										break;
									case 0x3ffd:		// read data
										res = fdcRd(comp->bdi->fdc,FDC_DATA);
										break;
								}
							}
						case HW_PLUS2:
							break;
					}
					break;
				}
				break;
		}
		return res;
}

void zxOut(ZXComp *comp, Z80EX_WORD port, Z80EX_BYTE val) {
	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
	if (ideOut(comp->ide,port,val,comp->bdi->flag & BDI_ACTIVE)) return;
	if (gsOut(comp->gs,port,val) == GS_OK) return;
	if (bdiOut(comp->bdi,port,val)) return;
	port = zxGetPort(comp,port);
	switch (port) {
		case 0xfffd:
		case 0xbffd:
			tsOut(comp->ts,port,val);
			break;
		default:
			if ((port & 0xff) == 0xfe) {
				comp->vid->nextbrd = val & 0x07;
				if (!(comp->vid->flags & VID_BORDER_4T)) {
					comp->vid->brdcol = val & 0x07;
				}
				comp->beeplev = val & 0x10;
				comp->tape->outsig = (val & 0x08) ? 1 : 0;
			} else {
				switch (comp->hw->type) {
					case HW_ZX48:
						break;
					case HW_PENT:
						sdrvOut(comp->sdrv,port & 0x00ff,val);
						switch(port) {
							case 0x7ffd:
								if (comp->block7ffd) break;
								comp->prt0 = val;
								comp->vid->curscr = (val & 0x08) ? 1 : 0;
								comp->block7ffd = val & 0x20;
								zxMapMemory(comp);
								break;
						}
						break;
					case HW_P1024:
						switch(port) {
							case 0x7ffd:
								if (comp->block7ffd) break;
								comp->vid->curscr = (val & 0x08) ? 1 : 0;
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
								comp->vid->curscr = (val & 0x08) ? 1 : 0;
								comp->block7ffd = val & 0x20;
								zxMapMemory(comp);
								break;
							case 0x1ffd:
								comp->prt1 = val;
								zxMapMemory(comp);
								break;
							case 0x00dd:
								sdrvOut(comp->sdrv,0xfb,val);
								break;
						}
						break;
					case HW_PLUS3:
						if (comp->bdi->fdc->type == FDC_765) {
							switch (port) {
								case 0x2ffd:		// motor control
									if (val & 1) comp->bdi->fdc->flop[0]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[0]->flag &= ~FLP_MOTOR;
									if (val & 2) comp->bdi->fdc->flop[1]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[1]->flag &= ~FLP_MOTOR;
									if (val & 4) comp->bdi->fdc->flop[2]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[2]->flag &= ~FLP_MOTOR;
									if (val & 8) comp->bdi->fdc->flop[3]->flag |= FLP_MOTOR; else comp->bdi->fdc->flop[3]->flag &= ~FLP_MOTOR;
									break;
								case 0x3ffd:		// write data
									fdcWr(comp->bdi->fdc,FDC_DATA,val);
									break;
							}
						}
					case HW_PLUS2:
						switch (port) {
							case 0x7ffd:
								if (comp->block7ffd) break;
								comp->prt0 = val;
								comp->vid->curscr = (val & 0x08) ? 1 : 0;
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

void iowr(Z80EX_CONTEXT* cpu, Z80EX_WORD port, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	res3 = res2 + z80ex_op_tstate(cpu) - 3;
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	zxOut(comp,port,val);
}

Z80EX_BYTE intrq(Z80EX_CONTEXT* cpu, void* ptr) {
	return 0xff;
}

void rzxClear(ZXComp* zx) {
	int i;
	for (i = 0; i < zx->rzxSize; i++) {
		if (zx->rzxData[i].frmData) free(zx->rzxData[i].frmData);
	}
	if (zx->rzxData) free(zx->rzxData);
	zx->rzxSize = 0;
	zx->rzxData = NULL;
}

ZXComp* zxCreate() {
	ZXComp* comp = (ZXComp*)malloc(sizeof(ZXComp));
	void* ptr = (void*)comp;
	comp->flags = ZX_JUSTBORN;
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
	comp->sdrv = sdrvCreate(SDRV_105_2);
	comp->rzxSize = 0;
	comp->rzxData = NULL;
	comp->gsCount = 0;
	comp->resbank = RES_48;
	comp->tickCount = 0;
	gsReset(comp->gs);
	zxReset(comp,RES_DEFAULT);
	return comp;
}

void zxDestroy(ZXComp* comp) {
	z80ex_destroy(comp->cpu);
	memDestroy(comp->mem);
	vidDestroy(comp->vid);
	keyDestroy(comp->keyb);
	joyDestroy(comp->joy);
	mouseDestroy(comp->mouse);
	tapDestroy(comp->tape);
	bdiDestroy(comp->bdi);
	ideDestroy(comp->ide);
	tsDestroy(comp->ts);
	gsDestroy(comp->gs);
	sdrvDestroy(comp->sdrv);
	if (comp->rzxData) free(comp->rzxData);
	free(comp);
}

void zxReset(ZXComp* comp,int wut) {
	int resto = comp->resbank;
	comp->rzxPlay = 0;
	comp->block7ffd = 0;
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
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
	memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
	rzxClear(comp);
	comp->rzxPlay = 0;
	z80ex_reset(comp->cpu);
	comp->vid->curscr = 0;
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

double zxExec(ZXComp* comp) {
	res1 = res2 = res3 = res4 = 0;
	vflg = 0;
	do {
		res2 += z80ex_step(comp->cpu);
	} while (z80ex_last_op_type(comp->cpu) != 0);
	pcreg = z80ex_get_reg(comp->cpu,regPC);
	vflg |= vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
	res1 = res2;
	if (comp->rzxPlay) {
		comp->intStrobe = (comp->rzxFetches < 1);
	} else {
		comp->intStrobe = (vflg & VID_INT) ? 1 : 0;
	}
	comp->frmStrobe = (vflg & VID_FRM) ? 1 : 0;
	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzxPlay) {
		res3 = res4 = 0;
		res2 = z80ex_nmi(comp->cpu);
		res1 += res2;
		if (res2 != 0) {
			comp->bdi->flag |= BDI_ACTIVE;
			zxMapMemory(comp);
			vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
		}
	}
	if (comp->intStrobe) {
		res3 = res4 = 0;
		res2 = z80ex_int(comp->cpu);
		res1 += res2;
		vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
		if (comp->rzxPlay) {
			comp->rzxFrame++;
			if (comp->rzxFrame >= comp->rzxSize) {
				comp->rzxPlay = 0;
				comp->rzxSize = 0;
				if (comp->rzxData) free(comp->rzxData);
				comp->rzxData = NULL;
			} else {
				comp->rzxFetches = comp->rzxData[comp->rzxFrame].fetches;
				comp->rzxPos = 0;
			}
		}
	}
	pcreg = z80ex_get_reg(comp->cpu,regPC);
	if (memGetCellFlags(comp->mem,pcreg) & MEM_BRK_FETCH) {
		comp->flags |= ZX_BREAK;
	}

	ltk = res1 * comp->dotPerTick;
	comp->tickCount += res1;
	tapSync(comp->tape,ltk);

	if (comp->gs->flag & GS_ENABLE) comp->gsCount += ltk;
	if (comp->bdi->fdc->type != FDC_NONE) bdiSync(comp->bdi,ltk);
	return ltk;
}
