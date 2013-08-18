#include "hardware.h"
#include <stdlib.h>
#include <string.h>

HardWare hwTab[] = {
	{
		"ZX48K","ZX 48K",
		HW_ZX48,
		MEM_48,
		&speMapMem,&speOut,&speIn,&stdMRd
	},{
		"Pentagon","Pentagon",
		HW_PENT,
		MEM_128 | MEM_512,
		&penMapMem,&penOut,&penIn,&stdMRd
	},{
		"Pentagon1024SL","Pentagon 1024 SL",
		HW_P1024,
		MEM_1M,
		&p1mMapMem,&p1mOut,&p1mIn,&stdMRd
	},{
		"PentEvo","Evo Baseconf",
		HW_PENTEVO,
		MEM_4M,
		&evoMapMem,&evoOut,&evoIn,&evoMRd
	},{
		"TSLab","Evo TSConf",
		HW_TSLAB,
		MEM_4M,
		&tslMapMem,&tslOut,&tslIn,&tslMRd
	},{
		"Scorpion","ZS Scorpion",
		HW_SCORP,
		MEM_256 | MEM_1M,
		&scoMapMem,&scoOut,&scoIn,&scoMRd
	},{
		"ATM2","ATM Turbo 2+",
		HW_ATM2,
		MEM_128 | MEM_256 | MEM_512 | MEM_1M,
		&atm2MapMem,&atm2Out,&atm2In,&stdMRd //&atm2MRd
	},{
		"Spectrum +2","Spectrum +2",
		HW_PLUS2,
		MEM_128,
		&pl2MapMem,&pl2Out,&pl2In,&stdMRd
	},{
		"Spectrum +3","Spectrum +3",
		HW_PLUS3,
		MEM_128,
		&pl2MapMem,&pl3Out,&pl3In,&stdMRd
	},
	{NULL,NULL,0,0,NULL,NULL,NULL}
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

Z80EX_BYTE stdMRd(ZXComp* comp, Z80EX_WORD adr, int m1) {
	if (m1 && (comp->bdi->fdc->type == FDC_93)) {
		mbt = memGetBankPtr(comp->mem,adr)->type;
		if (comp->dosen && (mbt == MEM_RAM)) {
			comp->dosen = 0;
			comp->hw->mapMem(comp);
		}
		if (!comp->dosen && ((adr & 0x3f00) == 0x3d00) && (comp->prt0 & 0x10) && (mbt == MEM_ROM)) {
			comp->dosen = 1;
			comp->hw->mapMem(comp);
		}
	}
	return memRd(comp->mem,adr);
}
