#include "hardware.h"
#include <stdlib.h>
#include <string.h>

HardWare hwTab[] = {
	{
		HW_ZX48,"ZX48K","ZX 48K",50,MEM_48K,
		speMapMem,speOut,speIn,stdMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENT,"Pentagon","Pentagon",50,MEM_128K | MEM_512K,
		penMapMem,penOut,penIn,stdMRd,stdMWr,NULL,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_P1024,"Pentagon1024SL","Pentagon 1024 SL",50,MEM_1M,
		p1mMapMem,p1mOut,p1mIn,stdMRd,stdMWr,NULL,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_SCORP,"Scorpion","ZS Scorpion",50,MEM_256K | MEM_1M,
		scoMapMem,scoOut,scoIn,scoMRd,stdMWr,NULL,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_ATM2,"ATM2","ATM Turbo 2+",50,MEM_128K | MEM_256K | MEM_512K | MEM_1M,
		atm2MapMem,atm2Out,atm2In,stdMRd,stdMWr,atm2Reset,zx_sync,atm2_keyp,atm2_keyr,zx_vol
	},{
		HW_PROFI,"Profi","Profi",50,MEM_512K | MEM_1M,
		prfMapMem,prfOut,prfIn,stdMRd,stdMWr,prfReset,zx_sync,prf_keyp,prf_keyr,zx_vol
	},{
		HW_PHOENIX,"Phoenix","ZXM Phoenix",50,MEM_2M,
		phxMapMem,phxOut,phxIn,stdMRd,stdMWr,phxReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENTEVO,"PentEvo","Evo Baseconf",50,MEM_4M,
		evoMapMem,evoOut,evoIn,evoMRd,evoMWr,evoReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_TSLAB,"TSLab","Evo TSConf",50,MEM_4M,
		tslMapMem,tslOut,tslIn,tslMRd,tslMWr,tslReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_PLUS2,"Spectrum +2","Spectrum +2",50,MEM_128K,
		pl2MapMem,pl2Out,pl2In,stdMRd,stdMWr,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PLUS3,"Spectrum +3","Spectrum +3",50,MEM_128K,
		pl2MapMem,pl3Out,pl3In,stdMRd,stdMWr,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_MSX,"MSX","MSX-1",60,MEM_128K,
		msxMapMem,msxOut,msxIn,stdMRd,stdMWr,msxReset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_MSX2,"MSX2","MSX-2 (alfa)",60,MEM_128K,
		msx2mapper,msx2Out,msx2In,msx2mrd,msx2mwr,msx2Reset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_GBC,"GameBoy","Game Boy",60,MEM_64K,
		gbMaper,NULL,NULL,gbMemRd,gbMemWr,gbReset,gbcSync,gbc_keyp,gbc_keyr,gbc_vol
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_NES,"NES","NES",50,MEM_64K,
		nesMaper,NULL,NULL,nesMemRd,nesMemWr,nesReset,nesSync,nes_keyp,nes_keyr,nes_vol
	},{
		HW_C64,"Commodore64","Commodore64",50,MEM_128K,
		c64_maper,NULL,NULL,c64_mrd,c64_mwr,c64_reset,c64_sync,c64_keyp,c64_keyr,c64_vol
	},{
		HW_NULL,"","",50,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_BK0010, "BK0010","BK0010",50,MEM_64K,
		bk_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_reset,bk_sync,bk_keyp,bk_keyr,bk_vol
	},{
		HW_NULL,NULL,NULL,50,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// eot
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
		mbt = comp->mem->map[(adr >> 8) & 0xff].type;
		if (comp->dos && (mbt == MEM_RAM)) {
			comp->dos = 0;
			comp->hw->mapMem(comp);
		}
		if (!comp->dos && ((adr & 0x3f00) == 0x3d00) && comp->rom && (mbt == MEM_ROM)) {
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
