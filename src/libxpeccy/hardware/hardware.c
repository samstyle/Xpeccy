#include "hardware.h"
#include <stdlib.h>
#include <string.h>

HardWare hwTab[] = {
	{
		HW_DUMMY,HWG_NULL,"Dummy","Dummy",16,MEM_64K,1.0,
		hw_dum_map, hw_dum_iwr, hw_dum_ird, hw_dum_mrd, hw_dum_mwr, NULL, NULL, NULL, NULL, hw_dum_vol
	},{
		HW_ZX48,HWG_ZX,"ZX48K","ZX 48K",16,MEM_64K,1.0,
		speMapMem,speOut,speIn,stdMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENT,HWG_ZX,"Pentagon","Pentagon",16,MEM_128K | MEM_512K,1.0,
		penMapMem,penOut,penIn,stdMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_P1024,HWG_ZX,"Pentagon1024SL","Pentagon 1024 SL",16,MEM_1M,1.0,
		p1mMapMem,p1mOut,p1mIn,stdMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_SCORP,HWG_ZX,"Scorpion","ZS Scorpion",16,MEM_256K | MEM_1M,1.0,
		scoMapMem,scoOut,scoIn,scoMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_ATM2,HWG_ZX,"ATM2","ATM Turbo 2+",16,MEM_128K | MEM_256K | MEM_512K | MEM_1M,1.0,
		atm2MapMem,atm2Out,atm2In,stdMRd,stdMWr,atm2Reset,zx_sync,atm2_keyp,atm2_keyr,zx_vol
	},{
		HW_PROFI,HWG_ZX,"Profi","Profi",16,MEM_512K | MEM_1M,1.0,
		prfMapMem,prfOut,prfIn,stdMRd,stdMWr,prfReset,zx_sync,prf_keyp,prf_keyr,zx_vol
	},{
		HW_PHOENIX,HWG_ZX,"Phoenix","ZXM Phoenix",16,MEM_2M,1.0,
		phxMapMem,phxOut,phxIn,stdMRd,stdMWr,phxReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENTEVO,HWG_ZX,"PentEvo","Evo Baseconf",16,MEM_4M,1.0,
		evoMapMem,evoOut,evoIn,evoMRd,evoMWr,evoReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_TSLAB,HWG_ZX,"TSLab","Evo TSConf",16,MEM_4M,1.0,
		tslMapMem,tslOut,tslIn,tslMRd,tslMWr,tslReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_PLUS2,HWG_ZX,"Spectrum +2","Spectrum +2",16,MEM_128K,1.0,
		pl2MapMem,pl2Out,pl2In,stdMRd,stdMWr,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PLUS3,HWG_ZX,"Spectrum +3","Spectrum +3",16,MEM_128K,1.0,
		pl2MapMem,pl3Out,pl3In,stdMRd,stdMWr,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_MSX,HWG_MSX,"MSX","MSX-1",16,MEM_128K,1.0,
		msxMapMem,msxOut,msxIn,stdMRd,stdMWr,msxReset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_MSX2,HWG_MSX,"MSX2","MSX-2 (alfa)",16,MEM_128K,1.0,
		msx2mapper,msx2Out,msx2In,msx2mrd,msx2mwr,msx2Reset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_GBC,HWG_GB,"GameBoy","Game Boy",16,MEM_64K,1.0,
		gbMaper,NULL,NULL,gbMemRd,gbMemWr,gbReset,gbcSync,gbc_keyp,gbc_keyr,gbc_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_NES,HWG_NES,"NES","NES",16,MEM_64K,(double)8/7,
		nesMaper,NULL,NULL,nesMemRd,nesMemWr,nesReset,nesSync,nes_keyp,nes_keyr,nes_vol
	},{
		HW_C64,HWG_COMMODORE,"Commodore64","Commodore64",16,MEM_128K,1.0,
		c64_maper,NULL,NULL,c64_mrd,c64_mwr,c64_reset,c64_sync,c64_keyp,c64_keyr,c64_vol
//	},{
//		HW_NULL,HWG_NULL,"","",16,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_BK0010,HWG_BK,"BK0010","BK0010",8,MEM_32K,(double)29/23,
		bk_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_reset,bk_sync,bk_keyp,bk_keyr,bk_vol
	},{
//		HW_BK0011M,HWG_BK,"BK0011M","BK0011M",8,MEM_128K,
//		bk_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_reset,bk_sync,bk_keyp,bk_keyr,bk_vol
//	},{
		HW_NULL,HWG_NULL,NULL,NULL,16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// eot
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
	while (ptab[idx].mask != 0) {
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
	}
	return res;
}

void hwOut(xPort* ptab, Computer* comp, unsigned short port, unsigned char val, int dos) {
	int idx = 0;
	xPort* itm;
	while (ptab[idx].mask != 0) {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) &&\
				(itm->out != NULL) &&\
				((itm->dos & 2) || (itm->dos == dos)) &&\
				((itm->rom & 2) || (itm->rom == comp->rom)) &&\
				((itm->cpm & 2) || (itm->cpm == comp->cpm))) {
			itm->out(comp, port, val);
//			break;		// write to every matched port
		}
		if (itm->mask == 0) break;
		idx++;
	}
}
