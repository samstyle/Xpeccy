#include "../spectrum.h"

void penMapMem(Computer* comp) {
	int pg = (comp->flgDOS ? 2 : 0) | ((comp->flgROM) ? 1 : 0);
	memSetBank(comp->mem, 0x00, MEM_ROM, pg, MEM_16K, NULL, NULL, NULL);
	pg = (comp->p7FFD & 7) | ((comp->p7FFD & 0xc0) >> 3);
	memSetBank(comp->mem, 0x40, MEM_RAM, 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_RAM, 2, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xc0, MEM_RAM, pg, MEM_16K, NULL, NULL, NULL);
}

// in

int penInFF(Computer* comp, int port) {
	return (comp->vid->vbrd || comp->vid->hbrd) ? 0xff : comp->vid->atrbyte & 0xff;
}

// out

void penOut7FFD(Computer* comp, int port, int val) {
	if (comp->p7FFD & 0x20) return;
	comp->flgROM = (val & 0x10) ? 1 : 0;
	comp->p7FFD = val & 0xff;
	comp->vid->curscr = (val & 0x08) ? 7 : 5;
	penMapMem(comp);
}

static xPort penPortMap[] = {
	{0x0001,0x00fe,2,2,2,xInFE,	xOutFE},
	{0x8002,0x7ffd,2,2,2,NULL,	penOut7FFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x00ff,0x001f,0,2,2,xIn1F,	NULL},		// joystick
	{0x05a3,0xfadf,0,2,2,xInFADF,	NULL},		// mouse
	{0x05a3,0xfbdf,0,2,2,xInFBDF,	NULL},
	{0x05a3,0xffdf,0,2,2,xInFFDF,	NULL},
	{0x0000,0x0000,2,2,2,penInFF,	NULL}
};

void penOut(Computer* comp, int port, int val) {
	difOut(comp->dif, port, val, comp->flgBDI);
	zx_dev_wr(comp, port, val);
	hwOut(penPortMap, comp, port, val, 1);
}

int penIn(Computer* comp, int port) {
	int res = -1;
	if (difIn(comp->dif, port, &res, comp->flgBDI)) return res;
	if (zx_dev_rd(comp, port, &res)) return res;
	res = hwIn(penPortMap, comp, port);
	return res;
}

// pent
xPortDsc zx_port_tab_a[] = {
	{0x7ffd, REG_BYTE, offsetof(Computer, p7FFD)},
	{-1, 0, 0}
};

HardWare pnt_hw_core = {HW_PENT,HWG_ZX,"Pentagon","Pentagon",16,MEM_128K | MEM_512K,1.0,NULL,16,zx_port_tab_a,
			zx_init,penMapMem,penOut,penIn,stdMRd,stdMWr,zx_irq,zx_ack,speReset,zx_sync,zx_keyp,zx_keyr,zx_vol};
