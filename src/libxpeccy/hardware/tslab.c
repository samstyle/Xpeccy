#include "hardware.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define p7FFD comp->prt0
#define pEFF7 comp->prt1
#define p21AF comp->prt2

void tslReset(ZXComp* comp) {
	comp->vid->tsconf.scrPal = 0xf0;
	memset(comp->vid->tsconf.cram,0x00,0x1e0);
	comp->prt2 = 0x04;
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
	comp->vid->intMask = 1;
	comp->tsconf.vdos = 0;
	tslUpdatePorts(comp);
}

void tslMapMem(ZXComp* comp) {
// bank0 maping taken from Unreal(TSConf)
	if (comp->tsconf.vdos) {
		memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0xff);		// vdos on : ramFF in bank0
	} else if (p21AF & 8) {
		if (p21AF & 4)
			memSetBank(comp->mem,MEM_BANK0,MEM_RAM,comp->tsconf.Page0);
		else
			memSetBank(comp->mem,MEM_BANK0,MEM_RAM, (comp->tsconf.Page0 & 0xfc) | ((p7FFD & 0x10) ? 1 : 0) | (comp->dosen ? 0 : 2));
	} else {
		if (p21AF & 4)
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,comp->tsconf.Page0);
		else
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM, (comp->tsconf.Page0 & 0xfc) | ((p7FFD & 0x10) ? 1 : 0) | (comp->dosen ? 0 : 2));
	}
}

const int tslXRes[4] = {256,320,320,360};
const int tslYRes[4] = {192,200,240,288};

const unsigned char tslCoLevs[32] = {
	0,11,21,32,42,53,64,74,
	85,95,106,117,127,138,148,159,
	170,180,191,201,212,223,233,244,
	255,255,255,255,255,255,255,255
};

const unsigned char tsl5bLevs[32] = {
	0,8,16,24,32,41,49,57,
	65,74,82,90,98,106,115,123,
	131,139,148,156,164,172,180,189,
	197,205,213,222,230,238,246,255
};

void tslUpdatePal(ZXComp* comp) {
	int col;
	for (int i = 0; i < 256; i++) {
		col = (comp->vid->tsconf.cram[(i << 1) + 1] << 8) | (comp->vid->tsconf.cram[i << 1]);
		if (col & 0x8000) {
			comp->vid->pal[i].r = tsl5bLevs[(col >> 10) & 0x1f];
			comp->vid->pal[i].g = tsl5bLevs[(col >> 5) & 0x1f];
			comp->vid->pal[i].b = tsl5bLevs[col  & 0x1f];
		} else {
			comp->vid->pal[i].r = tslCoLevs[(col >> 10) & 0x1f];
			comp->vid->pal[i].g = tslCoLevs[(col >> 5) & 0x1f];
			comp->vid->pal[i].b = tslCoLevs[col  & 0x1f];
		}
	}
}

Z80EX_BYTE tslMRd(ZXComp* comp, Z80EX_WORD adr, int m1) {
	if (m1 && (comp->dif->type == DIF_BDI)) {
		if (comp->dosen && (adr > 0x4000) && (!comp->tsconf.vdos)) {
			comp->dosen = 0;
			if (p7FFD & 0x10) comp->hw->mapMem(comp);	// don't switch ROM0 to ROM2
		}
		if (!comp->dosen && ((adr & 0xff00) == 0x3d00) && (p7FFD & 0x10) && ((p21AF & 0x04) == 0x00)) {
			comp->dosen = 1;
			comp->hw->mapMem(comp);
		}
	}
	return memRd(comp->mem,adr);
}

void tslMWr(ZXComp* comp, Z80EX_WORD adr, Z80EX_BYTE val) {
	if ((comp->tsconf.flag & 0x10) && ((adr & 0xf000) == comp->tsconf.tsMapAdr)) {
		if ((adr & 0xe00) == 0x000) {
			comp->vid->tsconf.cram[adr & 0x1ff] = val;
			tslUpdatePal(comp);
			//comp->palchan = 1; // comp->flag |= ZX_PALCHAN;
		}
		if ((adr & 0xe00) == 0x200) {
			comp->vid->tsconf.sfile[adr & 0x1ff] = val;
		}
	}
	memWr(comp->mem,adr,val);
}

void tslUpdatePorts(ZXComp* comp) {
	unsigned char val = comp->tsconf.p00af;
	comp->vid->tsconf.xSize = tslXRes[(val & 0xc0) >> 6];
	comp->vid->tsconf.ySize = tslYRes[(val & 0xc0) >> 6];
	comp->vid->tsconf.xPos = comp->vid->sync.h + ((comp->vid->full.h - comp->vid->sync.h - comp->vid->tsconf.xSize) >> 1);
	comp->vid->tsconf.yPos = comp->vid->sync.v + ((comp->vid->full.v - comp->vid->sync.v - comp->vid->tsconf.ySize) >> 1);
	switch(val & 3) {
		case 0: vidSetMode(comp->vid,VID_TSL_NORMAL); break;
		case 1: vidSetMode(comp->vid,VID_TSL_16); break;
		case 2: vidSetMode(comp->vid,VID_TSL_256); break;
		case 3: vidSetMode(comp->vid,VID_TSL_TEXT); break;
		default: vidSetMode(comp->vid,VID_UNKNOWN); break;	// never
	}
	comp->vid->nogfx = (val & 0x20) ? 1 : 0;

	comp->vid->tsconf.vidPage = comp->tsconf.p01af;
	comp->vid->tsconf.soxl = comp->tsconf.p02af;
	comp->vid->tsconf.soxh = comp->tsconf.p03af;
	comp->vid->tsconf.soyl = comp->tsconf.p04af;
	comp->vid->tsconf.soyh = comp->tsconf.p05af;
	val = comp->tsconf.p07af;
	comp->vid->tsconf.scrPal = (val & 0x0f) << 4;
	comp->vid->tsconf.T0Pal76 = (val & 0x30) << 2;
	comp->vid->tsconf.T1Pal76 = (val & 0xc0);
}

// in

Z80EX_BYTE tsInFF(ZXComp* comp, Z80EX_WORD port) {			// dos
	Z80EX_BYTE res = 0xff;
	if (comp->dif->fdc->flp->virt) {
		comp->tsconf.vdos = 1;
		tslMapMem(comp);
	} else {
		difIn(comp->dif, port, &res, 1);
	}
	return res;
}

Z80EX_BYTE tsInBDI(ZXComp* comp, Z80EX_WORD port) {			// dos
	Z80EX_BYTE res = 0xff;
	if (comp->tsconf.vdos) {
		comp->tsconf.vdos = 0;
		tslMapMem(comp);
	} else {
		res = tsInFF(comp, port);
	}
	return res;
}

Z80EX_BYTE tsIn57(ZXComp* comp, Z80EX_WORD port) {
	return sdcRead(comp->sdc);
}

Z80EX_BYTE tsIn77(ZXComp* comp, Z80EX_WORD port) {
	Z80EX_BYTE res = 0x00;
	if (comp->sdc->image != NULL) res |= 0x01;	// inserted
	if (comp->sdc->flag & SDC_LOCK) res |= 0x02;	// wrprt
	return res;
}

Z80EX_BYTE tsInBFF7(ZXComp* comp, Z80EX_WORD port) {
	return (pEFF7 & 0x80) ? cmsRd(comp) : 0xff;
}

// out

void tsOutBDI(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {		// dos
	if (comp->tsconf.vdos) {
		comp->tsconf.vdos = 0;
		tslMapMem(comp);
	} else {
		if (comp->dif->fdc->flp->virt) {
			comp->tsconf.vdos = 1;
			tslMapMem(comp);
		} else {
			difOut(comp->dif, port, val, 1);
		}
	}
}

void tsOutFF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {		// dos
	comp->dif->fdc->flp = comp->dif->fdc->flop[val & 3];
	if (comp->dif->fdc->flp->virt) {
		comp->tsconf.vdos = 1;
		tslMapMem(comp);
	} else if (comp->tsconf.vdos) {
		// comp->dif->fdc->fptr = comp->dif->fdc->flop[val & 3];	// out VGSys[1:0]
	} else {
		difOut(comp->dif, port, val, 1);
	}
}

void tsOutFE(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->brdcol = 0xf0 | (val & 7);
	comp->vid->nextbrd = comp->vid->brdcol;
	comp->beeplev = (val & 0x10) ? 1 : 0;
	comp->tape->levRec = (val & 0x08) ? 1 : 0;
}

void tsOutFB(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	sdrvOut(comp->sdrv, 0xfb, val);
}

void tsOut57(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	sdcWrite(comp->sdc,val);
}

void tsOut77(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->sdc->flag &= ~0x03;
	comp->sdc->flag |= (val & 3);
}

void tsOut7FFD(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (p7FFD & 0x20) return;
	p7FFD = val;
	int num = (val & 7) | ((val & 0xc0) >> 3);		// page (512K)
	if (p21AF & 0x80) {				// 1x : !a13
		if (~port & 0x2000) num &= 7;
	} else if (p21AF & 0x40) {			// 01 : 128
		num &= 7;
	}
	memSetBank(comp->mem,MEM_BANK3,MEM_RAM,num);
	comp->vid->tsconf.vidPage = 5;
	comp->vid->curscr = (val & 8) ? 7 : 5;
	tslMapMem(comp);
}

void tsOutBFF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (pEFF7 & 0x80) cmsWr(comp,val);
}

void tsOutDFF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	if (pEFF7 & 0x80) comp->cmos.adr = val;
}

void tsOutEFF7(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	pEFF7 = val;
}

// xxaf

Z80EX_BYTE tsIn00AF(ZXComp* comp, Z80EX_WORD port) {
	Z80EX_BYTE res = comp->tsconf.pwr_up ? 0x40 : 0x00;	// b6: PWR_UP (1st run)
	comp->tsconf.pwr_up = 0;
	res |= 3;						// 11 : 5bit VDAC
	return res;
}

void tsOut00AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->tsconf.p00af = val;}
void tsOut01AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->tsconf.p01af = val;}
void tsOut02AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->tsconf.p02af = val;}
void tsOut03AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->tsconf.p03af = val;}

void tsOut04AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->tsconf.p04af = val;
	comp->vid->tsconf.scrLine = ((comp->tsconf.p05af & 1) << 8) | (comp->tsconf.p04af);
}

void tsOut05AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->tsconf.p05af = val;
	comp->vid->tsconf.scrLine = ((comp->tsconf.p05af & 1) << 8) | (comp->tsconf.p04af);
}

void tsOut06AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.tconfig = val;}
void tsOut07AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->tsconf.p07af = val;}
void tsOut0FAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->nextbrd = val;}

void tsOut10AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->tsconf.Page0 = val;
	tslMapMem(comp);
}

void tsOut11AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	memSetBank(comp->mem,MEM_BANK1,MEM_RAM,val);
}

void tsOut12AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {memSetBank(comp->mem,MEM_BANK2,MEM_RAM,val);}
void tsOut13AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {memSetBank(comp->mem,MEM_BANK3,MEM_RAM,val);}

Z80EX_BYTE tsIn12AF(ZXComp* comp, Z80EX_WORD port) {return comp->mem->pt[2]->num;}
Z80EX_BYTE tsIn13AF(ZXComp* comp, Z80EX_WORD port) {return comp->mem->pt[3]->num;}

void tsOut15AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->tsconf.flag = val & 0x10;		// FM_EN
	comp->tsconf.tsMapAdr = ((val & 0x0f) << 12);
}

void tsOut16AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.TMPage = val;}
void tsOut17AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.T0GPage = val & 0xf8;}
void tsOut18AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.T1GPage = val & 0xf8;}
void tsOut19AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.SGPage = val & 0xf8;}

void tsOut1AAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.src.l = val;}
void tsOut1BAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.src.h = val;}
void tsOut1CAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.src.x = val;}
void tsOut1DAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.dst.l = val;}
void tsOut1EAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.dst.h = val;}
void tsOut1FAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.dst.x = val;}

void tsOut20AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	switch (val & 3) {
		case 0: zxSetFrq(comp,3.5); break;
		case 1: zxSetFrq(comp,7.0); break;
		case 2: zxSetFrq(comp,14.0); break;	// normal
		case 3: zxSetFrq(comp,14.0); break;	// overclock
	}
}

void tsOut21AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	p21AF = val;
	p7FFD &= ~0x10;
	if (val & 1) p7FFD |= 0x10;
	//if (val & 2) {comp->mem->flags &= ~MEM_B0_WP;} else {comp->mem->flags |= MEM_B0_WP;}
	tslMapMem(comp);
}

void tsOut22AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->intpos.h = (val << 1);}
void tsOut23AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->intpos.v &= 0xff00;
	comp->vid->intpos.v |= val;
}

void tsOut24AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->vid->intpos.v &= 0x00ff;
	comp->vid->intpos.v |= ((val & 1) << 8);
}

void tsOut26AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.len = val;}

Z80EX_BYTE tsIn27AF(ZXComp* comp, Z80EX_WORD port) {return 0x00;}

void tsOut27AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	int cnt, cnt2;
	unsigned char tmp;
	unsigned char* ptr = NULL;
	int sadr = (comp->dma.src.x << 14) | ((comp->dma.src.h & 0x3f) << 8) | (comp->dma.src.l & 0xfe);
	int dadr = (comp->dma.dst.x << 14) | ((comp->dma.dst.h & 0x3f) << 8) | (comp->dma.dst.l & 0xfe);
	int lcnt = (comp->dma.len + 1) << 1;
	switch (val & 0x87) {
		case 0x01:		// ram->ram
			for (cnt = 0; cnt <= comp->dma.num; cnt++) {
				for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
					comp->mem->ramData[dadr + cnt2] = comp->mem->ramData[sadr + cnt2];
				}
				sadr += (val & 0x20) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// SALGN
				dadr += (val & 0x10) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// DALGN
			}
			break;
		case 0x81:		// blitter
			for (cnt = 0; cnt <= comp->dma.num; cnt++) {
				for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
					tmp = comp->mem->ramData[sadr + cnt2];
					if (val & 0x08) {
						if (tmp != 0) comp->mem->ramData[dadr + cnt2] = tmp;
					} else {
						if (tmp & 0xf0) {
							comp->mem->ramData[dadr + cnt2] &= 0x0f;
							comp->mem->ramData[dadr + cnt2] |= (tmp & 0xf0);
						}
						if (tmp & 0x0f) {
							comp->mem->ramData[dadr + cnt2] &= 0xf0;
							comp->mem->ramData[dadr + cnt2] |= (tmp & 0x0f);
						}
					}
					// comp->mem->ramData[dadr + cnt2] = comp->mem->ramData[sadr + cnt2];
				}
				sadr += (val & 0x20) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// SALGN
				dadr += (val & 0x10) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// DALGN
			}
			break;
		case 0x02:		// SPI->RAM
			for (cnt = 0; cnt <= comp->dma.num; cnt++) {
				for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
					comp->mem->ramData[dadr + cnt2] = sdcRead(comp->sdc);
				}
				dadr += (val & 0x10) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;
			}
			break;
		case 0x82:		// RAM->SPI
			for (cnt = 0; cnt <= comp->dma.num; cnt++) {
				for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
					sdcWrite(comp->sdc, comp->mem->ramData[sadr + cnt2]);
				}
				sadr += (val & 0x20) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;
			}
			break;
		case 0x03:		// IDE->RAM
			for (cnt = 0; cnt <= comp->dma.num; cnt++) {
				for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
					if (!ideIn(comp->ide, 0x00, &tmp, 1)) tmp = 0xff;
					comp->mem->ramData[dadr + cnt2] = tmp;
				}
				dadr += (val & 0x10) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;
			}
			break;
		case 0x83:		// RAM->IDE
			for (cnt = 0; cnt <= comp->dma.num; cnt++) {
				for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
					ideOut(comp->ide, 0x00, comp->mem->ramData[sadr + cnt2], 1);
				}
				sadr += (val & 0x20) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;
			}
			break;
		case 0x04:		// FILL->RAM
			for (cnt = 0; cnt <= comp->dma.num; cnt++) {
				for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
					comp->mem->ramData[dadr + cnt2] = comp->mem->ramData[sadr + (cnt2 & 1)];
				}
				dadr += (val & 0x10) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// DALGN
			}
			break;
		case 0x84:
		case 0x85:		// RAM->SFILE
			ptr = (val & 1) ? comp->vid->tsconf.sfile : comp->vid->tsconf.cram;
			for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
				*(ptr + ((dadr + cnt2) & 0x1ff)) = comp->mem->ramData[sadr + cnt2];
			}
			if (~val & 1) tslUpdatePal(comp);
			break;
		default:
			printf("0x27AF: unsupported src-dst: %.2X\n",val & 0x87);
			comp->brk = 1;
			break;
	}
	comp->dma.src.x = ((sadr & 0x3fc000) >> 14);
	comp->dma.src.h = ((sadr & 0x3f00) >> 8);
	comp->dma.src.l = sadr & 0xff;
	comp->dma.dst.x = ((dadr & 0x3fc000) >> 14);
	comp->dma.dst.h = ((dadr & 0x3f00) >> 8);
	comp->dma.dst.l = dadr & 0xff;
	if (comp->vid->intMask & 4) {		// DMA INT
		comp->vid->intDMA = 1;
	}
}

void tsOut28AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->dma.num = val;}

void tsOut29AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	// comp->tsconf.FDDVirt = val;
	comp->dif->fdc->flop[0]->virt = (val & 0x01) ? 1 : 0;
	comp->dif->fdc->flop[1]->virt = (val & 0x02) ? 1 : 0;
	comp->dif->fdc->flop[2]->virt = (val & 0x04) ? 1 : 0;
	comp->dif->fdc->flop[3]->virt = (val & 0x08) ? 1 : 0;
}

void tsOut2AAF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->intMask = val;}

void tsOut40AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t0xl = val;}
void tsOut41AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t0xh = val & 1;}
void tsOut42AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t0yl = val;}
void tsOut43AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t0yh = val & 1;}
void tsOut44AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t1xl = val;}
void tsOut45AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t1xh = val & 1;}
void tsOut46AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t1yl = val;}
void tsOut47AF(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {comp->vid->tsconf.t1yh = val & 1;}

// catch

Z80EX_BYTE tsInCatch(ZXComp* comp, Z80EX_WORD port) {
	comp->brk = 1;
	return 0xff;
}

void tsOutCatch(ZXComp* comp, Z80EX_WORD port, Z80EX_BYTE val) {
	comp->brk = 1;
}

xPort tsPortMap[] = {
	// xxaf
	{0xffff,0x00af,1,0,&tsIn00AF,	&tsOut00AF},
	{0xffff,0x01af,1,0,NULL,	&tsOut01AF},
	{0xffff,0x02af,1,0,NULL,	&tsOut02AF},
	{0xffff,0x03af,1,0,NULL,	&tsOut03AF},
	{0xffff,0x04af,1,0,NULL,	&tsOut04AF},
	{0xffff,0x05af,1,0,NULL,	&tsOut05AF},
	{0xffff,0x06af,1,0,NULL,	&tsOut06AF},
	{0xffff,0x07af,1,0,NULL,	&tsOut07AF},
	{0xffff,0x0faf,1,0,NULL,	&tsOut0FAF},

	{0xffff,0x10af,1,0,NULL,	&tsOut10AF},
	{0xffff,0x11af,1,0,NULL,	&tsOut11AF},
	{0xffff,0x12af,1,0,&tsIn12AF,	&tsOut12AF},
	{0xffff,0x13af,1,0,&tsIn13AF,	&tsOut13AF},

	{0xffff,0x15af,1,0,NULL,	&tsOut15AF},
	{0xffff,0x16af,1,0,NULL,	&tsOut16AF},
	{0xffff,0x17af,1,0,NULL,	&tsOut17AF},
	{0xffff,0x18af,1,0,NULL,	&tsOut18AF},
	{0xffff,0x19af,1,0,NULL,	&tsOut19AF},

	{0xffff,0x1aaf,1,0,NULL,	&tsOut1AAF},
	{0xffff,0x1baf,1,0,NULL,	&tsOut1BAF},
	{0xffff,0x1caf,1,0,NULL,	&tsOut1CAF},
	{0xffff,0x1daf,1,0,NULL,	&tsOut1DAF},
	{0xffff,0x1eaf,1,0,NULL,	&tsOut1EAF},
	{0xffff,0x1faf,1,0,NULL,	&tsOut1FAF},

	{0xffff,0x20af,1,0,NULL,	&tsOut20AF},
	{0xffff,0x21af,1,0,NULL,	&tsOut21AF},
	{0xffff,0x22af,1,0,NULL,	&tsOut22AF},
	{0xffff,0x23af,1,0,NULL,	&tsOut23AF},
	{0xffff,0x24af,1,0,NULL,	&tsOut24AF},
	// {0xffff,0x25af,1,0,NULL,	NULL},		// INTVECT (obsolete)

	{0xffff,0x26af,1,0,NULL,	&tsOut26AF},
	{0xffff,0x27af,1,0,&tsIn27AF,	&tsOut27AF},

	{0xffff,0x28af,1,0,NULL,	&tsOut28AF},
	{0xffff,0x29af,1,0,NULL,	&tsOut29AF},
	{0xffff,0x2aaf,1,0,NULL,	&tsOut2AAF},
	{0xffff,0x2baf,1,0,NULL,	NULL},		// cache config

	{0xffff,0x40af,1,0,NULL,	&tsOut40AF},
	{0xffff,0x41af,1,0,NULL,	&tsOut41AF},
	{0xffff,0x42af,1,0,NULL,	&tsOut42AF},
	{0xffff,0x43af,1,0,NULL,	&tsOut43AF},
	{0xffff,0x44af,1,0,NULL,	&tsOut44AF},
	{0xffff,0x45af,1,0,NULL,	&tsOut45AF},
	{0xffff,0x46af,1,0,NULL,	&tsOut46AF},
	{0xffff,0x47af,1,0,NULL,	&tsOut47AF},
	// !dos
	{0x00f7,0x00fe,1,0,&xInFE,	&tsOutFE},	// fe
	{0x00ff,0x0057,1,0,&tsIn57,	&tsOut57},	// 57
	{0x00ff,0x0077,1,0,&tsIn77,	&tsOut77},	// 77
	{0x00ff,0x00fb,1,0,NULL,	&tsOutFB},	// fb
	{0x10ff,0xeff7,1,0,NULL,	&tsOutEFF7},	// eff7
	{0x20ff,0xdff7,1,0,NULL,	&tsOutDFF7},	// dff7
	{0x40ff,0xbff7,1,0,&tsInBFF7,	&tsOutBFF7},	// bff7
	{0x80ff,0x7ffd,1,0,NULL,	&tsOut7FFD},	// 7ffd
	{0xc0ff,0xbffd,1,0,NULL,	&xOutBFFD},	// bffd
	{0xc0ff,0xfffd,1,0,&xInFFFD,	&xOutFFFD},	// fffd
	{0xffff,0xfadf,1,0,&xInFADF,	NULL},		// fadf
	{0xffff,0xfbdf,1,0,&xInFBDF,	NULL},		// fbdf
	{0xffff,0xffdf,1,0,&xInFFDF,	NULL},		// ffdf
	// dos
	{0x009f,0x001f,0,1,&tsInBDI,	&tsOutBDI},	// 1f,3f,5f,7f
	{0x00ff,0x00ff,0,1,&tsInFF,	&tsOutFF},	// ff
	{0x0000,0x0000,1,0,NULL,	NULL},
//	{0x0000,0x0000,1,0,&tsInCatch,	&tsOutCatch}
};

void tslOut(ZXComp* comp,Z80EX_WORD port,Z80EX_BYTE val,int dos) {
	hwOut(tsPortMap, comp, port, val, dos);
}

Z80EX_BYTE tslIn(ZXComp* comp,Z80EX_WORD port,int dos) {
	return hwIn(tsPortMap, comp, port, dos);
}
