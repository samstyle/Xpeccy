#include "../spectrum.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define p7FFD comp->prt0
#define pEFF7 comp->prt1
#define p21AF comp->prt2

void tslMapMem(ZXComp* comp) {
// bank0 maping taken from Unreal(TSConf)
	if (p21AF & 8)
		if (p21AF & 4)
			memSetBank(comp->mem,MEM_BANK0,MEM_RAM,comp->tsconf.Page0);
		else
			memSetBank(comp->mem,MEM_BANK0,MEM_RAM, comp->tsconf.Page0 + (((comp->dosen) ? 0 : 2) | ((p7FFD & 0x10) ? 0 : 1)));
	else
		if (p21AF & 4)
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM,comp->tsconf.Page0 & 3);
		else
			memSetBank(comp->mem,MEM_BANK0,MEM_ROM, ((p7FFD & 0x10) ? 1 : 0) | (comp->dosen ? 0 : 2));
}

int tslXRes[4] = {256,320,320,360};
int tslYRes[4] = {192,200,240,288};

void tslOut(ZXComp* comp,Z80EX_WORD port,Z80EX_BYTE val,int bdiz) {
	if ((port & 0x0001) == 0x0000) port |= 0x00fe;		// make XXFE
	if ((port & 0x80ff) == 0x00fd) port = 0x7ffd;		// ???
	int cnt,lcnt,sadr,dadr;
	switch (port) {
		case 0x00af:
			comp->vid->tsconf.xSize = tslXRes[(val & 0xc0) >> 6];
			comp->vid->tsconf.ySize = tslYRes[(val & 0xc0) >> 6];
			comp->vid->tsconf.xPos = comp->vid->bord.h - ((comp->vid->tsconf.xSize - 256) >> 1);
			comp->vid->tsconf.yPos = comp->vid->bord.v - ((comp->vid->tsconf.ySize - 192) >> 1);
			switch(val & 7) {
				case 0: vidSetMode(comp->vid,VID_TSL_NORMAL); break;
				case 1: vidSetMode(comp->vid,VID_TSL_16); break;
				case 2: vidSetMode(comp->vid,VID_TSL_256); break;
				case 3: vidSetMode(comp->vid,VID_TSL_TEXT); break;
				default: vidSetMode(comp->vid,VID_UNKNOWN); break;
			}
			comp->vid->flags &= ~VID_NOGFX;
			if (val & 0x20) comp->vid->flags |= VID_NOGFX;
			break;
		case 0x01af: comp->vid->tsconf.vidPage = val; break;
		case 0x02af: comp->vid->tsconf.soxl = val; break;
		case 0x03af: comp->vid->tsconf.soxh = val; break;
		case 0x04af: comp->vid->tsconf.soyl = val; break;
		case 0x05af: comp->vid->tsconf.soyh = val; break;
		case 0x06af: comp->vid->tsconf.tconfig = val; break;
		case 0x07af:
			comp->vid->tsconf.scrPal = (val & 0x0f) << 4;
			comp->vid->tsconf.T0Pal76 = (val & 0x30) << 2;
			comp->vid->tsconf.T1Pal76 = (val & 0xc0);
			break;
		case 0x0faf: comp->vid->nextbrd = val; break;
		case 0x10af:			// Page0
			comp->tsconf.Page0 = val;
			// memSetBank(comp->mem,MEM_BANK0,MEM_RAM,val);
			tslMapMem(comp);
			break;
		case 0x11af:			// Page1
			comp->tsconf.Page1 = val;
			memSetBank(comp->mem,MEM_BANK1,MEM_RAM,val);
			break;
		case 0x12af:			// Page2
			comp->tsconf.Page2 = val;
			memSetBank(comp->mem,MEM_BANK2,MEM_RAM,val);
			break;
		case 0x13af:			// Page3
			comp->tsconf.Page3 = val;
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,val);
			break;
		case 0x15af:
			comp->tsconf.flag = val & 0x10;		// FM_EN
			comp->tsconf.tsMapAdr = ((val & 0x0f) << 12);
			break;
		case 0x16af: comp->vid->tsconf.TMPage = val & 0xf8; break;
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
			tslMapMem(comp);
			break;
		case 0x22af:
			printf("out 22AF,%.2X\n",val);
			break;
		case 0x23af:					// VSINTL
			comp->vid->intpos.v &= 0xff00;
			comp->vid->intpos.v |= val;
			//printf("out 23AF,%.2X\n",val);
			break;
		case 0x24af:					// VSINTH
			comp->vid->intpos.v &= 0x00ff;
			comp->vid->intpos.v |= ((val & 1) << 8);
			//printf("out 24AF,%.2X\n",val);
			break;
		case 0x25af:
			printf("out 25AF,%.2X\n",val);
			break;
		case 0x26af:
			comp->dma.len = val;
			break;
		case 0x27af:
			switch (val & 0x87) {
				case 0x01:		// ram->ram
					sadr = (comp->dma.src.x << 14) | ((comp->dma.src.h & 0x3f) << 8) | (comp->dma.src.l & 0xfe);
					dadr = (comp->dma.dst.x << 14) | ((comp->dma.dst.h & 0x3f) << 8) | (comp->dma.dst.l & 0xfe);
					lcnt = (comp->dma.len + 1) << 1;
//					printf("\nsrc : %.2X %.2X %.2X (%X)\n",comp->dma.src.x,comp->dma.src.h,comp->dma.src.l,sadr);
//					printf("dst : %.2X %.2X %.2X (%X)\n",comp->dma.dst.x,comp->dma.dst.h,comp->dma.dst.l,dadr);
//					printf("len : %.2X (%i bytes)\n",comp->dma.len,lcnt);
//					printf("num : %.2X\n",comp->dma.num);

					for (cnt = 0; cnt <= comp->dma.num; cnt++) {
						memcpy(comp->mem->ramData + dadr, comp->mem->ramData + sadr,lcnt);	// ?? will crush @ end of mem
						sadr += (val & 0x20) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// SALGN
						dadr += (val & 0x10) ? ((val & 0x08) ? 0x200 : 0x100) : lcnt;		// DALGN
					}
					//comp->dma.src.x = ((sadr & 0x3fc000) >> 14);
					//comp->dma.src.h = ((sadr & 0x3f00) >> 8);
					//comp->dma.src.l = sadr & 0xff;
					//comp->dma.dst.x = ((dadr & 0x3fc000) >> 14);
					//comp->dma.dst.h = ((dadr & 0x3f00) >> 8);
					//comp->dma.dst.l = dadr & 0xff;
					break;
				case 0x81: break;	// nothing
				default:
					printf("0x27AF: unsupported src-dst: %.2X\n",val & 0x87);
					assert(0);
			}
			break;
		case 0x28af:
			comp->dma.num = val;
			break;
		case 0x29af:
			comp->tsconf.FDDVirt = val;
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
		case 0x7ffd:
			if (p7FFD & 0x20) break;
			p7FFD = val;
			memSetBank(comp->mem,MEM_BANK3,MEM_RAM,val & 7);		// TODO: highmem block mode
			comp->vid->curscr = (val & 8) ? 1 : 0;
			tslMapMem(comp);
			break;
		case 0xeff7:
			pEFF7 = val;
			break;
		case 0xbff7:			// cmos.data
			if (pEFF7 & 0x80) comp->cmos.data[comp->cmos.adr] = val;
			if (comp->cmos.adr == 0xf0) {
				comp->keyb->flags &= ~INF_PCKEY;
				if (val & 2) comp->keyb->flags |= INF_PCKEY;
			}
			// if (comp->cmos.adr > 0xef) printf("write to cmos %.2X,%.2X\n",comp->cmos.adr,val);
			break;
		case 0xdff7:			// cmos.adr
			if (pEFF7 & 0x80) comp->cmos.adr = val;
			break;
		case 0xbffd:
		case 0xbefd:
		case 0xfffd:
			tsOut(comp->ts,port | 0x0100,val);
			break;
		default:
			switch (port & 0xff) {
				case 0x1f:
					if (bdiz) bdiOut(comp->bdi,FDC_COM,val);
					break;
				case 0x3f:
					if (bdiz) bdiOut(comp->bdi,FDC_TRK,val);
					break;
				case 0x5f:
					if (bdiz) bdiOut(comp->bdi,FDC_SEC,val);
					break;
				case 0x7f:
					if (bdiz) bdiOut(comp->bdi,FDC_DATA,val);
					break;
				case 0xff:
					if (bdiz) bdiOut(comp->bdi,BDI_SYS,val);
					break;
				case 0xfe:
					comp->vid->brdcol = 0xf0 | (val & 7);
					comp->vid->nextbrd = comp->vid->brdcol;
					comp->beeplev = val & 0x10;
					comp->tape->outsig = (val & 0x08) ? 1 : 0;
					break;
				case 0xf7:			// WAS IST DAS?
					break;
				default:
					if ((port & 0x0001) == 0x0000) break;
					printf("TSLab : out %.4X,%.2X (%i)\n",port,val,bdiz);
					//comp->flag |= ZX_BREAK;
					assert(0);
			}
			break;
	}
}

Z80EX_BYTE tslIn(ZXComp* comp,Z80EX_WORD port,int bdiz) {
	Z80EX_BYTE res = 0xff;
//	printf("TSLab : in %.4X (%i)\n",port,bdiz);
	if ((port & 0x0001) == 0x0000) port |= 0x00fe;
	switch(port) {
		case 0x00af:
			res = 0;		// b6: PWR_UP
			break;
		case 0x12af:
			res = comp->tsconf.Page2;
			break;
		case 0x13af:
			res = comp->tsconf.Page3;
			break;
		case 0x27af:
			res = 0;		// b7: DMA status
			break;
		case 0xbff7:
			switch (comp->cmos.adr) {
				case 0xf0:
					res = keyReadCode(comp->keyb);
					break;
				default:
					res = (pEFF7 & 0x80) ? comp->cmos.data[comp->cmos.adr] : 0xff;
					break;
			}
//			res = (pEFF7 & 0x80) ? comp->cmos.data[comp->cmos.adr] : 0xff;
			break;
		case 0xfffd:
			res = tsIn(comp->ts,port);
			break;
		default:
			switch (port & 0xff) {
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
				case 0xff:
					res = bdiz ? bdiIn(comp->bdi,BDI_SYS) : 0xff;
					break;
				case 0xfe:
					res = keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
					break;
				case 0xf6: res = 0xff; break;			// WAS IST DAS?
				case 0xf7: res = 0xff; break;			// WAS IST DAS?
				default:
					printf("TSLab : in %.4X (%i)\n",port,bdiz);
					//comp->flag |= ZX_BREAK;
					assert(0);
			}
			break;
	}
	return res;
}
