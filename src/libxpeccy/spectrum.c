#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "spectrum.h"
#include "filetypes/filetypes.h"

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
	if (comp->contMem && (mptr->type == MEM_RAM) && (mptr->num & 1)) {	// pages 1,3,5,7 (48K model)
		res3 = TCPU(comp->cpu);					// until RD/WR cycle
		vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
		res4 = res3;
		vidWait(comp->vid);					// contended WAIT
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
	if (m1 && comp->rzxPlay) comp->rzx.fetches--;
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
	if (comp->contIO) {
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

	res3 = TCPU(comp->cpu) + 3;
	vidSync(comp->vid,(res3 - res4) * comp->nsPerTick);
	res4 = res3;
// tape sync
	tapSync(comp->tape,comp->tapCount);
	comp->tapCount = 0;
// play rzx
	if (comp->rzxPlay) {
		if (comp->rzx.pos < comp->rzx.data[comp->rzx.frame].frmSize) {
			res = comp->rzx.data[comp->rzx.frame].frmData[comp->rzx.pos];
			comp->rzx.pos++;
			return res;
		} else {
			printf("overIO\n");
			return 0xff;
		}
	}
	bdiz = (comp->dosen && (comp->dif->type == DIF_BDI)) ? 1 : 0;
// request to external devices
	if (ideIn(comp->ide, port, &res, bdiz)) return res;
	if (gsIn(comp->gs, port, &res)) return res;
	if (ulaIn(comp->vid->ula, port, &res)) return res;
	return comp->hw->in(comp, port, bdiz);
}

void iowr(CPUCONT Z80EX_WORD port, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	comp->padr = port;
	comp->pval = val;
}

Z80EX_BYTE intrq(CPUCONT void* ptr) {
	return ((ZXComp*)ptr)->intVector;
}

void rzxFree(ZXComp* zx) {
	if (!zx->rzx.data) return;
	for (int i = 0; i < zx->rzx.size; i++) {
		if (zx->rzx.data[i].frmData)
			free(zx->rzx.data[i].frmData);
	}
	free(zx->rzx.data);
	zx->rzx.data = 0;
	zx->rzx.size = 0;
}

void rzxStop(ZXComp* zx) {
	rzxFree(zx);
	zx->rzxPlay = 0;
	if (zx->rzx.file) fclose(zx->rzx.file);
	zx->rzx.file = NULL;
}

void zxInitPalete(ZXComp* comp) {
	for (int i = 0; i<16; i++) {
		comp->vid->pal[i].b = (i & 1) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
		comp->vid->pal[i].r = (i & 2) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
		comp->vid->pal[i].g = (i & 4) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
	}
}

void zxSetUlaPalete(ZXComp* comp) {
	for (int i = 0; i < 64; i++) {
		comp->vid->pal[i].b = (comp->vid->ula->pal[i] & 0x03) << 6;		// Bb0 : must me Bbb
		comp->vid->pal[i].r = (comp->vid->ula->pal[i] & 0x1c) << 3;
		comp->vid->pal[i].g = (comp->vid->ula->pal[i] & 0xe0);
	}
}

ZXComp* zxCreate() {
	ZXComp* comp = (ZXComp*)malloc(sizeof(ZXComp));
	void* ptr = (void*)comp;
	memset(ptr,0,sizeof(ZXComp));
	memset(comp->brkIOMap, 0, 0x10000);
	comp->resbank = RES_48;
	comp->firstRun = 1;

#ifdef SELFZ80
	comp->cpu = cpuCreate(&memrd,&memwr,&iord,&iowr,&intrq,ptr);
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
	comp->dif = difCreate(DIF_NONE);
	comp->ide = ideCreate(IDE_NONE);
	comp->ide->smuc.cmos = &comp->cmos;
	comp->sdc = sdcCreate();
// sound
	comp->ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
	comp->gs = gsCreate();
	comp->sdrv = sdrvCreate(SDRV_NONE);
	comp->saa = saaCreate();
// baseconf
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

	comp->rzx.size = 0;
	comp->rzx.data = NULL;
	comp->tapCount = 0;
	comp->tickCount = 0;

	gsReset(comp->gs);
	zxInitPalete(comp);
	comp->cmos.adr = 0;
	memset(comp->cmos.data, 0x00, 256);
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
	difDestroy(comp->dif);
	ideDestroy(comp->ide);
	tsDestroy(comp->ts);
	gsDestroy(comp->gs);
	sdrvDestroy(comp->sdrv);
	rzxStop(comp);
	free(comp);
}

void zxReset(ZXComp* comp,int res) {
	zxInitPalete(comp);
	comp->vid->ula->active = 0;
	comp->rzxPlay = 0;
	comp->prt2 = 0;
	comp->prt1 = 0;
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
	memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
	rzxStop(comp);
	comp->rzxPlay = 0;
	RESETCPU(comp->cpu);
	comp->vid->curscr = 5;
	vidSetMode(comp->vid,VID_NORMAL);
	comp->dosen = 0;
	comp->vid->intMask = 1;
	difReset(comp->dif);	//	bdiReset(comp->bdi);
	if (comp->gs->reset) gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
	saaReset(comp->saa);
	if (res == RES_DEFAULT) res = comp->resbank;
	comp->dosen = ((res == RES_DOS) || (res == RES_SHADOW)) ? 1 : 0;
	comp->prt0 = ((res == RES_DOS) || (res == RES_48)) ? 0x10 : 0x00;
	if (comp->hw->reset) comp->hw->reset(comp);
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
	comp->vid->intSize = is;
	comp->vid->frmsz = fh * fv;
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

int zxINT(ZXComp* comp, unsigned char vect) {
	res4 = 0;
	comp->intVector = vect;
	res2 = INTCPU(comp->cpu);
	res1 += res2;
	vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
	return res2;
}

int zxExec(ZXComp* comp) {

	comp->mem->flag = 0;

	res4 = 0;
	EXECCPU(comp->cpu,res2);
//	comp->nsCount += res2 * comp->nsPerTick;

	vidSync(comp->vid,(res2 - res4 - 1) * comp->nsPerTick);
	if (comp->padr) {
		tapSync(comp->tape,comp->tapCount);
		comp->tapCount = 0;
		bdiz = (comp->dosen && (comp->dif->type == DIF_BDI)) ? 1 : 0;
		ideOut(comp->ide, comp->padr, comp->pval, bdiz);
		gsOut(comp->gs, comp->padr, comp->pval);
		if (!bdiz) saaWrite(comp->saa, comp->padr, comp->pval);		// bdi ports must be closed!
		if (ulaOut(comp->vid->ula, comp->padr, comp->pval)) {
			if (comp->vid->ula->palchan) {
				zxSetUlaPalete(comp);
				comp->vid->ula->palchan = 0;
			}
		}
		comp->hw->out(comp, comp->padr, comp->pval, bdiz);
		comp->padr = 0;
	}
	vidSync(comp->vid, comp->nsPerTick);

	res1 = res2;

//	if (comp->rzxPlay) {
//		comp->intStrobe = (comp->rzxFetches < 1);
//		comp->frmStrobe = comp->intStrobe;
//	} else if (comp->vid->newFrame) {
//		comp->vid->newFrame = 0;
//		comp->frmStrobe = 1;
//	}
	pcreg = GETPC(comp->cpu);	// z80ex_get_reg(comp->cpu,regPC);
// NMI
	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzxPlay) {
		res4 = 0;
		res2 = NMICPU(comp->cpu);	// z80ex_nmi(comp->cpu);
		res1 += res2;
		if (res2 != 0) {
			comp->dosen = 1;
			comp->prt0 |= 0x10;
			comp->hw->mapMem(comp);
			vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
		}
	}
// INTs handle
	if (comp->rzxPlay) {
		if (comp->rzx.fetches < 1) {
			zxINT(comp, 0xff);
			comp->rzx.frame++;
			if (comp->rzx.frame >= comp->rzx.size) {
				rzxLoadFrame(comp);
			} else {
				comp->rzx.fetches = comp->rzx.data[comp->rzx.frame].fetches;
				comp->rzx.pos = 0;
			}
		}
	} else {
		if (comp->vid->intFRAME) {
			if (zxINT(comp,0xff)) comp->vid->intFRAME = 0;
		} else if (comp->vid->intLINE) {
			if (zxINT(comp,0xfd)) comp->vid->intLINE = 0;
		} else if (comp->vid->intDMA) {
			if (zxINT(comp,0xfb)) comp->vid->intDMA = 0;
		}
	}
	if (comp->vid->newFrame) {
		comp->vid->newFrame = 0;
		comp->frmStrobe = 1;
	}
// TSConf : update 'next-line' registers
	if (comp->vid->nextrow && comp->vid->istsconf) {
		tslUpdatePorts(comp);
		comp->vid->nextrow = 0;
	}
	nsTime = res1 * comp->nsPerTick;
	comp->tickCount += res1;
// breakpoints
	pcreg = GETPC(comp->cpu);
	unsigned char* ptr = memGetFptr(comp->mem, pcreg);
	if (*ptr & MEM_BRK_FETCH) {
		comp->brk = 1;
	} else if (*ptr & MEM_BRK_TFETCH) {
		comp->brk = 1;
		*ptr &= ~MEM_BRK_TFETCH;
	}
	if (comp->mem->flag & (MEM_BRK_RD | MEM_BRK_WR)) {
		comp->brk = 1;
	}
	if (comp->debug) comp->brk = 0;
// sync devices
	comp->tapCount += nsTime;
	if (comp->gs->enable) comp->gs->sync += nsTime;
	difSync(comp->dif, nsTime);
// return ns eated @ this step
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
