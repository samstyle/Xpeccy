#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spectrum.h"
#include "filetypes/filetypes.h"

int nsTime;
// int res1 = 0;
int res2 = 0;
int res3 = 0;	// tick in op, wich has last OUT/MWR (and vidSync)
int res4 = 0;	// save last res3 (vidSync on OUT/MWR process do res3-res4 ticks)
unsigned short pcreg;

MemPage* mptr;

// external devices

xDevice* compFindDev(Computer* comp, int type) {
	xDevice* res = NULL;
	int idx = 0;
	xDevice* dev;
	while((idx < MAX_DEV_COUNT) && comp->devList[idx]) {
		dev = comp->devList[idx];
		if (dev->type == type) {
			res = dev;
			idx = MAX_DEV_COUNT;
		}
		idx++;
	}
	return res;
}

void compAddDev(Computer* comp, int type) {
	if (compFindDev(comp, type)) return;		// already added
	int idx = 0;
	while(idx < MAX_DEV_COUNT) {
		if (comp->devList[idx] == NULL) {
			comp->devList[idx] = devCreate(type);
			idx = MAX_DEV_COUNT;
		}
		idx++;
	}
}

void compDelDev(Computer* comp, int type) {
	int idx = 0;
	xDevice* dev;
	while(idx < MAX_DEV_COUNT) {
		dev = comp->devList[idx];
		if (dev) {
			if (dev->type == type) {
				devDestroy(dev);
				while(idx < MAX_DEV_COUNT - 1) {
					comp->devList[idx] = comp->devList[idx + 1];
					idx++;
				}
				comp->devList[MAX_DEV_COUNT - 1] = NULL;
			}
		}
		idx++;
	}
}

void compDelAllDev(Computer* comp) {
	int idx = 0;
	xDevice* dev;
	while((idx < MAX_DEV_COUNT) && comp->devList[idx]) {
		dev = comp->devList[idx];
		devDestroy(dev);
		comp->devList[idx] = NULL;
		idx++;
	}
}

void compDevReset(Computer* comp) {
	int idx = 0;
	xDevice* dev;
	while ((idx < MAX_DEV_COUNT) && comp->devList[idx]) {
		dev = comp->devList[idx];
		if (dev->reset)
			dev->reset(dev->ptr.ptr);
		idx++;
	}
}

void compDevRequest(Computer* comp) {
	int idx = 0;
	xDevice* dev;
	comp->bus.busy = 0;
	comp->bus.dos = comp->dos;
	while ((idx < MAX_DEV_COUNT) && !comp->bus.busy && comp->devList[idx]) {
		dev = comp->devList[idx];
		if (dev->req)
			dev->req(dev->ptr.ptr, &comp->bus);
		idx++;
	}
}

void compDevSync(Computer* comp, int ns) {
	int idx = 0;
	xDevice* dev;
	while((idx < MAX_DEV_COUNT) && comp->devList[idx]) {
		dev = comp->devList[idx];
		if (dev->sync)
			dev->sync(dev->ptr.ptr, ns);
		idx++;
	}
}

void compDevFlush(Computer* comp) {
	int idx = 0;
	xDevice* dev;
	while((idx < MAX_DEV_COUNT) && comp->devList[idx]) {
		dev = comp->devList[idx];
		if (dev->flush)
			dev->flush(dev->ptr.ptr);
		idx++;
	}
}

void compSetDevArg(Computer* comp, int devid, int argid, xArg arg) {
	xDevice* dev = compFindDev(comp, devid);
	if (!dev) return;
	if (!dev->set) return;
	dev->set(dev->ptr.ptr, argid, arg);
}

xArg compGetDevArg(Computer* comp, int devid, int argid) {
	xArg arg;
	arg.b = 0;
	arg.i = 0;
	arg.s = NULL;
	xDevice* dev = compFindDev(comp, devid);
	if (!dev) return arg;
	if (!dev->get) return arg;
	return dev->get(dev->ptr.ptr, argid);
}

// ...

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

	comp->bus.busy = 0;
	comp->bus.iorq = 1;
	comp->bus.memrq = 0;
	comp->bus.rd = 1;
	comp->bus.adr = port;
	comp->bus.dos = comp->dos;
	compDevRequest(comp);
	if (comp->bus.busy) return comp->bus.data;

	if (ideIn(comp->ide, port, &res, bdiz)) return res;
//	if (gsIn(comp->gs, port, &res)) return res;
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
	int i;
	memset(ptr,0,sizeof(Computer));
	memset(comp->brkIOMap, 0, 0x10000);
	comp->resbank = RES_48;
	comp->firstRun = 1;

	comp->cpu = cpuCreate(CPU_Z80,&memrd,&memwr,&iord,&iowr,&intrq,ptr);
	comp->mem = memCreate();
	comp->vid = vidCreate(comp->mem);

	comp->frqMul = 1;
	compSetBaseFrq(comp, 3.5);

	memset(comp->brkIOMap, 0, 0x10000);
	memset(comp->brkRomMap, 0, 0x80000);
	memset(comp->brkRamMap, 0, 0x400000);
	memset(comp->brkAdrMap, 0, 0x1000);

	for(i = 0; i < MAX_DEV_COUNT; i++)
		comp->devList[i] = NULL;

	compAddDev(comp, DEV_GSOUND);
	compAddDev(comp, DEV_SDRIVE);
	compAddDev(comp, DEV_SAA);

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
	comp->slot = sltCreate();
// sound
	comp->ts = tsCreate(TS_NONE,SND_AY,SND_NONE);
//	comp->gs = gsCreate();
//	comp->sdrv = sdrvCreate(SDRV_NONE);
	comp->gbsnd = gbsCreate();
//	comp->saa = saaCreate();
	comp->beep = bcCreate();
// baseconf
	comp->evo.evo2F = 0;
	comp->evo.evo4F = 0;
	comp->evo.evo6F = 0;
	comp->evo.evo8F = 0;
//					0   1   2   3   4   5   6   7   8    9    A    B    C    D    E    F
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

//	gsReset(comp->gs);
//	zxInitPalete(comp);
	comp->cmos.adr = 0;
	memset(comp->cmos.data, 0x00, 256);
	comp->cmos.data[17] = 0xaa;
	return comp;
}

void compDestroy(Computer* comp) {
	rzxStop(comp);

	compDelAllDev(comp);

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
//	gsDestroy(comp->gs);
	gbsDestroy(comp->gbsnd);
//	sdrvDestroy(comp->sdrv);
	bcDestroy(comp->beep);
	sltDestroy(comp->slot);
	free(comp);
}

void compReset(Computer* comp,int res) {
	if (comp->rzx.play) rzxStop(comp);
	vidReset(comp->vid);
	comp->prt2 = 0;
	comp->p1FFD = 0;
	comp->pEFF7 = 0;
	comp->rom = 0;
	comp->dos = 0;
	memSetBank(comp->mem, MEM_BANK0, MEM_ROM, 0, NULL, NULL, NULL);
	memSetBank(comp->mem, MEM_BANK1, MEM_RAM, 5, NULL, NULL, NULL);
	memSetBank(comp->mem, MEM_BANK2, MEM_RAM, 2, NULL, NULL, NULL);
	memSetBank(comp->mem, MEM_BANK3, MEM_RAM, 0, NULL, NULL, NULL);
	comp->cpu->reset(comp->cpu);

	compDevReset(comp);		// send RESET to external devices

	difReset(comp->dif);	//	bdiReset(comp->bdi);
//	if (comp->gs->reset) gsReset(comp->gs);
	tsReset(comp->ts);
	ideReset(comp->ide);
//	saaReset(comp->saa);
	if (res == RES_DEFAULT) res = comp->resbank;
	comp->p7FFD = ((res == RES_DOS) || (res == RES_48)) ? 0x10 : 0x00;
	comp->dos = ((res == RES_DOS) || (res == RES_SHADOW)) ? 1 : 0;
	comp->rom = (comp->p7FFD & 0x10) ? 1 : 0;
	comp->cpm = 0;
	if (comp->hw->reset) comp->hw->reset(comp);
	comp->hw->mapMem(comp);
}

void compSetLayout(Computer *comp, vLayout lay) {
	if (comp->vid->lay.lock) return;
	comp->vid->lay = lay;
	comp->vid->frmsz = lay.full.x * lay.full.y;
}

// cpu freq

void compUpdateTimings(Computer* comp) {
	long perNoTurbo = 1e3 / comp->cpuFrq;
	if (perNoTurbo & 1) perNoTurbo++;
	comp->nsPerTick = perNoTurbo / comp->frqMul;
	int type = comp->hw ? comp->hw->id : HW_NULL;
	switch (type) {
		case HW_MSX:
		case HW_MSX2:
			perNoTurbo = perNoTurbo * 5 / 6;				// convert 50Hz->60Hz
			comp->nsPerTick = perNoTurbo / comp->frqMul;
			vidUpdateTimings(comp->vid, perNoTurbo >> 1);
			break;
		case HW_GBC:
			comp->gbsnd->wav.period = perNoTurbo << 5;			// 128KHz period for wave generator = cpu.frq / 32
			comp->gb.timer.div.per = comp->nsPerTick << 8;			// 16KHz timer divider tick. this timer depends on turbo speed
			vidUpdateTimings(comp->vid, perNoTurbo << 1);
			break;
		default:
			vidUpdateTimings(comp->vid, perNoTurbo >> 1);
			break;
	}
#ifdef ISDEBUG
	// printf("%f x %i : %i ns\n",comp->cpuFrq,comp->frqMul,comp->nsPerTick);
#endif
}

void compSetBaseFrq(Computer* comp, double frq) {
	comp->cpuFrq = frq;
	compUpdateTimings(comp);
}

void compSetTurbo(Computer* comp, int mult) {
	comp->frqMul = mult;
	comp->cpu->speed = (mult > 1) ? 1 : 0;
	compUpdateTimings(comp);
}

// hardware

void compSetHardware(Computer* comp, const char* name) {
	HardWare* hw = findHardware(name);
	if (hw == NULL) return;
	comp->hw = hw;
	comp->vid->istsconf = (hw->id == HW_TSLAB) ? 1 : 0;
	comp->vid->ismsx = ((hw->id == HW_MSX) || (hw->id == HW_MSX2)) ? 1 : 0;
	comp->vid->isgb = (hw->id == HW_GBC) ? 1 : 0;
	compUpdateTimings(comp);
}

// exec 1 opcode, sync devices, return eated ns

int compExec(Computer* comp) {
	res4 = 0;
	res2 = 0;
	comp->vid->time = 0;
// exec cpu opcode OR handle interrupt. get T states back
	res2 = comp->cpu->exec(comp->cpu);
// scorpion WAIT: add 1T to odd-T command
	if (comp->evenM1 && (res2 & 1))
		res2++;
// out @ last tick
	vidSync(comp->vid,(res2 - res4) * comp->nsPerTick);
	if (comp->padr) {

		comp->bus.busy = 0;
		comp->bus.iorq = 1;
		comp->bus.memrq = 0;
		comp->bus.rd = 0;
		comp->bus.adr = comp->padr;
		comp->bus.data = comp->pval;
		compDevRequest(comp);

		tapSync(comp->tape,comp->tapCount);
		comp->tapCount = 0;
		bdiz = (comp->dos && (comp->dif->type == DIF_BDI)) ? 1 : 0;
		ideOut(comp->ide, comp->padr, comp->pval, bdiz);
//		gsOut(comp->gs, comp->padr, comp->pval);
//		if (!bdiz) saaWrite(comp->saa, comp->padr, comp->pval);		// bdi ports must be closed!
		if (ulaOut(comp->vid->ula, comp->padr, comp->pval)) {
			if (comp->vid->ula->palchan) {
				zxSetUlaPalete(comp);
				comp->vid->ula->palchan = 0;
			}
		}
		comp->hw->out(comp, comp->padr, comp->pval, bdiz);
		comp->padr = 0;
	}
	// vidSync(comp->vid, comp->nsPerTick);
// ...
	// res1 = res2;
	pcreg = comp->cpu->pc;
// NMI
	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzx.play) {
		comp->cpu->intrq |= 2;		// request nmi
		comp->dos = 1;			// set dos page
		comp->rom = 1;
		comp->hw->mapMem(comp);
	}
// execution completed : get eated time
	nsTime = comp->vid->time;
	comp->tickCount += res2;
// sync / INT detection
	if (comp->rzx.play) {
		if (comp->rzx.frm.fetches < 1) {
			comp->intVector = 0xff;
			comp->cpu->intrq |= 1;
			comp->rzx.fCurrent++;
			comp->rzx.fCount--;
			rzxGetFrame(comp);
		}
	} else if (comp->hw->sync) {
		comp->hw->sync(comp, nsTime);
	}
// new frame
	if (comp->vid->newFrame) {
		comp->vid->newFrame = 0;
		comp->frmStrobe = 1;
	}
// TSConf : update 'next-line' registers			TODO:move to TSConf hw->sync
	if (comp->vid->nextrow && comp->vid->istsconf) {
		tslUpdatePorts(comp);
		comp->vid->nextrow = 0;
	}
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

	compDevSync(comp, nsTime);

	comp->beep->accum += nsTime;
	comp->tapCount += nsTime;
//	if (comp->gs->enable)
//		comp->gs->sync += nsTime;
	difSync(comp->dif, nsTime);
	// tsSync(comp->ts, nsTime);

	if (comp->hw->id == HW_GBC) {
		// sound
		gbsSync(comp->gbsnd, nsTime);
		// timer
		if (comp->gb.timer.div.per) {
			comp->gb.timer.div.cnt -= nsTime;
			if (comp->gb.timer.div.cnt < 0) {
				comp->gb.timer.div.cnt += comp->gb.timer.div.per;
				comp->gb.iomap[0x04]++;
			}
		}
		if (comp->gb.timer.t.on && (comp->gb.timer.t.per > 0)) {
			comp->gb.timer.t.cnt -= nsTime;
			if (comp->gb.timer.t.cnt < 0) {
				comp->gb.timer.t.cnt += comp->gb.timer.t.per;
				if (comp->gb.iomap[0x05] == 0xff) {		// overflow
					comp->gb.iomap[0x05] = comp->gb.iomap[0x06];
					comp->gb.timer.t.intrq = 1;
				} else {
					comp->gb.iomap[0x05]++;
				}
			}
		}
		// cpu speed change
		if (comp->cpu->stop && comp->cpu->speedrq) {
			comp->cpu->stop = 0;
			comp->cpu->pc++;
			comp->cpu->speedrq = 0;
			comp->cpu->speed ^= 1;
			compSetTurbo(comp, comp->cpu->speed ? 2.0 : 1.0);
		}
	}
// return ns eated @ this step
	return nsTime;
}

// cmos

unsigned char toBCD(unsigned char val) {
	unsigned char rrt = val % 10;
	rrt |= ((val/10) << 4);
	return rrt;
}

#include <time.h>

unsigned char cmsRd(Computer* comp) {
	unsigned char res = 0x00;

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
	xAdr xadr = memGetXAdr(comp->mem, madr);
	unsigned char* ptr;
	if (xadr.type == MEM_RAM) {
		ptr = comp->brkRamMap + xadr.abs;
	} else {
		ptr = comp->brkRomMap + xadr.abs;
	}
	return ptr;
}

void setBrk(Computer* comp, unsigned short adr, unsigned char val) {
	xAdr xadr = memGetXAdr(comp->mem, adr);
	if (xadr.type == MEM_RAM) {
		comp->brkRamMap[xadr.abs] = val;
	} else {
		comp->brkRomMap[xadr.abs] = val;
	}
}

unsigned char getBrk(Computer* comp, unsigned short adr) {
	unsigned char res = 0x00;
	xAdr xadr = memGetXAdr(comp->mem, adr);
	if (xadr.type == MEM_RAM) {
		res = comp->brkRamMap[xadr.abs];
	} else {
		res = comp->brkRomMap[xadr.abs];
	}
	res |= (comp->brkAdrMap[adr] & 0x0f);
	return res;
}
