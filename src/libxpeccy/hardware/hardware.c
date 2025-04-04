#include "hardware.h"
#include <stdlib.h>
#include <string.h>
// layout = v9938:342:313:16:13:57:80:64:0:0:256:192
// {{full},{border},{blank},{screen},{ipos},ilen}

static vLayout gbcLay = {{228,154},{0,0},{68,10},{160,144},{0,0},64};
static vLayout bkLay = {{256+96,256+24},{0,0},{96,24},{256,256},{0,0},0};
static vLayout spclstLay = {{384+16,256+8},{0,0},{16,8},{384,256},{0,0},0};
static vLayout nesPALLay = {{342,312},{0,0},{85,72},{256,240},{0,0},64};	// 342x312
static vLayout v9938Lay = {{342,313},{16,13},{57,80},{256,192},{0,0},64};
static vLayout cmdrLay = {{512,312},{24,30},{144,44},{320,200},{0,0},64};
static vLayout ibmLay = {{720,492},{0,0},{80,12},{640,480},{0,0},1};

// pent
xPortDsc zx_port_tab_a[] = {
	{0x7ffd, REG_BYTE, offsetof(Computer, p7FFD)},
	{-1, 0, 0}
};

// pent1024
xPortDsc zx_port_tab_b[] = {
	{0x7ffd, REG_BYTE, offsetof(Computer, p7FFD)},
	{0xeff7, REG_BYTE, offsetof(Computer, pEFF7)},
	{-1, 0, 0}
};

// scorp
xPortDsc zx_port_tab_s[] = {
	{0x7ffd, REG_BYTE, offsetof(Computer, p7FFD)},
	{0x1ffd, REG_BYTE, offsetof(Computer, p1FFD)},
	{-1, 0, 0}
};

// phoenix

xPortDsc zx_port_tab_px[] = {
	{0x7ffd, REG_BYTE, offsetof(Computer, p7FFD)},
	{0xeff7, REG_BYTE, offsetof(Computer, pEFF7)},
	{0x1ffd, REG_BYTE, offsetof(Computer, p1FFD)},
	{-1, 0, 0}
};

// profi
xPortDsc zx_port_tab_p[] = {
	{0x7ffd, REG_BYTE, offsetof(Computer, p7FFD)},
	{0xdffd, REG_BYTE, offsetof(Computer, pDFFD)},
	{-1, 0, 0}
};

// tsconf

xPortDsc zx_port_tab_ts[] = {
	{0x7ffd, REG_BYTE, offsetof(Computer, p7FFD)},
	{0xeff7, REG_BYTE, offsetof(Computer, pEFF7)},
	{0x01af, REG_BYTE, offsetof(Computer, tsconf.p01af)},
	{0x02af, REG_BYTE, offsetof(Computer, tsconf.p02af)},
	{0x03af, REG_BYTE, offsetof(Computer, tsconf.p03af)},
	{0x04af, REG_BYTE, offsetof(Computer, tsconf.p04af)},
	{0x05af, REG_BYTE, offsetof(Computer, tsconf.p05af)},
	{0x21af, REG_BYTE, offsetof(Computer, tsconf.p21af)},
	{-1, 0, 0}
};

HardWare hwTab[] = {
	{
		HW_DUMMY,HWG_NULL,"Dummy","Dummy",16,MEM_256,1.0,NULL,16,NULL,
		zx_init,hw_dum_map, hw_dum_iwr, hw_dum_ird, hw_dum_mrd, hw_dum_mwr, NULL, NULL, NULL, NULL, NULL, NULL, hw_dum_vol
	},{
		HW_ZX48,HWG_ZX,"ZX48K","ZX 48K",16,MEM_64K | MEM_16K,1.0,NULL,16,NULL,
		zx_init,speMapMem,speOut,speIn,stdMRd,stdMWr,zx_irq,zx_ack,zx48_reset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENT,HWG_ZX,"Pentagon","Pentagon",16,MEM_128K | MEM_512K,1.0,NULL,16,zx_port_tab_a,
		zx_init,penMapMem,penOut,penIn,stdMRd,stdMWr,zx_irq,zx_ack,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_P1024,HWG_ZX,"Pentagon1024SL","Pentagon 1024 SL",16,MEM_1M,1.0,NULL,16,zx_port_tab_b,
		zx_init,p1mMapMem,p1mOut,p1mIn,stdMRd,stdMWr,zx_irq,zx_ack,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_SCORP,HWG_ZX,"Scorpion","ZS Scorpion",16,MEM_256K | MEM_1M,1.0,NULL,16,zx_port_tab_s,
		zx_init,scoMapMem,scoOut,scoIn,scoMRd,stdMWr,zx_irq,zx_ack,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_ATM2,HWG_ZX,"ATM2","ATM Turbo 2+ (v7.10)",16,MEM_128K | MEM_256K | MEM_512K | MEM_1M,1.0,NULL,16,zx_port_tab_b,
		zx_init,atm2MapMem,atm2Out,atm2In,stdMRd,stdMWr,zx_irq,zx_ack,atm2Reset,atm2_sync,atm2_keyp,atm2_keyr,zx_vol
	},{
		HW_PROFI,HWG_ZX,"Profi","Profi",16,MEM_512K | MEM_1M,1.0,NULL,16,zx_port_tab_p,
		zx_init,prfMapMem,prfOut,prfIn,stdMRd,stdMWr,zx_irq,zx_ack,prfReset,zx_sync,prf_keyp,prf_keyr,zx_vol
	},{
		HW_PHOENIX,HWG_ZX,"Phoenix","ZXM Phoenix",16,MEM_2M,1.0,NULL,16,zx_port_tab_px,
		zx_init,phxMapMem,phxOut,phxIn,stdMRd,stdMWr,zx_irq,zx_ack,phxReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PENTEVO,HWG_ZX,"PentEvo","Evo Baseconf",16,MEM_4M,1.0,NULL,16,zx_port_tab_b,
		zx_init,evoMapMem,evoOut,evoIn,evoMRd,evoMWr,zx_irq,zx_ack,evoReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_TSLAB,HWG_ZX,"TSLab","Evo TSConf",16,MEM_4M,1.0,NULL,16,zx_port_tab_ts,
		zx_init,tslMapMem,tslOut,tslIn,tslMRd,tslMWr,zx_irq,zx_ack,tslReset,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,16,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_PLUS2,HWG_ZX,"Spectrum +2","Spectrum +2",16,MEM_128K,1.0,NULL,16,zx_port_tab_s,
		zx_init,pl2MapMem,pl2Out,pl2In,stdMRd,stdMWr,zx_irq,zx_ack,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_PLUS3,HWG_ZX,"Spectrum +3","Spectrum +3",16,MEM_128K,1.0,NULL,16,zx_port_tab_s,
		zx_init,pl2MapMem,pl3Out,pl3In,stdMRd,stdMWr,zx_irq,zx_ack,plusRes,zx_sync,zx_keyp,zx_keyr,zx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,16,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_MSX,HWG_MSX,"MSX","MSX-1",16,MEM_128K,1.0,&v9938Lay,16,NULL,
		msx_init,msxMapMem,msxOut,msxIn,stdMRd,stdMWr,NULL,NULL,msxReset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_MSX2,HWG_MSX,"MSX2","MSX-2 (alfa)",16,MEM_128K,1.0,&v9938Lay,16,NULL,
		msx2_init,msx2mapper,msx2Out,msx2In,msx2mrd,msx2mwr,NULL,NULL,msx2Reset,msx_sync,msx_keyp,msx_keyr,msx_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,16,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_GBC,HWG_GB,"GameBoy","Game Boy",16,MEM_64K,1.0,&gbcLay,16,NULL,
		gbc_init,gbMaper,NULL,NULL,gbMemRd,gbMemWr,gbc_irq,NULL,gbReset,gbcSync,gbc_keyp,gbc_keyr,gbc_vol
	},{
		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,16,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_NES,HWG_NES,"NES","NES",16,MEM_64K,(double)8/7,&nesPALLay,16,NULL,
		nes_init,nesMaper,NULL,NULL,nesMemRd,nesMemWr,nes_irq,NULL,nesReset,nesSync,nes_keyp,nes_keyr,nes_vol
	},{
		HW_C64,HWG_COMMODORE,"Commodore64","Commodore64",16,MEM_128K,1.0,&cmdrLay,16,NULL,
		c64_init,c64_maper,NULL,NULL,c64_mrd,c64_mwr,c64_irq,NULL,c64_reset,c64_sync,c64_keyp,c64_keyr,c64_vol
//	},{
//		HW_NULL,HWG_NULL,"","",16,0,1.0,NULL,16,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// separator
	},{
		HW_BK0010,HWG_BK,"BK0010","BK0010",8,MEM_32K,(double)29/23,&bkLay,16,NULL,
		bk_init,bk_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_irq,NULL,bk_reset,bk_sync,bk_keyp,bk_keyr,bk_vol
	},{
		HW_BK0011M,HWG_BK,"BK0011M","BK0011M",8,MEM_128K,(double)29/23,&bkLay,16,NULL,
		bk_init,bk11_mem_map,NULL,NULL,bk_mrd,bk_mwr,bk_irq,NULL,bk11_reset,bk_sync,bk_keyp,bk_keyr,bk_vol
	},{
		HW_SPCLST,HWG_SPCLST,"Specualist","Specialist",16,MEM_64K,1.0,&spclstLay,16,NULL,
		spc_init,spc_mem_map,NULL,NULL,spc_mrd,spc_mwr,NULL,NULL,spc_reset,spc_sync,spc_keyp,spc_keyr,spc_vol
	},{
		HW_IBM_PC,HWG_PC,"IBM PC","IBM PC",16,MEM_1M | MEM_2M | MEM_4M,1.0,&ibmLay,24,NULL,
		ibm_init,ibm_mem_map,ibm_iowr,ibm_iord,ibm_mrd,ibm_mwr,ibm_irq,ibm_ack,ibm_reset,ibm_sync,ibm_keyp,ibm_keyr,ibm_vol
	},{
		HW_NULL,HWG_NULL,NULL,NULL,16,0,1.0,NULL,16,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL		// eot
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
	if (pg->type == MEM_RAM) {
		vid_sync(comp->vid, comp->nsPerTick * (comp->cpu->t - res4));	// before
		res4 = comp->cpu->t;
		wns = vid_wait(comp->vid, pg->num << 8);
		comp->cpu->t += wns / comp->nsPerTick;
		vid_sync(comp->vid, (comp->cpu->t - res4) * comp->nsPerTick);
		res4 = comp->cpu->t;
	}
}

int stdMRd(Computer* comp, int adr, int m1) {
	pg = mem_get_page(comp->mem, adr);	// = &comp->mem->map[(adr >> 8) & 0xff];
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
#if !Z80_NEW_RW_CYCLE
	if (comp->contMem)
		zx_cont_mem(comp);
#endif
	return memRd(comp->mem, adr & 0xffff) & 0xff;
}

void stdMWr(Computer *comp, int adr, int val) {
	pg = mem_get_page(comp->mem, adr);	// = &comp->mem->map[(adr >> 8) & 0xff];
#if !Z80_NEW_RW_CYCLE
	if (comp->contMem)
		zx_cont_mem(comp);
#endif
	memWr(comp->mem,adr,val);
}

// io
// TODO: contended io here or in spectrum.c? Z80: add 4T after io cycle?

int hwIn(xPort* ptab, Computer* comp, int port) {
	int res = 0xff;
	int idx = 0;
	xPort* itm;
	do {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) &&\
				(itm->in != NULL) &&\
				((itm->dos & 2) || (itm->dos == comp->bdiz)) &&\
				((itm->rom & 2) || (itm->rom == comp->rom)) &&\
				((itm->cpm & 2) || (itm->cpm == comp->cpm))) {
			res = itm->in(comp, port);
			break;
		}
		idx++;
	} while (itm->mask != 0);
	return res;
}

void hwOut(xPort* ptab, Computer* comp, int port, int val, int mult) {
	int idx = 0;
	int catch = 0;
	xPort* itm;
	do {
		itm = &ptab[idx];
		if (((port & itm->mask) == (itm->value & itm->mask)) &&\
				(itm->out != NULL) &&\
				((itm->dos & 2) || (itm->dos == comp->bdiz)) &&\
				((itm->rom & 2) || (itm->rom == comp->rom)) &&\
				((itm->cpm & 2) || (itm->cpm == comp->cpm))) {
			itm->out(comp, port, val);
			catch |= !mult;
		}
		idx++;
	} while ((itm->mask != 0) && !catch);
}

// max 32 ports
xPortValue pvTab[33];

xPortValue* hwGetPorts(Computer* comp) {
	int i = 0;
	xPortDsc* tab = comp->hw->portab;
	if (tab) {
		void* ptr;
		while ((tab[i].port > 0) && (i < 33)) {
			pvTab[i].port = tab[i].port;
			if (tab[i].offset) {
				ptr = ((void*)comp) + tab[i].offset;
				switch(tab[i].type) {
					case REG_BYTE: pvTab[i].value = *((unsigned char*)ptr) & 0xff; break;
					case REG_WORD: pvTab[i].value = *((unsigned short*)ptr) & 0xffff; break;
					case REG_32: pvTab[i].value = *((unsigned int*)ptr); break;
					default: pvTab[i].value = *((unsigned char*)ptr) & 0xff; break;
				}
			} else if (comp->hw->in) {
				pvTab[i].value = comp->hw->in(comp, pvTab[i].port);
			}
			i++;
		}
	}
	pvTab[i].port = -1;
	return pvTab;
}
