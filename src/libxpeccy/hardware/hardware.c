#include "hardware.h"
#include <stdlib.h>
#include <string.h>

HardWare hwTab[] = {
	{
		"ZX48K","ZX 48K",
		HW_ZX48,
		MEM_48,
		&speMapMem,&speOut,&speIn,&stdMRd,&stdMWr,&speReset
	},{
		"Pentagon","Pentagon",
		HW_PENT,
		MEM_128 | MEM_512,
		&penMapMem,&penOut,&penIn,&stdMRd,&stdMWr,NULL
	},{
		"Pentagon1024SL","Pentagon 1024 SL",
		HW_P1024,
		MEM_1M,
		&p1mMapMem,&p1mOut,&p1mIn,&stdMRd,&stdMWr,NULL
	},{
		"PentEvo","Evo Baseconf",
		HW_PENTEVO,
		MEM_4M,
		&evoMapMem,&evoOut,&evoIn,&evoMRd,&evoMWr,&evoReset
	},{
		"TSLab","Evo TSConf",
		HW_TSLAB,
		MEM_4M,
		&tslMapMem,&tslOut,&tslIn,&tslMRd,&tslMWr,&tslReset
	},{
		"Scorpion","ZS Scorpion",
		HW_SCORP,
		MEM_256 | MEM_1M,
		&scoMapMem,&scoOut,&scoIn,&scoMRd,&stdMWr,NULL
	},{
		"ATM2","ATM Turbo 2+",
		HW_ATM2,
		MEM_128 | MEM_256 | MEM_512 | MEM_1M,
		&atm2MapMem,&atm2Out,&atm2In,&stdMRd,&stdMWr,&atm2Reset
	},{
		"Spectrum +2","Spectrum +2",
		HW_PLUS2,
		MEM_128,
		&pl2MapMem,&pl2Out,&pl2In,&stdMRd,&stdMWr,NULL
	},{
		"Spectrum +3","Spectrum +3",
		HW_PLUS3,
		MEM_128,
		&pl2MapMem,&pl3Out,&pl3In,&stdMRd,&stdMWr,NULL
	},{
		"Profi","Profi",
		HW_PROFI,
		MEM_512 | MEM_1M,
		&prfMapMem,&prfOut,&prfIn,&stdMRd,&stdMWr,&prfReset
	},
	{
		"MSX","MSX-1",
		HW_MSX,
		MEM_128,
		&msxMapMem,&msxOut,&msxIn,&stdMRd,&stdMWr,&msxReset
	},
	{NULL,NULL,0,0,NULL,NULL,NULL,NULL,NULL,NULL}
};

HardWare* findHardware(const char* name) {
	HardWare* hw = NULL;
	int idx = 0;
	while (hwTab[idx].name != NULL) {
		if (strcmp(hwTab[idx].name,name) == 0) {
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
	int ctch = 0;
	while (1) {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) && ((itm->dos & 2) || (itm->dos == dos)) && (itm->out != NULL)) {
			if ((itm->mask != 0) || (ctch == 0)) {
				itm->out(comp, port, val);
				ctch = 1;
			}
		}
		if (itm->mask == 0) break;
		idx++;
	}
}
