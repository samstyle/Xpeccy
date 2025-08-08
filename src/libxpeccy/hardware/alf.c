#include "hardware.h"

#define regRomN	reg[0x5f]

int alf_sltrd(int adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	int res = -1;
	if (comp->slot) {
		adr = (adr & 0x3fff) | ((comp->regRomN & 0x3f) << 14);		// full address
		if (comp->slot->data && (adr <= comp->slot->memMask)) {				// not loaded = 0xff
			res = comp->slot->data[adr];
		}
	}
	return res;
}

void alf_mapmem(Computer* comp) {
	if (comp->rom) {		// switch to rom1 after snapshot loading
		comp->rom = 0;
		comp->regRomN = 1;
	}
	if (comp->regRomN & 0x80) {
		memSetBank(comp->mem, 0x00, MEM_SLOT, comp->regRomN & 0x3f, MEM_16K, alf_sltrd, NULL, comp);	// cartrige data
	} else {
		memSetBank(comp->mem, 0x00, MEM_ROM, comp->regRomN & 0x3f, MEM_16K, NULL, NULL, NULL);		// std rom
	}
	memSetBank(comp->mem, 0x40, MEM_RAM, 5, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0x80, MEM_RAM, 2, MEM_16K, NULL, NULL, NULL);
	memSetBank(comp->mem, 0xc0, MEM_RAM, comp->p7FFD & 7, MEM_16K, NULL, NULL, NULL);
}

void alf_reset(Computer* comp) {
	comp->intVector = 0xff;
	comp->vid->curscr = 5;
	vid_set_mode(comp->vid, VID_NORMAL);
	comp->regRomN = 0x00;
	comp->p7FFD = 0x00;
	comp->rom = 0;
	alf_mapmem(comp);
}

// 1F rd:kempston
int alf_in1F(Computer* comp, int adr) {
	int res = comp->joy->state;
	if (!comp->joy->extbuttons) res &= 0x1f;
	res ^= 0xa0;
	return res;
}

// 5F wr:rom page
void alf_out5F(Computer* comp, int adr, int data) {
	comp->regRomN = data & 0xff;
	alf_mapmem(comp);
}

// FE wr: border/sound
void alf_outFE(Computer* comp, int adr, int data) {
	comp->vid->nextbrd = (data & 0x07);
	comp->beep->lev = !!(data & 0x10);
}

// FE rd: 2nd joystick
int alf_inFE(Computer* comp, int adr) {
	return comp->joyb->state ^ 0x1f;		// invert 5 bits
}

void alf_out7FFD(Computer* comp, int adr, int data) {
	if (comp->mem->ramSize == MEM_64K) return;		// 48K
	comp->p7FFD = data & 7;
	comp->vid->curscr = (data & 0x08) ? 7 : 5;
	alf_mapmem(comp);
}

static xPort alf_port_map[] = {
	{0x0081,0x00fe,2,2,2,alf_inFE,	alf_outFE},
	{0x0083,0x001f,2,2,2,alf_in1F,	alf_out5F},
	{0xc002,0x7ffd,2,2,2,NULL,	alf_out7FFD},
	{0xc002,0xbffd,2,2,2,NULL,	xOutBFFD},
	{0xc002,0xfffd,2,2,2,xInFFFD,	xOutFFFD},
	{0x0000,0x0000,2,2,2,NULL,	NULL}
};

int alf_ird(Computer* comp, int adr) {
	return hwIn(alf_port_map, comp, adr);
}

void alf_iwr(Computer* comp, int adr, int data) {
	hwOut(alf_port_map, comp, adr, data, 1);
}

int alf_mrd(Computer* comp, int adr, int m1) {
	return memRd(comp->mem, adr);
}

void alf_mwr(Computer* comp, int adr, int data) {
	memWr(comp->mem, adr, data);
}

void alf_sync(Computer* comp, int ns) {
	bcSync(comp->beep, ns);
	tsSync(comp->ts, ns);
}

sndPair alf_vol(Computer* comp, sndVolume* sv) {
	sndPair p;
	sndPair v;
	p.left = comp->beep->val * sv->beep / 6;
	p.right = p.left;
	v = tsGetVolume(comp->ts);
	p.left += v.left * sv->ay / 100;
	p.right += v.right * sv->ay / 100;
	return p;
}
