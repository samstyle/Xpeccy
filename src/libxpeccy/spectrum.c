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
unsigned short pcreg;

MemPage* mptr;

void zxMemRW(Computer* comp, int adr) {
	mptr = memGetBankPtr(comp->mem,adr);
	if (comp->contMem && (mptr->type == MEM_RAM) && (mptr->num & 1)) {	// pages 1,3,5,7 (48K model)
		res3 = comp->cpu->t;					// until RD/WR cycle
		vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
		res4 = res3;
		vidWait(comp->vid);					// contended WAIT
		vidSync(comp->vid, comp->nsPerTick * 3);		// 3T rd/wr cycle
		res4 += 3;
	} else {
		res3 = comp->cpu->t + 3;
		vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
		res4 = res3;
	}
}

unsigned char memrd(unsigned short adr,int m1,void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (m1 && comp->rzx.play) {
		comp->rzx.frm.fetches--;
	}
	zxMemRW(comp,adr);
	if (getBrk(comp, adr) & MEM_BRK_RD) {
		comp->brk = 1;
	}
	return comp->hw->mrd(comp,adr,m1);
}

void memwr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	zxMemRW(comp,adr);
	if (getBrk(comp, adr) & MEM_BRK_WR) {
		comp->brk = 1;
	}
	comp->hw->mwr(comp,adr,val);
}

int bdiz;

inline void zxIORW(Computer* comp, int port) {
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

unsigned char iord(unsigned short port, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char res = 0xff;

	res3 = comp->cpu->t + 3;
	vidSync(comp->vid,(res3 - res4) * comp->nsPerTick);
	res4 = res3;
// tape sync
	tapSync(comp->tape,comp->tapCount);
	comp->tapCount = 0;
// play rzx
	if (comp->rzx.play) {
		if (comp->rzx.frm.pos < comp->rzx.frm.size) {
			res = comp->rzx.frm.data[comp->rzx.frm.pos];
			comp->rzx.frm.pos++;
			return res;
		} else {
			rzxStop(comp);
			printf("overIO\n");
			comp->rzx.overio = 1;
			return 0xff;
		}
	}
	bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
// brk
	if (comp->brkIOMap[port] & IO_BRK_RD)
		comp->brk = 1;
// request to external devices
	if (ideIn(comp->ide, port, &res, bdiz)) return res;
	if (gsIn(comp->gs, port, &res)) return res;
	if (ulaIn(comp->vid->ula, port, &res)) return res;
	return comp->hw->in(comp, port, bdiz);
}

void iowr(unsigned short port, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->padr = port;
	comp->pval = val;
// brk
	if (comp->brkIOMap[port] & IO_BRK_WR)
		comp->brk = 1;
}

unsigned char intrq(void* ptr) {
	return ((Computer*)ptr)->intVector;
}

void rzxStop(Computer* zx) {
	zx->rzx.play = 0;
	if (zx->rzx.file) fclose(zx->rzx.file);
	zx->rzx.file = NULL;
	zx->rzx.fCount = 0;
	zx->rzx.frm.size = 0;
}

void zxInitPalete(Computer* comp) {
	for (int i = 0; i<16; i++) {
		comp->vid->pal[i].b = (i & 1) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
		comp->vid->pal[i].r = (i & 2) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
		comp->vid->pal[i].g = (i & 4) ? ((i & 8) ? 0xff : 0xa0) : 0x00;
	}
}

void zxSetUlaPalete(Computer* comp) {
	for (int i = 0; i < 64; i++) {
		comp->vid->pal[i].b = (comp->vid->ula->pal[i] & 0x03) << 6;		// Bb0 : must me Bbb
		comp->vid->pal[i].r = (comp->vid->ula->pal[i] & 0x1c) << 3;
		comp->vid->pal[i].g = (comp->vid->ula->pal[i] & 0xe0);
	}
}

Computer* compCreate() {
	Computer* comp = (Computer*)malloc(sizeof(Computer));
	void* ptr = (void*)comp;
	memset(ptr,0,sizeof(Computer));
	memset(comp->brkIOMap, 0, 0x10000);
	comp->resbank = RES_48;
	comp->firstRun = 1;

	comp->cpu = cpuCreate(&memrd,&memwr,&iord,&iowr,&intrq,ptr);
	comp->mem = memCreate();
	comp->vid = vidCreate(comp->mem);
	compSetFrq(comp,3.5);

	memset(comp->brkIOMap, 0, 0x10000);
	memset(comp->brkRomMap, 0, 0x80000);
	memset(comp->brkRamMap, 0, 0x400000);

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
	comp->msx.slotA.data = NULL;
	comp->msx.slotB.data = NULL;
	comp->msx.slotA.name[0] = 0x00;
	comp->msx.slotB.name[0] = 0x00;
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
//					0   1   2   3   4   5   6   7   8   9   A   B   C    D    E    F
	char blnm[] = {'x','B','o','o','t',000,000,000,000,000,000,000,0x38,0x98,0x00,0x00};
	char bcnm[] = {'x','E','v','o',' ','0','.','5','2',000,000,000,0x89,0x99,0x00,0x00};
	memcpy(comp->evo.blVer,blnm,16);
	memcpy(comp->evo.bcVer,bcnm,16);
	comp->cmos.mode = 0;
//tsconf
	comp->tsconf.pwr_up = 1;
	comp->tsconf.vdos = 0;
// rzx
	comp->rzx.start = 0;
	comp->rzx.play = 0;
	comp->rzx.overio = 0;
	comp->rzx.fCount = 0;
	comp->rzx.file = NULL;

	comp->tapCount = 0;
	comp->tickCount = 0;

	gsReset(comp->gs);
	zxInitPalete(comp);
	comp->cmos.adr = 0;
	memset(comp->cmos.data, 0x00, 256);
	comp->cmos.data[17] = 0xaa;
	return comp;
}

void compDestroy(Computer* comp) {
	cpuDestroy(comp->cpu);
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

void compReset(Computer* comp,int res) {
	if (comp->rzx.play) rzxStop(comp);
	zxInitPalete(comp);
	comp->vid->ula->active = 0;
	comp->rzx.play = 0;
	comp->prt2 = 0;
	comp->p1FFD = 0;
	comp->pEFF7 = 0;
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,5);
	memSetBank(comp->mem,MEM_BANK2,MEM_RAM,2);
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,0);
	cpuReset(comp->cpu);
	comp->vid->curscr = 5;
	vidSetMode(comp->vid,VID_NORMAL);
	comp->dos = 0;
	comp->vid->intMask = 1;
	difReset(comp->dif);	//	bdiReset(comp->bdi);
	if (comp->gs->reset) gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
	saaReset(comp->saa);
	if (res == RES_DEFAULT) res = comp->resbank;
	comp->p7FFD = ((res == RES_DOS) || (res == RES_48)) ? 0x10 : 0x00;
	comp->dos = ((res == RES_DOS) || (res == RES_SHADOW)) ? 1 : 0;
	comp->rom = (comp->p7FFD & 0x10) ? 1 : 0;
	comp->cpm = 0;
	comp->hw->mapMem(comp);
	if (comp->hw->reset) comp->hw->reset(comp);
}

void compSetLayout(Computer *comp, int fh, int fv, int bh, int bv, int sh, int sv, int ih, int iv, int is) {
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

void compSetHardware(Computer* comp, const char* name) {
	HardWare* hw = findHardware(name);
	if (hw == NULL) return;
	comp->hw = hw;
	comp->vid->istsconf = (hw->type == HW_TSLAB) ? 1 : 0;
	comp->vid->ismsx = (hw->type == HW_MSX) ? 1 : 0;
}

void compSetFrq(Computer* comp, double frq) {
	comp->cpuFrq = frq;
	comp->nsPerTick = 280 * 3.5 / frq;	// 280 ns per tick @ 3.5 MHz
}

int zxINT(Computer* comp, unsigned char vect) {
	res4 = 0;
	comp->intVector = vect;
	res2 = cpuINT(comp->cpu);
	res1 += res2;
	vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
	return res2;
}

int compExec(Computer* comp) {
	res4 = 0;
	res2 = cpuExec(comp->cpu);
// scorpion WAIT: add 1T to odd-T command
	if (comp->scrpWait && (res2 & 1))
		res2++;
// out @ last tick
	vidSync(comp->vid,(res2 - res4 - 1) * comp->nsPerTick);
	if (comp->padr) {
		tapSync(comp->tape,comp->tapCount);
		comp->tapCount = 0;
		bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
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
// ...
	res1 = res2;
	pcreg = comp->cpu->pc;
// NMI
	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzx.play) {
		res4 = 0;
		res2 = cpuNMI(comp->cpu);	// z80ex_nmi(comp->cpu);
		res1 += res2;
		if (res2 != 0) {
			comp->dos = 1;
			comp->p7FFD |= 0x10;
			comp->hw->mapMem(comp);
			vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
		}
	}
// INTs handle
	if (comp->rzx.play) {
		if (comp->rzx.frm.fetches < 1) {
			zxINT(comp, 0xff);
			comp->rzx.fCurrent++;
			comp->rzx.fCount--;
			rzxGetFrame(comp);
		}
	} else if (comp->vid->ismsx) {
		if ((comp->vid->v9918.reg[1] & 0x40) && comp->vid->newFrame)
			zxINT(comp, 0xff);
	} else {
		if (comp->vid->intFRAME) {
			zxINT(comp, 0xff);
		} else if (comp->vid->intLINE) {
			if (zxINT(comp,0xfd)) comp->vid->intLINE = 0;
		} else if (comp->vid->intDMA) {
			if (zxINT(comp,0xfb)) comp->vid->intDMA = 0;
		}
	}
// new frame
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
	pcreg = comp->cpu->pc;
	unsigned char* ptr = getBrkPtr(comp, pcreg);
	if (*ptr & (MEM_BRK_FETCH | MEM_BRK_TFETCH)) {
		comp->brk = 1;
		*ptr &= ~MEM_BRK_TFETCH;
	}
	if (comp->debug)
		comp->brk = 0;
// sync devices
	comp->tapCount += nsTime;
	if (comp->gs->enable) comp->gs->sync += nsTime;
	difSync(comp->dif, nsTime);
// return ns eated @ this step
	return nsTime;
}

// cmos

unsigned char cmsRd(Computer* comp) {
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

void cmsWr(Computer* comp, unsigned char val) {
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

// breaks

unsigned char* getBrkPtr(Computer* comp, unsigned short madr) {
	MemPage* ptr = &comp->mem->map[madr >> 14];
	int adr = (ptr->num << 14) | (madr & 0x3fff);
	return (ptr->type == MEM_RAM) ? &comp->brkRamMap[adr] : &comp->brkRomMap[adr];
}

unsigned char getBrk(Computer* comp, unsigned short madr) {
	unsigned char res = 0x00;
	MemPage* ptr = &comp->mem->map[madr >> 14];
	int adr = (ptr->num << 14) | (madr & 0x3fff);
	if (ptr->type == MEM_RAM) {
		res = comp->brkRamMap[adr];
	} else if (ptr->type == MEM_ROM) {
		res = comp->brkRomMap[adr];
	}
	res |= comp->brkAdrMap[madr];
	return res;
}
