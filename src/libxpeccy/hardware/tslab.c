#include "../spectrum.h"

#include <stdio.h>
#include <assert.h>

void tslMapMem(ZXComp* comp) {
}

int tslRes[8] = {256,192,320,200,320,240,360,288};

void tslOut(ZXComp* comp,Z80EX_WORD port,Z80EX_BYTE val,int bdiz) {
	if ((port & 0x01) == 0x00) {
		comp->vid->brdcol = val & 7;
		comp->beeplev = val & 0x10;
		comp->tape->outsig = (val & 0x08) ? 1 : 0;
	}
	int cnt,sadr,dadr;
	switch (port) {
		case 0x00af:
			comp->vid->tsconf.xSize = tslRes[(val & 0xc0) >> 5];
			comp->vid->tsconf.ySize = tslRes[((val & 0xc0) >> 5) + 1];
			printf("vmode %.2X\n",val & 7);
			switch(val & 7) {
				case 0: vidSetMode(comp->vid,VID_NORMAL); break;
				case 1: vidSetMode(comp->vid,VID_TSL_16); break;
				case 2: vidSetMode(comp->vid,VID_TSL_256); break;
				case 3: break;			// 1 page text mode
				default: vidSetMode(comp->vid,VID_UNKNOWN); break;
			}
			break;
		case 0x01af:
			comp->vid->tsconf.vidPage = val;
			break;
		case 0x07af:
			comp->tsconf.tsScrPal = val & 0x0f;
			break;
		case 0x10af:			// Page0
			comp->memMap[0].page = val;
			memSetBank(comp->mem,MEM_BANK0,MEM_RAM,val);
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
		case 0x1aaf:
			comp->dma.src.l = val;
			break;
		case 0x1baf:
			comp->dma.src.h = val;
			break;
		case 0x1caf:
			comp->dma.src.x = val;
			break;
		case 0x1daf:
			comp->dma.dst.l = val;
			break;
		case 0x1eaf:
			comp->dma.dst.h = val;
			break;
		case 0x1faf:
			comp->dma.dst.x = val;
			break;
		case 0x20af:
			switch (val & 3) {
				case 0: zxSetFrq(comp,3.5); break;
				case 1: zxSetFrq(comp,7.0); break;
				case 2: zxSetFrq(comp,14.0); break;	// normal
				case 3: zxSetFrq(comp,14.0); break;	// overclock
			}

			break;
		case 0x21af:
			comp->prt2 = val;
			switch (val & 0x0c) {
				case 0x00: memSetBank(comp->mem,MEM_BANK0,MEM_ROM,(comp->dosen ? 0 : 2) | (val & 1)); break;
				case 0x04: memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0); break;
				case 0x08: memSetBank(comp->mem,MEM_BANK0,MEM_RAM,comp->memMap[0].page); break;
				case 0x0c: memSetBank(comp->mem,MEM_BANK0,MEM_RAM,0); break;		// ?
			}

			tslMapMem(comp);
			break;
		case 0x26af:
			if (val == 0) val = 1;
			comp->dma.len = val;
			break;
		case 0x27af:
			switch (val & 0x87) {
				case 0x01:		// ram->ram
					sadr = (comp->dma.src.x << 14) | ((comp->dma.src.h & 0x3f) << 8) | (comp->dma.src.l & 0xfe);
					dadr = (comp->dma.dst.x << 14) | ((comp->dma.dst.h & 0x3f) << 8) | (comp->dma.dst.l & 0xfe);
					printf("src: %.4X,%.2X\n",sadr & 0x3fff,comp->dma.src.x);
					printf("dst: %.4X,%.2X\n",dadr & 0x3fff,comp->dma.dst.x);
					printf("len: %.4X bytes\n",comp->dma.len << 1);
					printf("num: %.2X\n",comp->dma.num);
					printf("ctr: %.2X\n",val);
					while(comp->dma.num) {
						for(cnt = 0; cnt < comp->dma.len; cnt++) {
							comp->mem->ram[dadr >> 14].data[dadr & 0x3fff] = comp->mem->ram[sadr >> 14].data[sadr & 0x3fff];
							comp->mem->ram[dadr >> 14].data[(dadr & 0x3fff) + 1] = comp->mem->ram[sadr >> 14].data[(sadr & 0x3fff) + 1];
							dadr += 2;
							sadr += 2;
						}
						if (val & 0x20)		// S_ALGN
							sadr = sadr - (comp->dma.len << 1) + ((val & 0x08) ? 0x200 : 0x100);
						if (val & 0x10)		// D_ALGN
							dadr = dadr - (comp->dma.len << 1) + ((val & 0x08) ? 0x200 : 0x100);

						comp->dma.num--;
					}
					comp->dma.src.x = ((sadr & 0x3fc000) >> 14);
					comp->dma.src.h = ((sadr & 0x3f00) >> 8);
					comp->dma.src.l = sadr & 0xff;
					comp->dma.dst.x = ((dadr & 0x3fc000) >> 14);
					comp->dma.dst.h = ((dadr & 0x3f00) >> 8);
					comp->dma.dst.l = dadr & 0xff;
					break;
				case 0x81: break;	// nothing
				default:
					printf("0x27: unsupported src-dst: %.2X\n",val & 0x87);
					assert(0);
			}
			break;
		case 0x28af:
			if (val == 0) val = 1;
			comp->dma.num = val;
			break;
		case 0x29af:
			comp->tsconf.FDDVirt = val;
			break;
		case 0xeff7:
			comp->prt1 = val;
			break;
		case 0xbff7:			// cmos.data
			if (comp->prt1 & 0x80) comp->cmos.data[comp->cmos.adr] = val;
			break;
		case 0xdff7:			// cmos.adr
			if (comp->prt1 & 0x80) comp->cmos.adr = val;
			break;
		default:
			if ((port & 0x01) == 0x00) break;
			printf("TSLab : out %.4X,%.2X\n",port,val);
			//comp->flag |= ZX_BREAK;
			assert(0);
	}
}

Z80EX_BYTE tslIn(ZXComp* comp,Z80EX_WORD port,int bdiz) {
	Z80EX_BYTE res = 0xff;
	if ((port & 0x01) == 0x00)
		return keyInput(comp->keyb, (port & 0xff00) >> 8) | (comp->tape->signal ? 0x40 : 0x00);
	switch(port) {
		case 0x00af:
			res = 0;		// b6: PWR_UP
			break;
		case 0x27af:
			res = 0;		// b7: DMA status
			break;
		case 0xbff7:
			res = (comp->prt1 & 0x80) ? comp->cmos.data[comp->cmos.adr] : 0xff;
			break;
		default:
			printf("TSLab : in %.4X\n",port);
			//comp->flag |= ZX_BREAK;
			assert(0);
	}
	return res;
}
