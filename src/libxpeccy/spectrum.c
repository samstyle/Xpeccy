#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spectrum.h"
#include "filetypes/filetypes.h"

static int nsTime;
static int res2;
int res3 = 0;	// tick in op, wich has last OUT/MWR (and vidSync)
int res4 = 0;	// save last res3 (vidSync on OUT/MWR process do res3-res4 ticks)

// ...

int vid_mrd_cb(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return comp->mem->ramData[adr & comp->mem->ramMask];
}

void zxMemRW(Computer* comp, int adr) {
	MemPage* mptr = &comp->mem->map[(adr >> 8) & 0xff];
	if (comp->contMem && (mptr->type == MEM_RAM) && (mptr->num & 0x40)) {	// 16K pages 1,3,5,7 (48K model)
		res3 = comp->cpu->t;						// until RD/WR cycle
		vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
		res4 = res3;
		vidWait(comp->vid);						// contended WAIT
		vidSync(comp->vid, comp->nsPerTick * 3);			// 3T rd/wr cycle
		res4 += 3;
	} else {
		res3 = comp->cpu->t + 3;
		vidSync(comp->vid, comp->nsPerTick * (res3 - res4));
		res4 = res3;
	}
}

int memrd(int adr, int m1, void* ptr) {
	Computer* comp = (Computer*)ptr;
#ifdef HAVEZLIB
	if (m1 && comp->rzx.play) {
		comp->rzx.frm.fetches--;
	}
#endif
	// zxMemRW(comp,adr);
	unsigned char* fptr = getBrkPtr(comp, adr & 0xffff);
	unsigned char flag = *fptr;
	if (comp->maping) {
		if ((comp->cpu->pc-1) == adr) {		// cuz pc is already incremented
			flag &= 0x0f;
			flag |= DBG_VIEW_EXEC;
			*fptr = flag;
		} else if (!(flag & 0xf0)) {
			flag |= DBG_VIEW_BYTE;
			*fptr = flag;
		}
	}
	if ((flag | comp->brkAdrMap[adr]) & MEM_BRK_RD) {
		comp->brk = 1;
	}
	return comp->hw->mrd(comp,adr,m1);
}

void memwr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	//zxMemRW(comp,adr);
	unsigned char* fptr = getBrkPtr(comp, adr & 0xffff);
	unsigned char flag = *fptr;
	if (comp->maping) {
		if (!(flag & 0xf0)) {
			flag |= DBG_VIEW_BYTE;
			*fptr = flag;
		}
	}
	if ((flag | comp->brkAdrMap[adr]) & MEM_BRK_WR) {
		comp->brk = 1;
	}
	comp->hw->mwr(comp,adr,val);
}

static int bdiz;

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

int iord(int port, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char res = 0xff;
// TODO: zx only
	res3 = comp->cpu->t + 3;
	vidSync(comp->vid,(res3 - res4) * comp->nsPerTick);
	res4 = res3;
// play rzx
#ifdef HAVEZLIB
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
#endif
	bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
// brk
	if (comp->brkIOMap[port] & MEM_BRK_RD)
		comp->brk = 1;

	return comp->hw->in ? comp->hw->in(comp, port, bdiz) : 0xff;
}

void iowr(int port, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	comp->padr = port;
	comp->pval = val;
// brk
	if (comp->brkIOMap[port] & MEM_BRK_WR)
		comp->brk = 1;
}

int intrq(void* ptr) {
	return ((Computer*)ptr)->intVector & 0xff;
}

// new (for future use)

int comp_rom_rd(Computer* comp, int adr) {
	adr &= comp->mem->romMask;
	if (comp->brkRomMap[adr] & MEM_BRK_RD)
		comp->brk = 1;
	return comp->mem->romData[adr] & 0xff;
}

void comp_rom_wr(Computer* comp, int adr, int val) {
	adr &= comp->mem->romMask;
	if (comp->brkRomMap[adr] & MEM_BRK_WR)
		comp->brk = 1;
	comp->mem->romData[adr] = val & 0xff;
}

int comp_ram_rd(Computer* comp, int adr) {
	adr &= comp->mem->ramMask;
	if (comp->brkRamMap[adr] & MEM_BRK_RD)
		comp->brk = 1;
	return comp->mem->ramData[adr] & 0xff;
}

void comp_ram_wr(Computer* comp, int adr, int val) {
	adr &= comp->mem->ramMask;
	if (comp->brkRamMap[adr] & MEM_BRK_WR)
		comp->brk = 1;
	comp->mem->ramData[adr] = val & 0xff;
}

int comp_slt_rd(Computer* comp, int adr) {
	if (!comp->slot->data) return 0xff;
	adr &= comp->slot->memMask;
	if (comp->slot->brkMap[adr] & MEM_BRK_RD)
		comp->brk = 1;
	return comp->slot->data[adr] & 0xff;
}

void comp_slt_wr(Computer* comp, int adr, int val) {
	if (!comp->slot->data) return;
	adr &= comp->slot->memMask;
	if (comp->slot->brkMap[adr] & MEM_BRK_WR)
		comp->brk = 1;
	comp->slot->data[adr] = val & 0xff;
}

// rzx

void rzxStop(Computer* zx) {
#ifdef HAVEZLIB
	zx->rzx.play = 0;
	if (zx->rzx.file) fclose(zx->rzx.file);
	zx->rzx.file = NULL;
	zx->rzx.fCount = 0;
	zx->rzx.frm.size = 0;
#endif
}

void zxSetUlaPalete(Computer* comp) {
	for (int i = 0; i < 64; i++) {
		comp->vid->pal[i].b = (comp->vid->ula->pal[i] << 6) & 0xe0;		// Bbb
		if (comp->vid->pal[i].b & 0x40)
			comp->vid->pal[i].b |= 0x20;
		comp->vid->pal[i].r = (comp->vid->ula->pal[i] << 3) & 0xe0;
		comp->vid->pal[i].g = (comp->vid->ula->pal[i] & 0xe0);
	}
}

Computer* compCreate() {
	Computer* comp = (Computer*)malloc(sizeof(Computer));
	memset(comp, 0x00, sizeof(Computer));
	comp->resbank = RES_48;
	comp->firstRun = 1;

	comp->cpu = cpuCreate(CPU_Z80,memrd,memwr,iord,iowr,intrq,comp);
	comp->mem = memCreate();
	comp->vid = vidCreate(vid_mrd_cb, comp);
	vidSetMode(comp->vid, VID_NORMAL);

// input
	comp->keyb = keyCreate();
	comp->joy = joyCreate();
	comp->mouse = mouseCreate();
	comp->ppi = ppi_create();
// storage
	comp->tape = tapCreate();
	comp->dif = difCreate(DIF_NONE);
	comp->ide = ideCreate(IDE_NONE);
	comp->ide->smuc.cmos = &comp->cmos;
	comp->sdc = sdcCreate();
	comp->slot = sltCreate();
// sound
	comp->ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
	comp->gs = gsCreate();
	comp->sdrv = sdrvCreate(SDRV_NONE);
	comp->gbsnd = gbsCreate();
	comp->saa = saaCreate();
	comp->beep = bcCreate();
	comp->nesapu = apuCreate(nes_apu_ext_rd, comp);
// baseconf
//			0   1   2   3   4   5   6   7   8   9   A   B   C    D    E    F
	unsigned char blnm[] = {'x','B','o','o','t',000,000,000,000,000,000,000,0x38,0x98,0x00,0x00};
	unsigned char bcnm[] = {'x','E','v','o',' ','0','.','5','2',000,000,000,0x89,0x99,0x00,0x00};
	memcpy(comp->evo.blVer,blnm,16);
	memcpy(comp->evo.bcVer,bcnm,16);
//tsconf
	comp->tsconf.pwr_up = 1;
// rzx
#ifdef HAVEZLIB
	comp->rzx.file = NULL;
#endif
	compSetHardware(comp, "Dummy");
	gsReset(comp->gs);
	comp->cmos.data[17] = 0xaa;
	comp->frqMul = 1;
	compSetBaseFrq(comp, 3.5);
	return comp;
}

void compDestroy(Computer* comp) {
	rzxStop(comp);
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
	gbsDestroy(comp->gbsnd);
	sdrvDestroy(comp->sdrv);
	saaDestroy(comp->saa);
	bcDestroy(comp->beep);
	apuDestroy(comp->nesapu);
	sltDestroy(comp->slot);
	ppi_destroy(comp->ppi);
	free(comp);
}

void compReset(Computer* comp,int res) {
#ifdef HAVEZLIB
	if (comp->rzx.play)
		rzxStop(comp);
#endif

	if (res == RES_DEFAULT)
		res = comp->resbank;
	comp->p7FFD = ((res == RES_DOS) || (res == RES_48)) ? 0x10 : 0x00;
	comp->dos = ((res == RES_DOS) || (res == RES_SHADOW)) ? 1 : 0;
	comp->rom = (comp->p7FFD & 0x10) ? 1 : 0;
	comp->cpm = 0;

	// kbdReleaseAll(comp->keyb);
	kbdSetMode(comp->keyb, KBD_SPECTRUM);

	vidReset(comp->vid);
	comp->ext = 0;
	comp->prt2 = 0;
	comp->p1FFD = 0;
	comp->pEFF7 = 0;

	difReset(comp->dif);	//	bdiReset(comp->bdi);
	if (comp->gs->reset)
		gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
	saaReset(comp->saa);
	if (comp->hw->reset)
		comp->hw->reset(comp);
	comp->hw->mapMem(comp);
	comp->cpu->reset(comp->cpu);
}

// cpu freq

//void compUpdateTimings(Computer* comp) {
//	int perNoTurbo = 1e3 / comp->cpuFrq;		// ns for full cpu tick
//	if (perNoTurbo & 1) perNoTurbo++;
//	int type = comp->hw ? comp->hw->id : HW_NULL;
//	switch (type) {
//		case HW_MSX:
//		case HW_MSX2:
//			comp->fps = 60;
//			vidUpdateTimings(comp->vid, perNoTurbo * 2 / 3);
//			break;
//		case HW_GBC:
//			comp->fps = 50;
//			comp->gbsnd->wav.period = perNoTurbo << 5;			// 128KHz period for wave generator = cpu.frq / 32
//			comp->gb.timer.div.per = (perNoTurbo / comp->frqMul) * 256;	// 16KHz timer divider tick. this timer depends on turbo speed
//			vidUpdateTimings(comp->vid, perNoTurbo << 1);
//			break;
//		case HW_C64:
//			vidUpdateTimings(comp->vid, perNoTurbo >> 3);
//			break;
//		case HW_BK0010:
//		case HW_BK0011M:
//			vidUpdateTimings(comp->vid, 302);
//			comp->vid->lockLayout = 0;
//			vidSetLayout(comp->vid, bkLay);
//			comp->vid->lockLayout = 1;
//			break;
//		case HW_NES:
			// base frq (21.477MHz | 26.602MHz)
			// cpu frq (base:12 | base:16)
			// ppu frq (base:4 | base:5)
			// apu frame (60Hz | 50Hz)
			// apu frq = cpu / 2
			// smallest wave period = cpu / 16
/*
			switch(comp->nes.type) {
				case NES_PAL:
					comp->fps = 50;
					perNoTurbo = 1e3 / 1.66;		// ~601
					comp_set_layout(comp, &nesPALLay);
					comp->vid->vbsline = 241;
					comp->vid->vbrline = 311;
					vidUpdateTimings(comp->vid, perNoTurbo / 3.2);		// 16 ticks = 5 dots
					comp->nesapu->wdiv = 3107;		// 5/6 = 3107? or 166/179 = 3458
					break;
				case NES_NTSC:
					comp->fps = 60;
					perNoTurbo = 1e3 / 1.79;		// ~559
					comp_set_layout(comp, &nesNTSCLay);
					comp->vid->vbsline = 241;
					comp->vid->vbrline = 261;
					vidUpdateTimings(comp->vid, perNoTurbo / 3);		// 15 ticks = 5 dots
					comp->nesapu->wdiv = 3729;
					break;
				default:							// dendy
					comp->fps = 59;
					perNoTurbo = 1e3 / 1.77;
					comp_set_layout(comp, &nesPALLay);
					comp->vid->vbsline = 291;
					comp->vid->vbrline = 311;
					vidUpdateTimings(comp->vid, perNoTurbo / 3);
					comp->nesapu->wdiv = 3729;
					break;
			}
			comp->nesapu->wper = perNoTurbo << 1;				// 1 APU tick = 2 CPU ticks
			break;
*/
//		case HW_SPCLST:
//			comp->vid->lockLayout = 0;
//			vidSetLayout(comp->vid, spclstLay);
//			comp->vid->lockLayout = 1;
//			vidSetMode(comp->vid, VID_SPCLST);
//			vidUpdateTimings(comp->vid, perNoTurbo >> 2);			// CPU:2MHz, dots:8MHz
//			break;
//		default:
//			comp->fps = 50;
//			vidUpdateTimings(comp->vid, perNoTurbo >> 1);
//			break;
//	}
//	comp->nsPerTick = perNoTurbo / comp->frqMul;
//#ifdef ISDEBUG
	// printf("%f x %i : %i ns\n",comp->cpuFrq,comp->frqMul,comp->nsPerTick);
//#endif
//}

void comp_update_timings(Computer* comp) {
	comp->nsPerTick = 1e3 / comp->cpuFrq;
	if (comp->hw->init)
		comp->hw->init(comp);
	comp->nsPerTick /= comp->frqMul;
}

void compSetBaseFrq(Computer* comp, double frq) {
	if (frq > 0)
		comp->cpuFrq = frq;
	comp_update_timings(comp);
}

void compSetTurbo(Computer* comp, double mult) {
	comp->frqMul = mult;
	comp->cpu->speed = (mult > 1) ? 1 : 0;
	comp_update_timings(comp);
}

void comp_set_layout(Computer* comp, vLayout* lay) {
	if (comp->hw->lay)
		lay = comp->hw->lay;
	vidSetLayout(comp->vid, lay);
}

// hardware

int compSetHardware(Computer* comp, const char* name) {
	HardWare* hw;
	if (name == NULL) {
		hw = comp->hw;
	} else {
		hw = findHardware(name);
	}
	if (hw == NULL) return 0;
	comp->hw = hw;
	comp->cpu->nod = 0;
	comp->vid->mrd = vid_mrd_cb;
	// compUpdateTimings(comp);
	compSetBaseFrq(comp, 0);	// recalculations
	return 1;
}

// exec 1 opcode, sync devices, return eated ns

int compExec(Computer* comp) {
	comp->vid->time = 0;
// breakpoints
	if (!comp->debug) {
		unsigned char brk = getBrk(comp, comp->cpu->pc);
		if (brk & (MEM_BRK_FETCH | MEM_BRK_TFETCH)) {
			comp->brk = 1;
			if (brk & MEM_BRK_TFETCH) {
				unsigned char *ptr = getBrkPtr(comp, comp->cpu->pc);
				*ptr &= ~MEM_BRK_TFETCH;
			}
			return 0;
		}
		if (comp->cpu->intrq && comp->brkirq) {
			comp->brk = 1;
			return 0;
		}
	}
// start
	res4 = 0;
// exec cpu opcode OR handle interrupt. get T states back
	res2 = comp->cpu->exec(comp->cpu);
// scorpion WAIT: add 1T to odd-T command
	if (comp->evenM1 && (res2 & 1))
		res2++;
// out @ last tick
	vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
	if (comp->padr) {
//		tapSync(comp->tape,comp->tapCount);
//		comp->tapCount = 0;
		bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
		if (ulaOut(comp->vid->ula, comp->padr, comp->pval)) {
			if (comp->vid->ula->palchan) {
				zxSetUlaPalete(comp);
				comp->vid->ula->palchan = 0;
			}
		}
		if (comp->hw->out)
			comp->hw->out(comp, comp->padr, comp->pval, bdiz);
		comp->padr = 0;
	}
// execution completed : get eated time & translate signals
	nsTime = comp->vid->time;
	comp->tickCount += res2;
	if (comp->vid->intFRAME) {
		if (!comp->intStrobe) {
			comp->intStrobe = 1;
			comp->fCount = comp->frmtCount;
			if (!comp->halt) {
				comp->hCount = comp->fCount;
			}
			comp->frmtCount = 0;
		}
	} else {
		comp->frmtCount += res2;
		if (comp->intStrobe)
			comp->intStrobe = 0;
	}
	if (comp->cpu->halt && !comp->halt) {
		comp->halt = 1;
		comp->hCount = comp->frmtCount;
	} else if (!comp->cpu->halt && comp->halt) {
		comp->halt = 0;
	}
// sync hardware
#ifdef HAVEZLIB
	if (comp->rzx.play) {
		if (comp->rzx.frm.fetches < 1) {
			comp->intVector = 0xff;
			comp->cpu->intrq |= Z80_INT;
			comp->rzx.fCurrent++;
			comp->rzx.fCount--;
			rzxGetFrame(comp);
		}
	} else if (comp->hw->sync) {
		comp->hw->sync(comp, nsTime);
	}
#else
	if (comp->hw->sync)
		comp->hw->sync(comp, nsTime);
#endif
// new frame
	if (comp->vid->newFrame) {
		comp->vid->newFrame = 0;
		comp->frmStrobe = 1;
	}
// return ns eated @ this step
	return nsTime;
}

// cmos

int toBCD(int val) {
	int rrt = val % 10;
	rrt |= ((val/10) << 4);
	return rrt;
}

#include <time.h>

unsigned char cmsRd(Computer* comp) {
	int res = 0x00;

	time_t rtime;
	time(&rtime);
	struct tm* ctime = localtime(&rtime);
	switch (comp->cmos.adr) {
		case 0x00: res = toBCD(ctime->tm_sec); break;
		case 0x02: res = toBCD(ctime->tm_min); break;
		case 0x04: res = toBCD(ctime->tm_hour); break;
		case 0x06: res = toBCD(ctime->tm_wday); break;
		case 0x07: res = toBCD(ctime->tm_mday); break;
		case 0x08: res = toBCD(ctime->tm_mon); break;
		case 0x09: res = toBCD(ctime->tm_year % 100); break;
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
	return res & 0xff;
}

void cmsWr(Computer* comp, int val) {
	switch (comp->cmos.adr) {
		case 0x0c:
			if (val & 1) comp->keyb->kBufPos = 0;		// reset PC-keyboard buffer
			break;
		default:
			comp->cmos.data[comp->cmos.adr] = val & 0xff;
			if (comp->cmos.adr > 0xef) {
				comp->cmos.mode = val;	// write to F0..FF : set F0..FF reading mode
				//printf("cmos mode %i\n",val);
			}
			break;
	}
}

// breaks

static unsigned char dumBrk = 0x00;

unsigned char* getBrkPtr(Computer* comp, unsigned short madr) {
	xAdr xadr = memGetXAdr(comp->mem, madr);
	unsigned char* ptr = NULL;
	switch (xadr.type) {
		case MEM_RAM: ptr = comp->brkRamMap + (xadr.abs & 0x3fffff); break;
		case MEM_ROM: ptr = comp->brkRomMap + (xadr.abs & 0x7ffff); break;
		case MEM_SLOT:
			if (comp->slot->brkMap)
				ptr = comp->slot->brkMap + (xadr.abs & comp->slot->memMask);
			break;
	}
	if (!ptr) {
		dumBrk = 0;
		ptr = &dumBrk;
	}
	return ptr;
}

void setBrk(Computer* comp, unsigned short adr, unsigned char val) {
	unsigned char* ptr = getBrkPtr(comp, adr);
	if (ptr == NULL) return;
	*ptr = (*ptr & 0xf0) | (val & 0x0f);
}

unsigned char getBrk(Computer* comp, unsigned short adr) {
	unsigned char* ptr = getBrkPtr(comp, adr);
	unsigned char res = ptr ? *ptr : 0x00;
	res |= (comp->brkAdrMap[adr] & 0x0f);
	return res;
}
