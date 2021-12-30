#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spectrum.h"
#include "filetypes/filetypes.h"

static int nsTime;
static int res2;
int res4 = 0;		// save last T synced with video (last is res2-res4

#ifndef RUNTIME_IO
#define RUNTIME_IO 1
#endif

// video callbacks

int vid_mrd_cb(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return comp->mem->ramData[adr & comp->mem->ramMask];
}

int memrd(int adr, int m1, void* ptr) {
	Computer* comp = (Computer*)ptr;
#ifdef HAVEZLIB
	if (m1 && comp->rzx.play) {
		comp->rzx.frm.fetches--;
	}
#endif
	unsigned char* fptr;
	if (comp->hw->id == HW_IBM_PC) {
		fptr = comp->brkRamMap + (adr & 0x3fffff);
	} else {
		fptr = getBrkPtr(comp, adr & 0xffff);
	}
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
	if (comp->hw->id != HW_IBM_PC)
		flag |= comp->brkAdrMap[adr & 0xffff];
	if (flag & MEM_BRK_RD)
		comp->brk = 1;
	return comp->hw->mrd(comp,adr,m1);
}

void memwr(int adr, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char* fptr;
	if (comp->hw->id == HW_IBM_PC) {
		fptr = comp->brkRamMap + (adr & 0x3fffff);
	} else {
		fptr = getBrkPtr(comp, adr & 0xffff);
	}
	unsigned char flag = *fptr;
	if (comp->maping) {
		if (!(flag & 0xf0)) {
			flag |= DBG_VIEW_BYTE;
			*fptr = flag;
		}
	}
	if (comp->hw->id != HW_IBM_PC)
		flag |= comp->brkAdrMap[adr & 0xffff];
	if (flag & MEM_BRK_WR)
		comp->brk = 1;
	comp->hw->mwr(comp,adr,val);
}

void zx_cont_delay(Computer* comp) {
	int wns = vid_wait(comp->vid);	// video is already at end of wait cycle
	int tns = 0;
	while (wns > 0) {
		comp->cpu->t++;
		wns -= comp->nsPerTick;
		tns += comp->nsPerTick;
	}
	vidSync(comp->vid, tns);
	res4 = comp->cpu->t;
}

void zx_free_ticks(Computer* comp, int t) {
	comp->cpu->t += t;
	vidSync(comp->vid, t * comp->nsPerTick);
	res4 = comp->cpu->t;
}

// Contention on T1
void zx_cont_t1(Computer* comp, int port) {
	if ((port & 0xc000) == 0x4000)
		zx_cont_delay(comp);
	zx_free_ticks(comp, 1);
}

// Contention on T2-T4
void zx_cont_tn(Computer* comp, int port) {
	if ((port & 0xc000) == 0x4000) {
		if (port & 1) {			// C:1 C:1 C:1 C:1
//			zx_cont_delay(comp);
//			zx_free_ticks(comp, 1);
			zx_cont_delay(comp);
			zx_free_ticks(comp, 1);
			zx_cont_delay(comp);
			zx_free_ticks(comp, 1);
			zx_cont_delay(comp);
			zx_free_ticks(comp, 1);
		} else {			// C:1 C:3
//			zx_cont_delay(comp);
//			zx_free_ticks(comp, 1);
			zx_cont_delay(comp);
			zx_free_ticks(comp, 3);
		}
	} else if (port & 1) {			// N:4
		zx_free_ticks(comp, 4-1);
	} else {				// N:1 C:3
//		zx_free_ticks(comp, 1);
		zx_cont_delay(comp);
		zx_free_ticks(comp, 3);
	}
}

static unsigned char ula_levs[8] = {0x00, 0x24, 0x49, 0x6d, 0x92, 0xb6, 0xdb, 0xff};

void zxSetUlaPalete(Computer* comp) {
	int i;
	int col;
	xColor xc;
	for (i = 0; i < 64; i++) {
		col = (comp->vid->ula->pal[i] << 1) & 7;	// blue
		if (col & 2) col |= 1;
		xc.b = ula_levs[col];
		col = (comp->vid->ula->pal[i] >> 2) & 7;	// red
		xc.r = ula_levs[col];
		col = (comp->vid->ula->pal[i] >> 5) & 7;	// green
		xc.g = ula_levs[col];
		comp->vid->pal[i] = xc;
	}
}

int iord(int port, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char res = 0xff;
// TODO: zx only
	if (comp->hw->grp == HWG_ZX) {
		if (comp->contIO && 0) {
			zx_cont_t1(comp, port);
			zx_cont_tn(comp, port);
		} else {
			vidSync(comp->vid,(comp->cpu->t + 3 - res4) * comp->nsPerTick);
			res4 = comp->cpu->t + 3;
		}
	}
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
	comp->bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
// brk
	if (comp->brkIOMap[port] & MEM_BRK_RD)
		comp->brk = 1;

	return comp->hw->in ? comp->hw->in(comp, port) : 0xff;
}

void iowr(int port, int val, void* ptr) {
	Computer* comp = (Computer*)ptr;
#if RUNTIME_IO
	comp->bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
	if (comp->hw->grp == HWG_ZX) {
		// sync video to current T
		vidSync(comp->vid, (comp->cpu->t - res4) * comp->nsPerTick);
		res4 = comp->cpu->t;
		if (comp->contIO) {
			zx_cont_t1(comp, port);
			comp->hw->out(comp, port, val);
			zx_cont_tn(comp, port);
			comp->cpu->t -= 4;
		} else {
			vidSync(comp->vid, comp->nsPerTick);
			res4++;
			comp->hw->out(comp, port, val);
		}
	} else {
		comp->hw->out(comp, port, val);
	}
	if (comp->vid->ula->palchan) {
		zxSetUlaPalete(comp);
		comp->vid->ula->palchan = 0;
	}
#else
// store adr & data, real writing will be later
	comp->padr = port;
	comp->pval = val;
#endif
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

//				0   1   2   3   4   5   6   7   8   9   A   B   C    D    E    F
const unsigned char blnm[] = {'x','B','o','o','t',000,000,000,000,000,000,000,0x38,0x98,0x00,0x00};
const unsigned char bcnm[] = {'x','E','v','o',' ',000,000,000,000,000,000,000,0x89,0x99,0x00,0x00};

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
	comp->cmos.kbuf = &comp->keyb->kbuf;
	comp->joy = joyCreate();
	comp->mouse = mouseCreate();
	comp->ppi = ppi_create();
	comp->ps2c = ps2c_create(comp->keyb, comp->mouse);
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
// ibm
	comp->dma8 = dma_create(comp, 0);
	comp->dma16 = dma_create(comp, 1);
// baseconf
	memcpy(comp->evo.blVer,blnm,16);
	memcpy(comp->evo.bcVer,bcnm,16);
//tsconf
	comp->tsconf.pwr_up = 1;
// rzx
#ifdef HAVEZLIB
	comp->rzx.file = NULL;
#endif
	comp->mpic.master = 1;
	comp->spic.master = 0;
	compSetHardware(comp, "Dummy");
	gsReset(comp->gs);
	comp->cmos.data[17] = 0xaa;	// 0a?
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
	ps2c_destroy(comp->ps2c);
	dma_destroy(comp->dma8);
	dma_destroy(comp->dma16);
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
	ps2c_reset(comp->ps2c);

	vidReset(comp->vid);
	comp->ext = 0;
	comp->prt2 = 0;
	comp->p1FFD = 0;
	comp->pEFF7 = 0;

	difReset(comp->dif);
	if (comp->gs->reset)
		gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
	saaReset(comp->saa);
	sdcReset(comp->sdc);
	dma_reset(comp->dma8);
	dma_reset(comp->dma16);
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
	mem_set_map_page(comp->mem, hw->pgsz);
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
#if RUNTIME_IO
	if (res2 > res4) {
		if (comp->hw->grp == HWG_ZX) {
			if (res2 > res4 + 1)
				vidSync(comp->vid, (res2 - res4 - 1) * comp->nsPerTick);
			comp->cpu->ack = comp->vid->intFRAME ? 1 : 0;
			vidSync(comp->vid, comp->nsPerTick);
		} else {
			vidSync(comp->vid, (res2 - res4) * comp->nsPerTick);
		}
	}
#else
	// out @ last tick
	vidSync(comp->vid,(res2 - res4 - 1) * comp->nsPerTick);			// 3T
	if (comp->padr) {
		comp->bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
		if (comp->hw->out)
			comp->hw->out(comp, comp->padr, comp->pval);
		comp->padr = 0;
		if (comp->vid->ula->palchan) {
			zxSetUlaPalete(comp);
			comp->vid->ula->palchan = 0;
		}
	}
	// TODO: zx - if INT is inactive at last tick, don't acknowledge it at the end of cycle
	comp->cpu->ack = comp->vid->intFRAME ? 1 : 0;
	vidSync(comp->vid, comp->nsPerTick);					// 1T
#endif
// execution completed : get eated time & translate signals
	nsTime = comp->vid->time;
	comp->tickCount += res2;
	// TODO: reset frmtCount @ INT strobe, but not @ end of opcode
	if (comp->vid->intFRAME) {
		if (!comp->intStrobe) {
			comp->frmtCount += comp->vid->intTime / comp->nsPerTick;		// ticks from command start to INT
			comp->intStrobe = 1;
			comp->fCount = comp->frmtCount;			// fix ticks @ frame
			if (!comp->halt) {
				comp->hCount = comp->fCount;		// if not HALT-ed
			}
			comp->frmtCount = (nsTime - comp->vid->intTime) / comp->nsPerTick;	// ticks from INT to command end
		} else {
			comp->frmtCount += res2;
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

unsigned char cmsRd(Computer* comp) {
	unsigned char res = 0xff;
	if (comp->cmos.adr >= 0xf0) {
		switch(comp->cmos.mode) {
			case 0: res = comp->evo.bcVer[comp->cmos.adr & 0x0f]; break;
			case 1: res = comp->evo.blVer[comp->cmos.adr & 0x0f]; break;
			case 2: res = keyReadCode(comp->keyb); break;		// read PC keyboard keycode
		}
	} else {
		res = cmos_rd(&comp->cmos, CMOS_DATA);
	}
	return res & 0xff;
}

void cmsWr(Computer* comp, int val) {
	switch (comp->cmos.adr) {
		case 0x0c:
			if (val & 1) comp->keyb->kbuf.pos = 0;		// reset PC-keyboard buffer
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
	xAdr xadr = mem_get_xadr(comp->mem, madr);
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
