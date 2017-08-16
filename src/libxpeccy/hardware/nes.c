#include "hardware.h"

#include <stdio.h>

// NOTE: NES is fukkeen trash. why did i start this?
// NOTE: Cartridges have potentially 255+ mappers

// PPU reads byte (except palette)
unsigned char nes_ppu_ext_rd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	unsigned char res;
	if (adr & 0x2000) {	// nametable
		adr &= comp->slot->ntmask;
		adr |= comp->slot->ntorsk;
		res = comp->vid->ppu->mem[adr];
	} else {
		if (comp->slot->chrrom) {
			res = sltRead(comp->slot, SLT_CHR, adr);
		} else {
			res = comp->vid->ppu->mem[adr];
		}
	}
	return res;
}

// PPU writes byte (except palette)
void nes_ppu_ext_wr(unsigned short adr, unsigned char val, void* ptr) {
	Computer* comp = (Computer*)ptr;
	if (adr & 0x2000) {
		adr &= comp->slot->ntmask;
		adr |= comp->slot->ntorsk;
		comp->vid->ppu->mem[adr] = val;
	} else {
		if (comp->slot->chrrom) {
			// CHR-RAM?
		} else {
			comp->vid->ppu->mem[adr & 0x1fff] = val;
		}
	}
}

// APU reads byte (DMC channel)
unsigned char nes_apu_ext_rd(unsigned short adr, void* ptr) {
	Computer* comp = (Computer*)ptr;
	return comp->hw->mrd(comp, adr, 0);
}

void nesReset(Computer* comp) {
	comp->vid->ppu->mrd = nes_ppu_ext_rd;
	comp->vid->ppu->mwr = nes_ppu_ext_wr;
	comp->vid->ppu->data = comp;
	ppuReset(comp->vid->ppu);
	vidSetMode(comp->vid, VID_NES);

	comp->slot->memMap[0] = 0;	// prg 4x8K pages
	comp->slot->memMap[1] = 1;
	comp->slot->memMap[2] = comp->slot->prglast - 1;
	comp->slot->memMap[3] = comp->slot->prglast;
	for (int i = 0; i < 8; i++)	// chr 8x1K pages
		comp->slot->chrMap[i] = i;
}

unsigned char nesMMrd(unsigned short adr, void* data) {
	Computer* comp = (Computer*)data;
	unsigned char* mem = comp->mem->ramData;
	unsigned char res = 0xff;
	adr &= 0x7fff;
	if (adr < 0x2000) {
		res = mem[adr & 0x7ff];
	} else if (adr < 0x4000) {
		adr &= 7;
		nesPPU* ppu = comp->vid->ppu;
		switch(adr) {
			case 2:
				// b7:vblank, b6:spr0 hit, b5:spr overflow
				res = 0x1f;
				if (comp->vid->vblank) res |= 0x80;
				if (comp->vid->ppu->sp0hit) res |= 0x40;
				if (comp->vid->ppu->spover) res |= 0x20;
				ppu->latch = 0;
				break;
			case 4:
				res = ppu->oam[ppu->oamadr & 0xff];
				break;
			case 7:
				res = ppuRead(ppu);
				break;
		}
	} else if (adr < 0x5000) {
		// audio, dma, io registers (0x17, other is unused)
		nesAPU* apu = comp->nesapu;
		switch (adr) {
			case 0x4015:
				res = 0;
				if (apu->ch0.len) res |= 0x01;
				if (apu->ch1.len) res |= 0x02;
				if (apu->cht.len) res |= 0x04;
				if (apu->chn.len) res |= 0x08;
				if (apu->chd.len) res |= 0x10;
				if (apu->frm) res |= 0x40;
				break;
			case 0x4016:		// joystick 1
				res = comp->nes.priJoy & 1;
				comp->nes.priJoy >>= 1;
				break;
			case 0x4017:		// joystick 2
				res = comp->nes.secJoy & 1;
				comp->nes.secJoy >>= 1;
				break;

		}
	} else if (adr < 0x6000) {
		// expansion rom (4K)
	} else if (comp->slot->data) {
		// cartrige ram (8K)
		res = sltRead(comp->slot, SLT_RAM, adr);
	}
	return res;
}

// taked from nesdev wiki
// convert to apu ticks = div 16
static int apuPcmPer[16] = {428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106,  84,  72,  54};
static int apuLenPAL[8] = {5,10,20,40,80,30,7,13};
static int apuLenNTSC[8] = {6,12,24,48,96,36,8,16};
static int apuLenGeneral[16] = {127,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

int apuGetLC(unsigned char val) {
	int len;
	switch (val & 0x88) {
		case 0x00: len = apuLenPAL[(val >> 4) & 7]; break;
		case 0x80: len = apuLenNTSC[(val >> 4) & 7]; break;
		default: len = apuLenGeneral[(val >> 4) & 15]; break;
	}
	return len;
}

// write @ page 4000..7fff
// 4000..4fff : IO block
// 5000..5fff : ?
// 6000..7fff : cartrige ram
void nesMMwr(unsigned short adr, unsigned char val, void* data) {
	Computer* comp = (Computer*)data;
	adr &= 0x7fff;
	if (adr < 0x2000) {
		comp->mem->ramData[adr & 0x7ff] = val;
	} else if (adr < 0x4000) {
		// write video registers
		nesPPU* ppu = comp->vid->ppu;
		switch (adr & 7) {
			case 0:		// PPUCTRL
				ppu->vastep = (val & 0x04) ? 1 : 0;
				ppu->spadr = (val & 0x08) ? 0x1000 : 0x0000;
				ppu->bgadr = (val & 0x10) ? 0x1000 : 0x0000;
				ppu->bigspr = (val & 0x20) ? 1 : 0;
				ppu->master = (val & 0x40) ? 1 : 0;
				ppu->inten = (val & 0x80) ? 1 : 0;

				ppu->tadr = (ppu->tadr & ~0x0c00) | ((val << 10) & 0x0c00);

				break;
			case 1:		// PPUMASK
				ppu->greyscale = (val & 0x01) ? 1 : 0;
				ppu->bgleft8 = (val & 0x02) ? 1 : 0;
				ppu->spleft8 = (val & 0x04) ? 1 : 0;
				ppu->bgen = (val & 0x08) ? 1 : 0;
				ppu->spen = (val & 0x10) ? 1 : 0;
				// TODO: b5,6,7 = color tint
				break;
			case 3:
				ppu->oamadr = val;
				break;
			case 4:
				ppu->oam[ppu->oamadr & 0xff] = val;
				ppu->oamadr++;
				break;
			case 5:
				if (ppu->latch) {
					ppu->tadr &= 0x0c1f;
					ppu->tadr |= ((val & 0x07) << 12);
					ppu->tadr |= ((val & 0xf8) << 2);
					ppu->latch = 0;
				} else {
					ppu->tadr &= ~0x001f;
					ppu->tadr |= ((val >> 3) & 0x1f);
					ppu->finex = val & 7;
					ppu->latch = 1;
				}
				break;
			case 6:
				if (ppu->latch) {
					ppu->tadr &= 0xff00;
					ppu->tadr |= (val & 0xff);
					ppu->vadr = ppu->tadr;			// VADR changes right now
					ppu->latch = 0;
				} else {
					ppu->tadr &= 0x00ff;
					ppu->tadr |= ((val << 8) & 0x3f00);
					ppu->latch = 1;
				}
				break;
			case 7:
				ppuWrite(ppu, val);
				break;
		}
	} else if (adr < 0x5000) {
		// write audio/dma/io?
		nesAPU* apu = comp->nesapu;
		switch (adr) {
			// ch0 : tone 0
			case 0x4000:
				apu->ch0.duty = (val >> 6) & 3;
				apu->ch0.env = (val & 0x10) ? 0 : 1;
				apu->ch0.elen = (val & 0x20) ? 0 : 1;
				if (apu->ch0.env) {
					//printf("ch0 env %.2X\n",val);
					apu->ch0.eper = ((val & 15) + 1);
					apu->ch0.ecnt = apu->ch0.eper;
					apu->ch0.vol = 0x0f;
				} else {
					apu->ch0.vol = val & 15;
				}
				apuToneDuty(&apu->ch0);
				break;
			case 0x4001:
//				printf("ch0 sweep %.2X\n",val);
				apu->ch0.sweep = (val & 0x80) ? 1 : 0;
				apu->ch0.sper = ((val >> 4) & 7) + 1;
				apu->ch0.sdir = (val & 0x08) ? 1 : 0;
				apu->ch0.sshi = val & 7;
				break;
			case 0x4002:
				apu->ch0.hper &= 0x700;
				apu->ch0.hper |= (val & 0xff);
				apuToneDuty(&apu->ch0);
				break;
			case 0x4003:
				//printf("wr 4003,%.2X\n",val);
				apu->ch0.hper &= 0xff;
				apu->ch0.hper |= ((val << 8) & 0x0700);
				switch (val & 0x88) {
					case 0x00: apu->ch0.len = apuLenPAL[(val >> 4) & 7]; break;
					case 0x80: apu->ch0.len = apuLenNTSC[(val >> 4) & 7]; break;
					default: apu->ch0.len = apuLenGeneral[(val >> 4) & 15]; break;
				}
				apuToneDuty(&apu->ch0);
				//printf("CH0 len = %i, p0 = %i, p1 = %i\n", apu->ch0.len, apu->ch0.per0, apu->ch0.per1);
				break;
			// ch1 : tone 1
			case 0x4004:
				apu->ch1.duty = (val >> 6) & 3;
				apu->ch1.env = (val & 0x10) ? 0 : 1;
				apu->ch1.elen = (val & 0x20) ? 0 : 1;
				if (apu->ch1.env) {
					apu->ch1.eper = ((val & 15) + 1);
					apu->ch1.ecnt = apu->ch1.eper;
					apu->ch1.vol = 0x0f;
				} else {
					apu->ch1.vol = val & 15;
				}
				apuToneDuty(&apu->ch1);
				break;
			case 0x4005:
				apu->ch1.sweep = (val & 0x80) ? 1 : 0;
				apu->ch1.sper = ((val >> 4) & 7) + 1;
				apu->ch1.sdir = (val & 0x08) ? 1 : 0;
				apu->ch1.sshi = val & 7;
				break;
			case 0x4006:
				apu->ch1.hper &= 0x700;
				apu->ch1.hper |= val;
				apuToneDuty(&apu->ch1);
				break;
			case 0x4007:
				apu->ch1.hper &= 0xff;
				apu->ch1.hper |= ((val << 8) & 0x0700);
				apu->ch1.len = apuGetLC(val);
				apuToneDuty(&apu->ch1);
				break;
			// ch2 : triangle
			case 0x4008:
				apu->cht.elen = (val & 0x80) ? 0 : 1;
				apu->cht.lcnt = (val & 0x7f);
				break;
			case 0x4009:
				break;
			case 0x400a:					// tone period divided by 2 cuz triangle clock is CPU/32
				apu->cht.hper &= 0x700;
				apu->cht.hper |= val & 0xff;
				break;
			case 0x400b:
				apu->cht.hper &= 0xff;
				apu->cht.hper |= (val << 8) & 0x70;
				apu->cht.len = apuGetLC(val);
				apu->cht.vol = 8;
				apu->cht.dir = 1;
				break;
			// ch3 : noise
			case 0x400c:
				apu->chn.elen = (val & 0x20) ? 0 : 1;
				apu->chn.env = (val & 0x10) ? 0 : 1;
				if (apu->chn.env) {
					apu->chn.eper = (val & 15) + 1;
					apu->chn.ecnt = apu->chn.eper;
					apu->chn.vol = 0x0f;
				} else {
					apu->chn.vol = val & 15;
				}
				break;
			case 0x400d:
				break;
			case 0x400e:
				apu->chn.hper = val & 0x0f;
				// b7:loop noise (short generator)
				break;
			case 0x400f:
				apu->chn.len = apuGetLC(val);
				break;
			// ch4 : dmc
			case 0x4010:
				apu->chd.env = (val & 0x80) ? 1 : 0;		// IRQ enable
				apu->chd.elen = (val & 0x40) ? 1 : 0;		// loop
				apu->chd.hper = apuPcmPer[val & 0x0f];		// period
				break;
			case 0x4011:
				apu->chd.vol = val & 0x7f;
				break;
			case 0x4012:
				apu->chd.sadr = 0xc000 | ((val << 6) & 0x3fc0);
				apu->chd.cadr = apu->chd.sadr;
				break;
			case 0x4013:
				apu->chd.len = ((val << 4) & 0x0ff0) | 1;
				apu->chd.lcnt = apu->chd.len;
				break;
			// ...
			case 0x4014:		// OAMDMA
				adr = (val << 8);
				do {
					comp->vid->ppu->oam[comp->vid->ppu->oamadr & 0xff] = memRd(comp->mem, adr);
					comp->vid->ppu->oamadr++;
					adr++;
				} while (adr & 0xff);
				// comp->cpu->t += 0x200;
				break;
			case 0x4015:
				if (~val & 0x01) apu->ch0.len = 0;
				if (~val & 0x02) apu->ch1.len = 0;
				if (~val & 0x04) apu->cht.len = 0;
				if (~val & 0x08) apu->chn.len = 0;
				if (~val & 0x10) apu->chd.len = 0;
				break;
			case 0x4016:
				if (val & 1) {		// b0: 0-1-0 = reload gamepads state
					comp->nes.priJoy = comp->nes.priPadState;
					comp->nes.secJoy = comp->nes.secPadState;
				}
				break;
			case 0x4017:
				apu->irqen = (val & 0x80) ? 0 : 1;
				apu->step5 = (val & 0x40) ? 1 : 0;
				break;
			default:
				//printf("write %.4X,%.2X\n",adr,val);
				break;
		}
	} else if (adr < 0x6000) {
		// nothing?
	} else if (comp->slot->data) {
		// sram 8K
		sltWrite(comp->slot, SLT_RAM, adr, val);
	}
}

unsigned char nesSLrd(unsigned short adr, void* data) {
	xCartridge* slot = (xCartridge*)data;
	return sltRead(slot, SLT_PRG, adr);
}

void nesSLwr(unsigned short adr, unsigned char val, void* data) {
	xCartridge* slot = (xCartridge*)data;
	sltWrite(slot, SLT_PRG, adr, val);
}

void nesMaper(Computer* comp) {
	memSetBank(comp->mem, MEM_BANK0, MEM_EXT, 0, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK1, MEM_EXT, 1, nesMMrd, nesMMwr, comp);
	memSetBank(comp->mem, MEM_BANK2, MEM_SLOT, 0, nesSLrd, nesSLwr, comp->slot);
	memSetBank(comp->mem, MEM_BANK3, MEM_SLOT, 1, nesSLrd, nesSLwr, comp->slot);
}

void nesSync(Computer* comp, int ns) {
	if (comp->vid->vbstrb && comp->vid->ppu->inten) {	// @ start of VBlank
		comp->vid->vbstrb = 0;
		comp->cpu->intrq |= MOS6502_INT_NMI;		// request NMI...
	}

	apuSync(comp->nesapu, ns);

	// MMC3 irq counter triggered @ HBlank (?)
	if ((comp->vid->hbstrb) && (comp->vid->ray.y < 239)) {
		comp->vid->hbstrb = 0;
		if (comp->slot->irqrl) {
			comp->slot->irqrl = 0;
			comp->slot->icnt = comp->slot->reg03;
		} else {
			comp->slot->icnt--;
			if (comp->slot->icnt == 0) {
				comp->slot->icnt = comp->slot->reg03;
				comp->slot->irq = comp->slot->irqen;
			}
		}
	}

	int irq = comp->nesapu->frm | comp->slot->irq;		// external irq signals
	comp->nesapu->frm = 0;
	comp->slot->irq = 0;

	if (irq && !(comp->cpu->f & MFI))
		comp->cpu->intrq |= MOS6502_INT_BRK;
}

unsigned char nesMemRd(Computer* comp, unsigned short adr, int m1) {
	return memRd(comp->mem, adr);
}

void nesMemWr(Computer* comp, unsigned short adr, unsigned char val) {
	memWr(comp->mem, adr, val);
}

// keypress

typedef struct {
	signed int id;
	int mask;
} nesKey;

static nesKey nesKeyMap[8] = {
	{XKEY_Z,1},		// A
	{XKEY_X,2},		// B
	{XKEY_SPACE,4},		// select
	{XKEY_ENTER,8},		// start
	{XKEY_UP,16},		// up
	{XKEY_DOWN,32},		// down
	{XKEY_LEFT,64},		// left
	{XKEY_RIGHT,128}	// right
};

int nesGetInputMask(signed int keyid) {
	int idx = 0;
	int mask = 0;
	while (idx < 8) {
		if (nesKeyMap[idx].id == keyid) {
			mask = nesKeyMap[idx].mask;
		}
		idx++;
	}
	return mask;
}

static char nesBgOn[] = " BG layer on ";
static char nesBgOff[] = " BG layer off ";
static char nesSpOn[] = " SPR layer on ";
static char nesSpOff[] = " SPR layer off ";
static char nesPAL[] = " PAL ";
static char nesNTSC[] = " NTSC ";

void nes_keyp(Computer* comp, keyEntry ent) {
	int mask = nesGetInputMask(ent.key);
	comp->nes.priPadState |= mask;
	switch (ent.key) {
		case XKEY_0:
			comp->nes.pal ^= 1;
			comp->msg = comp->nes.pal ? nesPAL : nesNTSC;
			compUpdateTimings(comp);
			break;
		case XKEY_1:
			comp->vid->ppu->bglock ^= 1;
			comp->msg = comp->vid->ppu->bglock ? nesBgOff : nesBgOn;
			break;
		case XKEY_2:
			comp->vid->ppu->splock ^= 1;
			comp->msg = comp->vid->ppu->splock ? nesSpOff : nesSpOn;
			break;
	}
}

void nes_keyr(Computer* comp, keyEntry ent) {
	int mask = nesGetInputMask(ent.key);
	if (mask == 0) return;
	comp->nes.priPadState &= ~mask;
}

sndPair nes_vol(Computer* comp) {
	sndPair vol = apuVolume(comp->nesapu);
	return  vol;
}
