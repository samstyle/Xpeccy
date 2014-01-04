#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "spectrum.h"

int nsTime;
int res1 = 0;
int res2 = 0;
int res3 = 0;	// tick in op, wich has last OUT/MWR (and vidSync)
int res4 = 0;	// save last res3 (vidSync on OUT/MWR process do res3-res4 ticks)
int res5 = 0;	// ticks ated by slow mem?
Z80EX_WORD pcreg;

// port decoding tables

/*
int zxGetPort(ZXComp* comp, Z80EX_WORD port) {
	switch (comp->hw->type) {
		case HW_ATM1:
			if ((port & 0x0007) == 0x0006) return 0xfe;	// (to read: a8..15, to write: a3.5.6.7)
			if ((port & 0x8202) == 0x8000) return 0xfdfd;	// mem.ext
			if ((port & 0x8202) == 0x0200) return 0x7ffd;	// mem
			if ((port & 0x8202) == 0x0000) return 0x7dfd;	// digital.rd / palete.wr
			if ((port & 0xc202) == 0x8200) return 0xbffd;	// ay
			if ((port & 0xc202) == 0xc200) return 0xfffd;
			if ((port & 0x0007) == 0x0002) return 0xfa;	// interface port
			if ((port & 0x0007) == 0x0003) return 0xfb;	// digital.wr/printer
			if (comp->dosen & 1) {
				if ((port & 0x009f) == 0x009f) return 0xff;				// bdi: ff
				if ((port & 0x00ff) == 0x001f) return 0x1f;
				if ((port & 0x00ff) == 0x003f) return 0x3f;
				if ((port & 0x00ff) == 0x005f) return 0x5f;
				if ((port & 0x00ff) == 0x007f) return 0x7f;
			}
			break;
		default:
			printf("zxGetPort : unknown hardware type %i\n",comp->hw->type);
			assert(0);
			break;
	}
//	printf("Unimplemented port %.4X (hw = %.2X)\n",port,comp->hw->type);
	return 0x0000;		// no port
}
*/

MemPage* mptr;

inline void zxMemRW(ZXComp* comp, int adr) {
	mptr = memGetBankPtr(comp->mem,adr);
	if ((comp->hwFlag & HW_CONTMEM) && (mptr->type == MEM_RAM) && (mptr->num & 1)) {	// pages 1,3,5,7 (48K model)
		res3 = TCPU(comp->cpu);					// until RD/WR cycle
		vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
		res4 = res3;
		vidWait(comp->vid);					// do WAIT
		vidSync(comp->vid, comp->nsPerTick * 3);		// 3T rd/wr cycle
		res4 += 3;
	} else {
		res3 = TCPU(comp->cpu) + 3;
		vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
		res4 = res3;
	}
}

Z80EX_BYTE memrd(CPUCONT Z80EX_WORD adr,int m1,void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	if (m1 && comp->rzxPlay) comp->rzxFetches--;
	zxMemRW(comp,adr);
	return comp->hw->mrd(comp,adr,m1);
}

void memwr(CPUCONT Z80EX_WORD adr, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	zxMemRW(comp,adr);
	comp->hw->mwr(comp,adr,val);
}

int bdiz;

inline void zxIORW(ZXComp* comp, int port) {
	if (comp->hwFlag & HW_CONTIO) {
		if ((port & 0xc000) == 0x4000) {
			if (port & 0x0001) {			// C:1 C:1 C:1 C:1
				vidWait(comp->vid); vidSync(comp->vid, comp->nsPerTick);
				vidWait(comp->vid); vidSync(comp->vid, comp->nsPerTick);
				vidWait(comp->vid); vidSync(comp->vid, comp->nsPerTick);
				vidWait(comp->vid); vidSync(comp->vid, comp->nsPerTick);
			} else {				// C:1 C:3
				vidWait(comp->vid); vidSync(comp->vid, comp->nsPerTick);
				vidWait(comp->vid); vidSync(comp->vid, 3 * comp->nsPerTick);
			}
		} else {
			if (port & 0x0001) {			// N:4
				vidSync(comp->vid, 4 * comp->nsPerTick);
			} else {				// N:1 C:3
				vidSync(comp->vid, comp->nsPerTick);
				vidWait(comp->vid); vidSync(comp->vid, 3 * comp->nsPerTick);
			}
		}
		res4 += 4;
	} else {
		vidSync(comp->vid, 3 * comp->nsPerTick);
		res4 += 3;
	}
}

Z80EX_BYTE iord(CPUCONT Z80EX_WORD port, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	Z80EX_BYTE res = 0xff;
/*
// video sync ( + cont io)
	res3 = TCPU(comp->cpu);			// start of IN/OUT cycle
	vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
	res4 = res3;
	zxIORW(comp,port);			// cont mem
*/
// tape sync
	tapSync(comp->tape,comp->tapCount);
	comp->tapCount = 0;
// play rzx
	if (comp->rzxPlay) {
		if (comp->rzxPos < comp->rzxData[comp->rzxFrame].frmSize) {
			res = comp->rzxData[comp->rzxFrame].frmData[comp->rzxPos];
			comp->rzxPos++;
			return res;
		} else {
			return 0xff;
		}
	}
	bdiz = ((comp->dosen & 1) && (comp->bdi->fdc->type == FDC_93)) ? 1 : 0;
// request to external devices
	if (ideIn(comp->ide,port,&res,comp->dosen & 1)) return res;
	if (gsIn(comp->gs,port,&res) == GS_OK) return res;
	return comp->hw->in(comp,port,bdiz);
}

void iowr(CPUCONT Z80EX_WORD port, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	comp->padr = port;
	comp->pval = val;
}
/*
// contended mem, move to end of IO cycle
	res3 = TCPU(comp->cpu);
	vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
	res4 = res3;
	tapSync(comp->tape,comp->tapCount);
	comp->tapCount = 0;
	bdiz = ((comp->dosen & 1) && (comp->bdi->fdc->type == FDC_93)) ? 1 : 0;
// request to external devices
	if (comp->hwFlag & HW_CONTIO) {
		if (!ideOut(comp->ide,port,val,comp->dosen & 1))
		if (gsOut(comp->gs,port,val) != GS_OK)
			comp->hw->out(comp,port,val,bdiz);
		zxIORW(comp,port);
	} else {
		zxIORW(comp,port);
		if (!ideOut(comp->ide,port,val,comp->dosen & 1))
		if (gsOut(comp->gs,port,val) != GS_OK)
			comp->hw->out(comp,port,val,bdiz);
	}
}
*/

Z80EX_BYTE intrq(CPUCONT void* ptr) {
	return ((ZXComp*)ptr)->intVector;
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

// 76543210
// -grb-GRB
const unsigned char defPalete[16] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77
};

const unsigned char tsPalInit[32] = {
	0x00,0x00,0x10,0x00,0x00,0x40,0x10,0x40,
	0x00,0x02,0x10,0x02,0x00,0x42,0x10,0x42,
	0x00,0x00,0x18,0x00,0x00,0x60,0x18,0x60,
	0x00,0x03,0x18,0x03,0x00,0x63,0x18,0x63
};

ZXComp* zxCreate() {
	int i;
	ZXComp* comp = (ZXComp*)malloc(sizeof(ZXComp));
	void* ptr = (void*)comp;
	memset(ptr,0,sizeof(ZXComp));
	comp->flag = ZX_PALCHAN;	// | ZX_JUSTBORN
	comp->hwFlag = 0;
	comp->padr = 0;

#ifdef SELFZ80
	comp->cpu = cpuCreate(&memrd,&memwr,&iord,*iowr,&intrq,ptr);
#else
	comp->cpu = z80ex_create(&memrd,ptr,&memwr,ptr,&iord,ptr,&iowr,ptr,&intrq,ptr);
#endif
	comp->mem = memCreate();
	comp->vid = vidCreate(comp->mem);
	zxSetFrq(comp,3.5);
// input
	comp->keyb = keyCreate();
	comp->joy = joyCreate();
	comp->mouse = mouseCreate();
// storage
	comp->tape = tapCreate();
	comp->bdi = bdiCreate();
	comp->ide = ideCreate(IDE_NONE);
	comp->ide->smuc.cmos = &comp->cmos;
	comp->sdc = sdcCreate();
// sound
	comp->ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
	comp->gs = gsCreate();
	comp->sdrv = sdrvCreate(SDRV_NONE);
// vaseconf
	comp->evo.evo2F = 0;
	comp->evo.evo4F = 0;
	comp->evo.evo6F = 0;
	comp->evo.evo8F = 0;
//			0   1   2   3   4   5   6   7   8   9   A   B   C    D    E    F
	char blnm[] = {'x','B','o','o','t',000,000,000,000,000,000,000,0x38,0x98,0x00,0x00};
	char bcnm[] = {'x','E','v','o',' ','0','.','5','2',000,000,000,0x89,0x99,0x00,0x00};
	memcpy(comp->evo.blVer,blnm,16);
	memcpy(comp->evo.bcVer,bcnm,16);
	comp->cmos.mode = 0;
//tsconf
	comp->tsconf.pwr_up = 1;
	comp->tsconf.vdos = 0;

	comp->rzxSize = 0;
	comp->rzxData = NULL;
	comp->tapCount = 0;
	comp->resbank = RES_48;
	comp->tickCount = 0;

	gsReset(comp->gs);
	for (i = 0; i < 16; i++) comp->colMap[i] = defPalete[i];
	comp->cmos.adr = 0;
	for (i = 0; i < 256; i++) comp->cmos.data[i] = 0x00;
	comp->cmos.data[17] = 0xaa;
	return comp;
}

void zxDestroy(ZXComp* comp) {
	KILLCPU(comp->cpu);	// z80ex_destroy(comp->cpu);
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
	int i;
	int resto = comp->resbank;
	for (i = 0; i < 16; i++) comp->colMap[i] = defPalete[i];	// reset palete to default
	comp->flag |= ZX_PALCHAN;
	comp->rzxPlay = 0;
	switch (wut) {
		case RES_48: resto = 1; break;
		case RES_128: resto = 0; break;
		case RES_DOS: resto = 3; break;
		case RES_SHADOW: resto = 2; break;
	}
	comp->prt2 = 0;
	comp->prt1 = 0;
	comp->prt0 = 0;
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
	memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
	comp->mem->flags = MEM_ROM_WP;
	rzxClear(comp);
	comp->rzxPlay = 0;
	RESETCPU(comp->cpu);	// z80ex_reset(comp->cpu);
	comp->vid->curscr = 5;
	vidSetMode(comp->vid,VID_NORMAL);
	comp->dosen = 0;
	comp->intVector = 0xff;
	switch (comp->hw->type) {
		case HW_ZX48:
			comp->prt0 = 0x10;		// else beta-disk doesn't enter in tr-dos
			if (resto & 2) comp->dosen = 1;
			break;
		case HW_ATM2:
			comp->dosen = 1;
			break;
		case HW_PENTEVO:
			comp->dosen = 1;
			comp->prt1 = 0x03;	// vid.mode 011
			break;
		case HW_TSLAB:
			comp->dosen = (resto & 2) ? 0 : 1;	// 0,1 = shadow,dos
			comp->prt0 = (resto & 1) ? 0x10 : 0x00;	// 2,3 = bas128,bas48
			comp->mem->flags = MEM_B0_WP;		// no MEM_ROM_WP, only MEM_B0_WP
			comp->vid->tsconf.scrPal = 0xf0;
			memset(comp->vid->tsconf.cram,0x00,0x1e0);
			memcpy(comp->vid->tsconf.cram + 0x1e0,tsPalInit,0x20);	// init zx palete ?
//			tslOut(comp,0x00af,0x00,0);			// std 256x192, NOGFX off
			comp->tsconf.Page0 = 0;
			comp->vid->nextbrd = 0xf7;
			comp->tsconf.p00af = 0;
			comp->tsconf.p01af = 0x05;
			comp->tsconf.p02af = 0;
			comp->tsconf.p03af = 0;
			comp->tsconf.p04af = 0;
			comp->tsconf.p05af = 0;
			comp->tsconf.p07af = 0x0f;
			comp->vid->tsconf.T0XOffset = 0;
			comp->vid->tsconf.T0YOffset = 0;
			comp->vid->tsconf.T1XOffset = 0;
			comp->vid->tsconf.T1YOffset = 0;
			comp->vid->tsconf.tconfig = 0;
			comp->vid->intpos.h = 0;
			comp->vid->intpos.v = 0;
			tslUpdatePorts(comp);
			break;
		default:
			comp->prt0 = ((resto & 1) << 4);
			if (resto & 2) comp->dosen = 1;
			break;
	}
	bdiReset(comp->bdi);
	if (comp->gs->flag & GS_RESET) gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
	comp->hw->mapMem(comp);
}

void zxSetLayout(ZXComp *comp, int fh, int fv, int bh, int bv, int sh, int sv, int ih, int iv, int is) {
	comp->vid->full.h = fh;
	comp->vid->full.v = fv;
	comp->vid->bord.h = bh;
	comp->vid->bord.v = bv;
	comp->vid->sync.h = sh;
	comp->vid->sync.v = sv;
	comp->vid->intpos.h = ih;
	comp->vid->intpos.v = iv;
	comp->vid->intsz = is;
	comp->vid->frmsz = fh * fv;
	vidUpdate(comp->vid);
	comp->nsPerFrame = 140 * comp->vid->frmsz;
}

void zxSetHardware(ZXComp* comp, const char* name) {
	HardWare* hw = findHardware(name);
	if (hw == NULL) return;
	comp->hw = hw;
	comp->vid->istsconf = (hw->type == HW_TSLAB) ? 1 : 0;
}

void zxSetFrq(ZXComp* comp, float frq) {
	comp->cpuFrq = frq;
	comp->nsPerTick = 280 * 3.5 / frq;	// 280 ns per tick @ 3.5 MHz
}

int zxExec(ZXComp* comp) {
	res4 = 0;
	EXECCPU(comp->cpu,res2);
	comp->nsCount += res2 * comp->nsPerTick;

	vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
	if (comp->padr) {
		tapSync(comp->tape,comp->tapCount);
		comp->tapCount = 0;
		bdiz = ((comp->dosen & 1) && (comp->bdi->fdc->type == FDC_93)) ? 1 : 0;
		if (!ideOut(comp->ide,comp->padr,comp->pval,comp->dosen & 1))
			if (gsOut(comp->gs,comp->padr,comp->pval) != GS_OK)
				comp->hw->out(comp,comp->padr,comp->pval,bdiz);
		comp->padr = 0;
	}
	res1 = res2;
	if (comp->rzxPlay) {
		comp->intStrobe = (comp->rzxFetches < 1);
		comp->frmStrobe = comp->intStrobe;
	} else {
		if (comp->nsCount >= comp->nsPerFrame) {
			comp->nsCount -= comp->nsPerFrame;
			comp->frmStrobe = 1;
		}
		if (comp->vid->intstrobe) {
			comp->intStrobe = 1;
			comp->vid->intstrobe = 0;
		}
	}
	pcreg = GETPC(comp->cpu);	// z80ex_get_reg(comp->cpu,regPC);
	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzxPlay) {
		res4 = 0;
		res2 = NMICPU(comp->cpu);	// z80ex_nmi(comp->cpu);
		res1 += res2;
		comp->nsCount += res2 * comp->nsPerTick;
		if (res2 != 0) {
			comp->dosen = 1;
			comp->prt0 |= 0x10;
			comp->hw->mapMem(comp);
			vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
		}
	}

	if (comp->intStrobe) {
		res4 = 0;
		res2 = INTCPU(comp->cpu);	// z80ex_int(comp->cpu);
		res1 += res2;
		comp->nsCount += res2 * comp->nsPerTick;
		vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
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
		comp->intStrobe = 0;
	}
	if ((comp->vid->nextrow) && (comp->hw->type == HW_TSLAB)) {
		tslUpdatePorts(comp);
		// vidTSRender(comp->vid,ptr);
		comp->vid->nextrow = 0;
	}
	nsTime = res1 * comp->nsPerTick;
	comp->tickCount += res1;

// TOO FAT
	pcreg = GETPC(comp->cpu); // z80ex_get_reg(comp->cpu,regPC);
	if (memGetCellFlags(comp->mem,pcreg) & MEM_BRK_FETCH) {
		comp->flag |= ZX_BREAK;
	}
	comp->tapCount += nsTime;
	if (comp->gs->flag & GS_ENABLE) comp->gs->sync += nsTime;
	if (comp->bdi->fdc->type != FDC_NONE) bdiSync(comp->bdi, nsTime);
	return nsTime;
}

// cmos

unsigned char cmsRd(ZXComp* comp) {
	unsigned char res = 0x00;
	switch (comp->cmos.adr) {
		case 0x0a: res = 0x00; break;
		case 0x0b: res = 0x02; break;
		case 0x0c: res = 0x00; break;
		case 0x0d: res = 0x80; break;
		default:
			if (comp->cmos.adr < 0xf0) {
				res = comp->cmos.data[comp->cmos.adr];
			} else {
				switch(comp->cmos.mode) {
					case 0: res = comp->evo.bcVer[comp->cmos.adr & 0x0f]; break;
					case 1: res = comp->evo.blVer[comp->cmos.adr & 0x0f]; break;
					case 2: res = keyReadCode(comp->keyb); break;		// read PC keyboard keycode
				}
			}
			break;
	}
	return res;
}

void cmsWr(ZXComp* comp, unsigned char val) {
	switch (comp->cmos.adr) {
		case 0x0c:
			if (val & 1) comp->keyb->kBufPos = 0;		// reset PC-keyboard buffer
			break;
		default:
			comp->cmos.data[comp->cmos.adr] = val;
			if (comp->cmos.adr > 0xef) {
				comp->cmos.mode = val;	// write to F0..FF : set F0..FF reading mode
				//printf("cmos mode %i\n",val);
			}
			break;
	}
}
