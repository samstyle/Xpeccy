#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "spectrum.h"

double ltk;
int res1 = 0;
int res2 = 0;
int res3 = 0;	// tick in op, wich has last OUT/MWR (and vidSync)
int res4 = 0;	// save last res3 (vidSync on OUT/MWR process do res3-res4 ticks)
int res5 = 0;	// ticks ated by slow mem?
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

Z80EX_BYTE memrd(Z80EX_CONTEXT* cpu,Z80EX_WORD adr,int m1,void* ptr) {
	Z80EX_BYTE res;
	ZXComp* comp = (ZXComp*)ptr;
	if ((comp->hw->type == HW_SCORP) && ((comp->mem->pt0->num & 3) == 2) && ((adr & 0xfff3) == 0x0100)) {
		comp->prt2 = ZSLays[(adr & 0x000c) >> 2][comp->prt2 & 3];
		comp->hw->mapMem(comp);
	}
	if (m1 == 1) {
		if (comp->rzxPlay) comp->rzxFetches--;
		if (comp->bdi->fdc->type == FDC_93) {
			if ((comp->dosen == 0) && ((adr & 0xff00) == 0x3d00) && (comp->prt0 & 0x10)) {
				comp->dosen = 1;
				comp->hw->mapMem(comp);
			}
			if ((comp->dosen == 1) && (memGetBankPtr(comp->mem,adr)->type == MEM_RAM)	// off when fetch in ram
					&& !((comp->hw->type == HW_ATM2) && (~comp->prt1 & 2))		// but not ATM2 cpm mode
					&& !((comp->hw->type == HW_PENTEVO && (~comp->prt1 & 0x40)))		// and not PentEvo keep dos alive
					) {
				comp->dosen = 0;
				comp->hw->mapMem(comp);
			}
		}
	}
//	res3 = res2 + z80ex_op_tstate(cpu);
//	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
//	res4 = res3;
//	if (((adr & 0xc000) == 0x4000) && (comp->hwFlag & HW_CONTMEM)) {
//		res5 = vidGetWait(comp->vid);
//		if (res5 != 0) {
//			vflg |= vidSync(comp->vid, comp->dotPerTick * res5);
//			res1 += res5;
//		}
//	}
	res = memRd(comp->mem,adr);
	return res;
}

void memwr(Z80EX_CONTEXT* cpu, Z80EX_WORD adr, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	res3 = res2 + z80ex_op_tstate(cpu);
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
//	if (((adr & 0xc000) == 0x4000) && (comp->hwFlag & HW_CONTMEM)) {
//		res5 = vidGetWait(comp->vid);
//		if (res5 != 0) {
//			vflg |= vidSync(comp->vid,comp->dotPerTick * res5);
//			res1 += res5;
//		}
//	}
	if ((comp->hw->type == HW_PENTEVO) && (comp->evo.evoBF & 4)) {		// PentEvo: write font byte
		comp->vid->font[adr & 0x7ff] = val;
	}
	memWr(comp->mem,adr,val);
	if (comp->mem->flags & MEM_BRK_WRITE) {
		comp->flag |= ZX_BREAK;
	}
}

Z80EX_BYTE iord(Z80EX_CONTEXT* cpu, Z80EX_WORD port, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	Z80EX_BYTE res = 0xff;
// video sync
	res3 = res2 + z80ex_op_tstate(cpu);
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
	if (comp->hwFlag & HW_CONTIO) {
		res5 = vidGetWait(comp->vid);
		if (res5 != 0) {
			vflg |= vidSync(comp->vid,comp->dotPerTick * res5);
			res1 += res5;
		}
	}

	tapSync(comp->tape,comp->tapCount);
	comp->tapCount = 0;
	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
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
// request to external devices
	if (ideIn(comp->ide,port,&res,comp->dosen & 1)) return res;
	if (gsIn(comp->gs,port,&res) == GS_OK) return res;
// request to internal devices
	int bdiz = ((comp->dosen & 1) && (comp->bdi->fdc->type == FDC_93)) ? 1 : 0;
	res = comp->hw->in(comp,port,bdiz);
	return res;
}

void zxOut(ZXComp *comp, Z80EX_WORD port, Z80EX_BYTE val) {
	tapSync(comp->tape,comp->tapCount);
	comp->tapCount = 0;
	gsSync(comp->gs,comp->gsCount);
	comp->gsCount = 0;
// request to external devices
	if (ideOut(comp->ide,port,val,comp->dosen & 1)) return;
	if (gsOut(comp->gs,port,val) == GS_OK) return;
// request to internal devices
	int bdiz = ((comp->dosen & 1) && (comp->bdi->fdc->type == FDC_93)) ? 1 : 0;
	comp->hw->out(comp,port,val,bdiz);
}

void iowait(ZXComp* comp, int ticks) {
	res5 = vidGetWait(comp->vid);
	if (res5 != 0) {
		vflg |= vidSync(comp->vid, comp->dotPerTick * res5);
		res1 += res5;
	}
	if (ticks != 0)
		vflg |= vidSync(comp->vid, comp->dotPerTick * ticks);
}

void iowr(Z80EX_CONTEXT* cpu, Z80EX_WORD port, Z80EX_BYTE val, void* ptr) {
	ZXComp* comp = (ZXComp*)ptr;
	res3 = res2 + z80ex_op_tstate(cpu) - 1;		// start of OUT cycle
	vflg |= vidSync(comp->vid,comp->dotPerTick * (res3 - res4));
	res4 = res3;
// if there is contended io, get wait and wait :)
	if (comp->hwFlag & HW_CONTIO) {
		switch(port & 0x4001) {
			case 0x0000:
				zxOut(comp,port,val);
				vflg |= vidSync(comp->vid,comp->dotPerTick);	// N:1
				iowait(comp,3);					// C:3
				break;
			case 0x0001:
				zxOut(comp,port,val);
				vflg |= vidSync(comp->vid, comp->dotPerTick * 4);	// N:4
				break;
			case 0x4000:
				zxOut(comp,port,val);
				iowait(comp,1);			// C:1
				iowait(comp,3);			// C:3
				break;
			case 0x4001:
				zxOut(comp,port,val);
				iowait(comp,1);			// C:1	4 times
				iowait(comp,1);
				iowait(comp,1);
				iowait(comp,1);
				break;
		}
		res4 += 4;
	} else {
		vflg |= vidSync(comp->vid, comp->dotPerTick * 3);
		zxOut(comp,port,val);
		res4 += 3;
	}
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

// 76543210
// -grb-GRB
const unsigned char defPalete[16] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77
};

ZXComp* zxCreate() {
	int i;
	ZXComp* comp = (ZXComp*)malloc(sizeof(ZXComp));
	void* ptr = (void*)comp;
	memset(ptr,0,sizeof(ZXComp));
	comp->flag = ZX_JUSTBORN | ZX_PALCHAN;
	comp->hwFlag = 0;

	comp->cpu = z80ex_create(&memrd,ptr,&memwr,ptr,&iord,ptr,&iowr,ptr,&intrq,ptr);
	zxSetFrq(comp,3.5);
	comp->mem = memCreate();
	comp->vid = vidCreate(comp->mem);
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
// evo
	comp->evo.evo2F = 0;
	comp->evo.evo4F = 0;
	comp->evo.evo6F = 0;
	comp->evo.evo8F = 0;

	comp->rzxSize = 0;
	comp->rzxData = NULL;
	comp->gsCount = 0;
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
	rzxClear(comp);
	comp->rzxPlay = 0;
	z80ex_reset(comp->cpu);
	comp->vid->curscr = 0;
	vidSetMode(comp->vid,VID_NORMAL);
	comp->dosen = 0;
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

void zxSetFrq(ZXComp* comp, float frq) {
	comp->cpuFrq = frq;
	comp->dotPerTick = 7.0 / frq;
	comp->nsPerTick = 1000.0 / frq;
}

int zxExec(ZXComp* comp) {
	res1 = res2 = res3 = res4 = res5 = 0;
	comp->vid->drawed = 0;
	vflg = 0;
	do {
		res2 += z80ex_step(comp->cpu);
	} while (z80ex_last_op_type(comp->cpu) != 0);
	pcreg = z80ex_get_reg(comp->cpu,regPC);
	vflg |= vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
	res1 += res2;
	if (comp->rzxPlay) {
		comp->intStrobe = (comp->rzxFetches < 1);
	} else {
		comp->intStrobe = (vflg & VID_INT) ? 1 : 0;
	}
	comp->frmStrobe = (vflg & VID_FRM) ? 1 : 0;
	if ((pcreg > 0x3fff) && comp->nmiRequest && !comp->rzxPlay) {
		res2 = res3 = res4 = res5 = 0;
		res2 = z80ex_nmi(comp->cpu);
		res1 += res2;
		if (res2 != 0) {
			comp->dosen = 1;
			comp->prt0 |= 0x10;
			comp->hw->mapMem(comp);
			vidSync(comp->vid,(res2 - res4) * comp->dotPerTick);
		}
	}
	if (comp->intStrobe) {
		res2 = res3 = res4 = res5 = 0;
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
		comp->flag |= ZX_BREAK;
	}

	comp->tickCount += res1;

	ltk = comp->vid->drawed; // res1 * comp->dotPerTick;
	comp->tapCount += ltk;
	if (comp->gs->flag & GS_ENABLE) comp->gsCount += ltk;
	if (comp->bdi->fdc->type != FDC_NONE) bdiSync(comp->bdi,ltk);

	return res1 * comp->nsPerTick;
}
