#include "hardware.h"
#include <stdlib.h>
#include <string.h>

HardWare hwTab[] = {
	{
		HW_ZX48,"ZX48K","ZX 48K",50,MEM_48,
		&speMapMem,&speOut,&speIn,&stdMRd,&stdMWr,&speReset,&zx_sync
	},{
		HW_PENT,"Pentagon","Pentagon",50,MEM_128 | MEM_512,
		&penMapMem,&penOut,&penIn,&stdMRd,&stdMWr,NULL,&zx_sync
	},{
		HW_P1024,"Pentagon1024SL","Pentagon 1024 SL",50,MEM_1M,
		&p1mMapMem,&p1mOut,&p1mIn,&stdMRd,&stdMWr,NULL,&zx_sync
	},{
		HW_SCORP,"Scorpion","ZS Scorpion",50,MEM_256 | MEM_1M,
		&scoMapMem,&scoOut,&scoIn,&scoMRd,&stdMWr,NULL,&zx_sync
	},{
		HW_ATM2,"ATM2","ATM Turbo 2+",50,MEM_128 | MEM_256 | MEM_512 | MEM_1M,
		&atm2MapMem,&atm2Out,&atm2In,&stdMRd,&stdMWr,&atm2Reset,&zx_sync
	},{
		HW_PROFI,"Profi","Profi",50,MEM_512 | MEM_1M,
		&prfMapMem,&prfOut,&prfIn,&stdMRd,&stdMWr,&prfReset,&zx_sync
	},{
		HW_PHOENIX,"Phoenix","ZXM Phoenix",50,MEM_2M,
		&phxMapMem,&phxOut,&phxIn,&stdMRd,&stdMWr,&phxReset,&zx_sync
	},{
		HW_PENTEVO,"PentEvo","Evo Baseconf",50,MEM_4M,
		&evoMapMem,&evoOut,&evoIn,&evoMRd,&evoMWr,&evoReset,&zx_sync
	},{
		HW_TSLAB,"TSLab","Evo TSConf",50,MEM_4M,
		&tslMapMem,&tslOut,&tslIn,&tslMRd,&tslMWr,&tslReset,&zx_sync
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL			// separator
	},{
		HW_PLUS2,"Spectrum +2","Spectrum +2",50,MEM_128,
		&pl2MapMem,&pl2Out,&pl2In,&stdMRd,&stdMWr,NULL,&zx_sync
	},{
		HW_PLUS3,"Spectrum +3","Spectrum +3",50,MEM_128,
		&pl2MapMem,&pl3Out,&pl3In,&stdMRd,&stdMWr,NULL,&zx_sync
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL			// separator
	},{
		HW_MSX,"MSX","MSX-1",60,MEM_128,
		&msxMapMem,&msxOut,&msxIn,&stdMRd,&stdMWr,&msxReset,&msx_sync
	},{
		HW_MSX2,"MSX2","MSX-2 (alfa)",60,MEM_128,
		&msx2mapper,&msx2Out,&msx2In,&msx2mrd,&msx2mwr,&msx2Reset,&msx_sync
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL			// separator
	},{
		HW_GBC,"GameBoy","Game Boy",60,MEM_48,
		&gbMaper,NULL,NULL,&gbMemRd,&gbMemWr,&gbReset,&gbcSync
	},{
		HW_NULL,NULL,NULL,50,0,NULL,NULL,NULL,NULL,NULL,NULL			// eot
	}
};

HardWare* findHardware(const char* name) {
	HardWare* hw = NULL;
	int idx = 0;
	while (hwTab[idx].name != NULL) {
		if ((hwTab[idx].id != HW_NULL) && !strcmp(hwTab[idx].name,name)) {
			hw = &hwTab[idx];
			break;
		}
		idx++;
	}
	return hw;
}

unsigned char mbt;

// mem

unsigned char stdMRd(Computer* comp, unsigned short adr, int m1) {
	if (m1 && (comp->dif->type == DIF_BDI)) {
		mbt = memGetBankPtr(comp->mem,adr)->type;
		if (comp->dos && (mbt == MEM_RAM)) {
			comp->dos = 0;
			comp->hw->mapMem(comp);
		}
		if (!comp->dos && ((adr & 0x3f00) == 0x3d00) && (comp->rom) && (mbt == MEM_ROM)) {
			comp->dos = 1;
			comp->hw->mapMem(comp);
		}
	}
	return memRd(comp->mem,adr);
}

void stdMWr(Computer *comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem,adr,val);
}

// io

unsigned char hwIn(xPort* ptab, Computer* comp, unsigned short port, int dos) {
	unsigned char res = 0xff;
	int idx = 0;
	xPort* itm;
	while (1) {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) &&\
				(itm->in != NULL) &&\
				((itm->dos & 2) || (itm->dos == dos)) &&\
				((itm->rom & 2) || (itm->rom == comp->rom)) &&\
				((itm->cpm & 2) || (itm->cpm == comp->cpm))) {
			res = itm->in(comp, port);
			break;
		}
		if (itm->mask == 0) break;
		idx++;
	};
	return res;
}

void hwOut(xPort* ptab, Computer* comp, unsigned short port, unsigned char val, int dos) {
	int idx = 0;
	xPort* itm;
	while (1) {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) &&\
				(itm->out != NULL) &&\
				((itm->dos & 2) || (itm->dos == dos)) &&\
				((itm->rom & 2) || (itm->rom == comp->rom)) &&\
				((itm->cpm & 2) || (itm->cpm == comp->cpm))) {
			itm->out(comp, port, val);
			break;
		}
		if (itm->mask == 0) break;
		idx++;
	}
}
