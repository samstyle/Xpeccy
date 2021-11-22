#include "hardware.h"
#include <stdlib.h>
#include <string.h>
// layout = v9938:342:313:16:13:57:80:64:0:0:256:192
// {{full},{border},{blank},{screen},{ipos},ilen}

static vLayout gbcLay = {{228,154},{0,0},{68,10},{160,144},{0,0},64};
static vLayout bkLay = {{256+96,256+24},{0,0},{96,24},{256,256},{0,0},0};
static vLayout spclstLay = {{384+16,256+8},{0,0},{16,8},{384,256},{0,0},0};
static vLayout nesPALLay = {{342,312},{0,0},{85,72},{256,240},{0,0},64};
static vLayout v9938Lay = {{342,313},{16,13},{57,80},{256,192},{0,0},64};
static vLayout cmdrLay = {{504,312},{42,42},{100,28},{320,200},{0,0},64};

HardWare hwTab[] = {
	{
		HW_DUMMY,HWG_NULL,"Dummy","Dummy",16,MEM_256,1.0,NULL,
		zx_init,hw_dum_map, hw_dum_iwr, hw_dum_ird, hw_dum_mrd, hw_dum_mwr, NULL, NULL, NULL, NULL, hw_dum_vol
	},{
		HW_ZX48,HWG_ZX,"ZX48K","ZX 48K",16,MEM_64K | MEM_16K,1.0,NULL,
		zx_init,speMapMem,speOut,speIn,stdMRd,stdMWr,zx48_reset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENT,HWG_ZX,"Pentagon","Pentagon",16,MEM_128K | MEM_512K,1.0,NULL,
		zx_init,penMapMem,penOut,penIn,stdMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_P1024,HWG_ZX,"Pentagon1024SL","Pentagon 1024 SL",16,MEM_1M,1.0,NULL,
		zx_init,p1mMapMem,p1mOut,p1mIn,stdMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_SCORP,HWG_ZX,"Scorpion","ZS Scorpion",16,MEM_256K | MEM_1M,1.0,NULL,
		zx_init,scoMapMem,scoOut,scoIn,scoMRd,stdMWr,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_ATM2,HWG_ZX,"ATM2","ATM Turbo 2+ (v7.10)",16,MEM_128K | MEM_256K | MEM_512K | MEM_1M,1.0,NULL,
		zx_init,atm2MapMem,atm2Out,atm2In,stdMRd,stdMWr,atm2Reset,atm2_sync,atm2_keyp,atm2_keyr,zx_vol
	},{
		HW_PROFI,HWG_ZX,"Profi","Profi",16,MEM_512K | MEM_1M,1.0,NULL,
		zx_init,prfMapMem,prfOut,prfIn,stdMRd,stdMWr,prfReset,zx_sync,prf_keyp,prf_keyr,zx_vol
	},{
		HW_PHOENIX,HWG_ZX,"Phoenix","ZXM Phoenix",16,MEM_2M,1.0,NULL,
		zx_init,phxMapMem,phxOut,phxIn,stdMRd,stdMWr,phxReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENTEVO,HWG_ZX,"PentEvo","Evo Baseconf",16,MEM_4M,1.0,NULL,
		zx_init,evoMapMem,evoOut,evoIn,evoMRd,evoMWr,evoReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_TSLAB,HWG_ZX,"TSLab","Evo TSConf",16,MEM_4M,1.0,NULL,
		zx_init,tslMapMem,tslOut,tslIn,tslMRd,tslMWr,tslReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_PLUS2,HWG_ZX,"Spectrum +2","Spectrum +2",16,MEM_128K,1.0,NULL,
		zx_init,pl2MapMem,pl2Out,pl2In,stdMRd,stdMWr,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PLUS3,HWG_ZX,"Spectrum +3","Spectrum +3",16,MEM_128K,1.0,NULL,
		zx_init,pl2MapMem,pl3Out,pl3In,stdMRd,stdMWr,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_MSX,HWG_MSX,"MSX","MSX-1",16,MEM_128K,1.0,&v9938Lay,
		msx_init,msxMapMem,msxOut,msxIn,stdMRd,stdMWr,msxReset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_MSX2,HWG_MSX,"MSX2","MSX-2 (alfa)",16,MEM_128K,1.0,&v9938Lay,
		msx2_init,msx2mapper,msx2Out,msx2In,msx2mrd,msx2mwr,msx2Reset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_GBC,HWG_GB,"GameBoy","Game Boy",16,MEM_64K,1.0,&gbcLay,
		gbc_init,gbMaper,NULL,NULL,gbMemRd,gbMemWr,gbReset,gbcSync,gbc_keyp,gbc_keyr,gbc_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_NES,HWG_NES,"NES","NES",16,MEM_64K,(double)8/7,&nesPALLay,
		nes_init,nesMaper,NULL,NULL,nesMemRd,nesMemWr,nesReset,nesSync,nes_keyp,nes_keyr,nes_vol
	},{
		HW_C64,HWG_COMMODORE,"Commodore64","Commodore64",16,MEM_128K,1.0,&cmdrLay,
		c64_init,c64_maper,NULL,NULL,c64_mrd,c64_mwr,c64_reset,c64_sync,c64_keyp,c64_keyr,c64_vol
//	},{
//		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_BK0010,HWG_BK,"BK0010","BK0010",8,MEM_32K,(double)29/23,&bkLay,
		bk_init,bk_mem_map,bk_iowr,NULL,bk_mrd,bk_mwr,bk_reset,bk_sync,bk_keyp,bk_keyr,bk_vol
	},{
//		HW_BK0011M,HWG_BK,"BK0011M","BK0011M",8,MEM_128K,(double)29/23,&bkLay,
//		bk_init,bk_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_reset,bk_sync,bk_keyp,bk_keyr,bk_vol
//	},{
		HW_SPCLST,HWG_SPCLST,"Specualist","Specialist",16,MEM_64K,1.0,&spclstLay,
		spc_init,spc_mem_map,NULL,NULL,spc_mrd,spc_mwr,spc_reset,spc_sync,spc_keyp,spc_keyr,spc_vol
#if USEIBM
	},{
		HW_IBM_PC,HWG_PC,"IBM PC","IBM PC",16,MEM_1M,1.0,NULL,
		ibm_init,ibm_mem_map,ibm_iowr,ibm_iord,ibm_mrd,ibm_mwr,ibm_reset,ibm_sync,ibm_keyp,ibm_keyr,ibm_vol
#endif
	},{
		HW_NULL,HWG_NULL,NULL,NULL,16,0,1.0,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// eot
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

// mem

static MemPage* pg;
extern int res4;
static int wns;

void zx_cont_mem(Computer* comp) {
	if ((pg->type == MEM_RAM) && (pg->num & 0x40)) {
		vidSync(comp->vid, comp->nsPerTick * (comp->cpu->t - res4));	// before
		res4 = comp->cpu->t;
		wns = vid_wait(comp->vid);
		while (wns > 0) {
			comp->cpu->t++;
			wns -= comp->nsPerTick;
		}
		vidSync(comp->vid, (comp->cpu->t - res4) * comp->nsPerTick);
		res4 = comp->cpu->t;
	}
}

int stdMRd(Computer* comp, int adr, int m1) {
	pg = &comp->mem->map[(adr >> 8) & 0xff];
	if (m1 && (comp->dif->type == DIF_BDI)) {
		if (comp->dos && (pg->type == MEM_RAM)) {
			comp->dos = 0;
			comp->hw->mapMem(comp);
		}
		if (!comp->dos && ((adr & 0x3f00) == 0x3d00) && comp->rom && (pg->type == MEM_ROM)) {
			comp->dos = 1;
			comp->hw->mapMem(comp);
		}
	}
	if (comp->contMem)
		zx_cont_mem(comp);
	return memRd(comp->mem, adr & 0xffff) & 0xff;
}

void stdMWr(Computer *comp, int adr, int val) {
	pg = &comp->mem->map[(adr >> 8) & 0xff];
	if (comp->contMem)
		zx_cont_mem(comp);
	memWr(comp->mem,adr,val);
}

// io
// TODO: contended io here or in spectrum.c? Z80: add 4T after io cycle?

int hwIn(xPort* ptab, Computer* comp, int port, int dos) {
	int res = 0xff;
	int idx = 0;
	xPort* itm;
	do {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) &&\
				(itm->in != NULL) &&\
				((itm->dos & 2) || (itm->dos == dos)) &&\
				((itm->rom & 2) || (itm->rom == comp->rom)) &&\
				((itm->cpm & 2) || (itm->cpm == comp->cpm))) {
			res = itm->in(comp, port);
			break;
		}
		idx++;
	} while (itm->mask != 0);
	return res;
}

void hwOut(xPort* ptab, Computer* comp, int port, int val, int dos, int mult) {
	int idx = 0;
	int catch = 0;
	xPort* itm;
	do {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) &&\
				(itm->out != NULL) &&\
				((itm->dos & 2) || (itm->dos == dos)) &&\
				((itm->rom & 2) || (itm->rom == comp->rom)) &&\
				((itm->cpm & 2) || (itm->cpm == comp->cpm))) {
			itm->out(comp, port, val);
			catch |= !mult;
		}
		idx++;
	} while ((itm->mask != 0) && !catch);
}
