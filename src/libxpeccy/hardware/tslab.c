#include "../spectrum.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define p7FFD comp->prt0
#define pEFF7 comp->prt1
#define p21AF comp->prt2

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

int tslXRes[4] = {256,320,320,360};
int tslYRes[4] = {192,200,240,288};

Z80EX_BYTE tslMRd(ZXComp* comp, Z80EX_WORD adr, int m1) {
	if (m1 && (comp->bdi->fdc->type == FDC_93)) {
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
			comp->flag |= ZX_PALCHAN;
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
	comp->vid->tsconf.xPos = ((comp->vid->full.h - comp->vid->sync.h - comp->vid->tsconf.xSize) >> 1) + comp->vid->sync.h;
	comp->vid->tsconf.yPos = ((comp->vid->full.v - comp->vid->sync.v - comp->vid->tsconf.ySize) >> 1) + comp->vid->sync.v;
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

void tslOut(ZXComp* comp,Z80EX_WORD port,Z80EX_BYTE val,int bdiz) {
	int cnt,cnt2,lcnt,sadr,dadr;
	unsigned char* ptr;
	switch (port) {
		case 0x00af: comp->tsconf.p00af = val; break;
		case 0x01af: comp->tsconf.p01af = val; break;
		case 0x02af: comp->tsconf.p02af = val; break;
		case 0x03af: comp->tsconf.p03af = val; break;
		case 0x04af: comp->tsconf.p04af = val;
			comp->vid->tsconf.scrLine = ((comp->tsconf.p05af & 1) << 8) | (comp->tsconf.p04af);
			break;
		case 0x05af: comp->tsconf.p05af = val;
			comp->vid->tsconf.scrLine = ((comp->tsconf.p05af & 1) << 8) | (comp->tsconf.p04af);
			break;
		case 0x06af: comp->vid->tsconf.tconfig = val; break;
		case 0x07af: comp->tsconf.p07af = val; break;
		case 0x0faf: comp->vid->nextbrd = val; break;
		case 0x10af:			// Page0
			comp->tsconf.Page0 = val;
			tslMapMem(comp);
			break;
		case 0x11af:			// Page1
			memSetBank(comp->mem,MEM_BANK1,MEM_RAM,val);
			break;
		case 0x12af:			// Page2
			memSetBank(comp->mem,MEM_BANK2,MEM_RAM,val);
			break;
		case 0x13af:			// Page3
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,val);
			break;
		case 0x15af:
			comp->tsconf.flag = val & 0x10;		// FM_EN
			comp->tsconf.tsMapAdr = ((val & 0x0f) << 12);
			break;
		case 0x16af: comp->vid->tsconf.TMPage = val; break;
		case 0x17af: comp->vid->tsconf.T0GPage = val & 0xf8; break;
		case 0x18af: comp->vid->tsconf.T1GPage = val & 0xf8; break;
		case 0x19af: comp->vid->tsconf.SGPage = val & 0xf8; break;
		case 0x1aaf: comp->dma.src.l = val; break;
		case 0x1baf: comp->dma.src.h = val; break;
		case 0x1caf: comp->dma.src.x = val; break;
		case 0x1daf: comp->dma.dst.l = val; break;
		case 0x1eaf: comp->dma.dst.h = val; break;
		case 0x1faf: comp->dma.dst.x = val; break;
		case 0x20af:
			switch (val & 3) {
				case 0: zxSetFrq(comp,3.5); break;
				case 1: zxSetFrq(comp,7.0); break;
				case 2: zxSetFrq(comp,14.0); break;	// normal
				case 3: zxSetFrq(comp,14.0); break;	// overclock
			}
			break;
		case 0x21af:			// MemConfig
			p21AF = val;
			p7FFD &= ~0x10;
			if (val & 1) p7FFD |= 0x10;
			if (val & 2) {comp->mem->flags &= ~MEM_B0_WP;} else {comp->mem->flags |= MEM_B0_WP;}
			tslMapMem(comp);
			break;
		case 0x22af:					// HSINT
			comp->vid->intpos.h = (val << 1);
			break;
		case 0x23af:					// VSINTL
			comp->vid->intpos.v &= 0xff00;
			comp->vid->intpos.v |= val;
			break;
		case 0x24af:					// VSINTH
			comp->vid->intpos.v &= 0x00ff;
			comp->vid->intpos.v |= ((val & 1) << 8);
			break;
		case 0x25af:					// INTVect
			comp->intVector = val | 1;
			break;
		case 0x26af:
			comp->dma.len = val;
			break;
		case 0x27af:
			sadr = (comp->dma.src.x << 14) | ((comp->dma.src.h & 0x3f) << 8) | (comp->dma.src.l & 0xfe);
			dadr = (comp->dma.dst.x << 14) | ((comp->dma.dst.h & 0x3f) << 8) | (comp->dma.dst.l & 0xfe);
			lcnt = (comp->dma.len + 1) << 1;
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
				case 0x81: break;	// RAM->BLT
				case 0x84:		// RAM->CRAM [adr = (dadr >> 1) & 0xff]
				case 0x85:		// RAM->SFILE
					ptr = (val & 1) ? comp->vid->tsconf.sfile : comp->vid->tsconf.cram;
					for (cnt = 0; cnt <= comp->dma.num; cnt++) {
						for (cnt2 = 0; cnt2 < lcnt; cnt2++) {
							*(ptr + (((dadr + cnt2) >> 1) & 0xff)) = comp->mem->ramData[sadr + cnt2];
						}
						sadr += (val & 0x20) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// SALGN
						dadr += (val & 0x10) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// DALGN
					}
					break;
				default:
					printf("0x27AF: unsupported src-dst: %.2X\n",val & 0x87);
#ifdef ISDEBUG
					comp->flag |= ZX_BREAK;
#endif
					break;
			}
			comp->dma.src.x = ((sadr & 0x3fc000) >> 14);
			comp->dma.src.h = ((sadr & 0x3f00) >> 8);
			comp->dma.src.l = sadr & 0xff;
			comp->dma.dst.x = ((dadr & 0x3fc000) >> 14);
			comp->dma.dst.h = ((dadr & 0x3f00) >> 8);
			comp->dma.dst.l = dadr & 0xff;
			break;
		case 0x28af:
			comp->dma.num = val;
			break;
		case 0x29af:
			// comp->tsconf.FDDVirt = val;
			comp->bdi->fdc->flop[0]->flag &= ~FLP_VIRT;
			comp->bdi->fdc->flop[1]->flag &= ~FLP_VIRT;
			comp->bdi->fdc->flop[2]->flag &= ~FLP_VIRT;
			comp->bdi->fdc->flop[3]->flag &= ~FLP_VIRT;
			if (val & 0x01) comp->bdi->fdc->flop[0]->flag |= FLP_VIRT;
			if (val & 0x02) comp->bdi->fdc->flop[1]->flag |= FLP_VIRT;
			if (val & 0x04) comp->bdi->fdc->flop[2]->flag |= FLP_VIRT;
			if (val & 0x08) comp->bdi->fdc->flop[3]->flag |= FLP_VIRT;
			break;
		case 0x2aaf:
			break;
		case 0x40af: comp->vid->tsconf.t0xl = val; break;
		case 0x41af: comp->vid->tsconf.t0xh = val & 1; break;
		case 0x42af: comp->vid->tsconf.t0yl = val; break;
		case 0x43af: comp->vid->tsconf.t0yh = val & 1; break;
		case 0x44af: comp->vid->tsconf.t1xl = val; break;
		case 0x45af: comp->vid->tsconf.t1xh = val & 1; break;
		case 0x46af: comp->vid->tsconf.t1yl = val; break;
		case 0x47af: comp->vid->tsconf.t1yh = val & 1; break;
		default:
			switch (port & 0xff) {
				case 0x1f:
				case 0x3f:
				case 0x5f:
				case 0x7f:
					if (!comp->dosen) break;
					if (comp->tsconf.vdos) {
						comp->tsconf.vdos = 0;
						tslMapMem(comp);
					} else {
						if (comp->bdi->fdc->fptr->flag & FLP_VIRT) {
							comp->tsconf.vdos = 1;
							tslMapMem(comp);
//							if (((port & 0xff) == 0x1f) && ((val & 0xe0) == 0xa0)) comp->flag |= ZX_BREAK;
						} else {
							if ((port & 0xff) == 0x1f) bdiOut(comp->bdi,FDC_COM,val);
							if ((port & 0xff) == 0x3f) bdiOut(comp->bdi,FDC_TRK,val);
							if ((port & 0xff) == 0x5f) bdiOut(comp->bdi,FDC_SEC,val);
							if ((port & 0xff) == 0x7f) bdiOut(comp->bdi,FDC_DATA,val);
						}
					}
					break;
				case 0xff:
					if (!comp->dosen) break;
					comp->bdi->fdc->fptr = comp->bdi->fdc->flop[val & 3];
					if ((comp->bdi->fdc->fptr->flag & FLP_VIRT)) {
						comp->tsconf.vdos = 1;
						tslMapMem(comp);
					} else if (comp->tsconf.vdos) {
						// comp->bdi->fdc->fptr = comp->bdi->fdc->flop[val & 3];	// out VGSys[1:0]
					} else {
						bdiOut(comp->bdi,BDI_SYS,val);
					}
					break;
				case 0xf6:
				case 0xfe:
					comp->vid->brdcol = 0xf0 | (val & 7);
					comp->vid->nextbrd = comp->vid->brdcol;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0xfb: sdrvOut(comp->sdrv,0xfb,val); break;	// covox
				case 0xf7:
					if (~port & 0x1000) pEFF7 = val;
					if ((~port & 0x4000) && ((pEFF7 & 0x80) || comp->dosen)) cmsWr(comp,val);	// BFF7 : cmos data
					if (~port & 0x2000) comp->cmos.adr = val;				// DFF7 : cmos adr
					break;
				case 0xfd:
					if (((port & 0x8000) == 0x0000) && (~p7FFD & 0x20)) {			// 7ffd
						p7FFD = val;
						cnt = (val & 7) | ((val & 0xc0) >> 3);		// page (512K)
						if (p21AF & 0x80) {				// 1x : !a13
							if (~port & 0x2000) cnt &= 7;
						} else if (p21AF & 0x40) {			// 01 : 128
							cnt &= 7;
						}
						memSetBank(comp->mem,MEM_BANK3,MEM_RAM,cnt);
//						comp->tsconf.Page3 = cnt;			// !!!
						comp->vid->tsconf.vidPage = 5;
						comp->vid->curscr = (val & 8) ? 1 : 0;
						tslMapMem(comp);
					}
					if ((port & 0xc000) == 0xc000) tsOut(comp->ts,0xfffd,val);	// fffd
					if ((port & 0xc000) == 0x8000) tsOut(comp->ts,0xbffd,val);	// bffd
					break;
				case 0x57:
					sdcWrite(comp->sdc,val);
					break;
				case 0x77:
					comp->sdc->flag &= ~0x03;
					comp->sdc->flag |= (val & 3);
					break;
/*
				case 0x10:
					ideOut(comp->ide,comp->evo.hiTrig ? 0x11 : 0x10,val,0);
					comp->evo.hiTrig ^= 1;		// change hi-low trigger
					break;
				case 0x11:
					ideOut(comp->ide,0x11,val,0);
					comp->evo.hiTrig = 0;		// next byte is low
					break;
				case 0x30:
				case 0x50:
				case 0x70:
				case 0x90:
				case 0xB0:
				case 0xD0:
				case 0xF0:
				case 0xC8:
					ideOut(comp->ide,port & 0xf0,val,0);
					comp->evo.hiTrig = 1;		// next byte is hi (low for in)
					break;
*/
				default:
//					if (ideOut(comp->ide,port,val,0)) break;
//					if (gsOut(comp->gs,port,val) == GS_OK) break;
#ifdef ISDEBUG
					printf("TSLab : out %.4X,%.2X (%i)\n",port,val,bdiz);
					comp->flag |= ZX_BREAK;
//					assert(0);
#endif
					break;
			}
			break;
	}
}

Z80EX_BYTE tslIn(ZXComp* comp,Z80EX_WORD port,int bdiz) {
	Z80EX_BYTE res = 0xff;
//	printf("TSLab : in %.4X (%i)\n",port,bdiz);
	switch(port) {
		case 0x00af:
			res = comp->tsconf.pwr_up ? 0x40 : 0x00;		// b6: PWR_UP (1st run)
			comp->tsconf.pwr_up = 0;
			break;
		case 0x12af: res = comp->mem->pt[2]->num; break;		// comp->tsconf.Page2;
		case 0x13af: res = comp->mem->pt[3]->num; break;		// comp->tsconf.Page3;
		case 0x27af:
			res = 0;				// b7: DMA status
			break;
		case 0xfadf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->buttons : 0xff; break;
		case 0xfbdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->xpos : 0xff; break;
		case 0xffdf: res = (comp->mouse->flags & INF_ENABLED) ? comp->mouse->ypos : 0xff; break;
		default:
			switch (port & 0xff) {
/*
				case 0x1f:
					res = bdiz ? bdiIn(comp->bdi,FDC_STATE) : 0xe0;
					break;
				case 0x3f:
					res = bdiz ? bdiIn(comp->bdi,FDC_TRK) : 0xff;
					break;
				case 0x5f:
					res = bdiz ? bdiIn(comp->bdi,FDC_SEC) : 0xff;
					break;
				case 0x7f:
					res = bdiz ? bdiIn(comp->bdi,FDC_DATA) : 0xff;
					break;
*/
				case 0x1f:
				case 0x3f:
				case 0x5f:
				case 0x7f:
					if (comp->dosen) {
						if (comp->tsconf.vdos) {
							comp->tsconf.vdos = 0;
							tslMapMem(comp);
						} else {
							if (comp->bdi->fdc->fptr->flag & FLP_VIRT) {
								comp->tsconf.vdos = 1;
								tslMapMem(comp);
							} else {
								if ((port & 0xff) == 0x1f) res = bdiIn(comp->bdi,FDC_COM);
								if ((port & 0xff) == 0x3f) res = bdiIn(comp->bdi,FDC_TRK);
								if ((port & 0xff) == 0x5f) res = bdiIn(comp->bdi,FDC_SEC);
								if ((port & 0xff) == 0x7f) res = bdiIn(comp->bdi,FDC_DATA);
							}
						}
					} else {
						if ((port & 0xff) == 0x1f) res = 0x00;		// kempston joystick
					}
					break;
				case 0xff:
					if (!comp->dosen) break;
					if (comp->bdi->fdc->fptr->flag & FLP_VIRT) {
						comp->tsconf.vdos = 1;
						tslMapMem(comp);
					} else {
						res = bdiIn(comp->bdi,BDI_SYS);
					}
					break;
				case 0xf6:
				case 0xfe:
					res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
					break;
				case 0xf7:
					if ((~port & 0x4000) && ((pEFF7 & 0x80) || comp->dosen)) res = cmsRd(comp);	// BFF7
					break;
				case 0xfd:
					if ((port & 0xc000) == 0xc000) res = tsIn(comp->ts, 0xfffd);	// fffd
					break;
				case 0x57:
					res = sdcRead(comp->sdc);
					break;
				case 0x77:
					res = 0x00;					// 0x02 : rd only
					if (comp->sdc->image != NULL) res |= 0x01;	// inserted
					if (comp->sdc->flag & SDC_LOCK) res |= 0x02;
					break;
/*
				case 0x10:
					ideIn(comp->ide,comp->evo.hiTrig ? 0x10 : 0x11,&res,0);
					comp->evo.hiTrig ^= 1;
					break;
				case 0x11:
					ideIn(comp->ide,0x11,&res,0);
					comp->evo.hiTrig = 1;		// next byte is low
					break;
				case 0x30:
				case 0x50:
				case 0x70:
				case 0x90:
				case 0xB0:
				case 0xD0:
				case 0xF0:
				case 0xC8:
					ideIn(comp->ide,port & 0xf0,&res,0);
					comp->evo.hiTrig = 1;		// next byte is low
					break;
*/
				default:
//					if (ideIn(comp->ide,port,&res,comp->dosen & 1)) break;
//					if (gsIn(comp->gs,port,&res) == GS_OK) break;
#ifdef ISDEBUG
					printf("TSLab : in %.4X (%i)\n",port,bdiz);
					comp->flag |= ZX_BREAK;
//					assert(0);
#endif
					break;
			}
			break;
	}
	return res;
}
